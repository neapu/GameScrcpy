//
// Created by neapu on 2025/12/10.
//

#include "Session.h"

#include "device/AdbHelper.h"

#include <logger.h>
#include <QFile>
#include <QCoreApplication>

constexpr auto SCRCPY_SERVER_PATH = "/data/local/tmp/scrcpy-server.jar";
constexpr auto SCRCPY_SERVER_VERSION = "3.3.3";

constexpr auto VIDEO_CODEC_H264_ID = 0x68323634;
constexpr auto VIDEO_CODEC_HEVC_ID = 0x68323635;
constexpr auto VIDEO_CODEC_AV1_ID  = 0x00617631;

static QString getScrcpyServerLocalPath()
{
#ifdef DEBUG_MODE
    return QStringLiteral(SOURCE_DIR) + "/tools/scrcpy-server"; // 没有.jar后缀
#else
    return QCoreApplication::applicationDirPath() + "/tools/scrcpy-server"; // 没有.jar后缀
#endif
}

Session::Session(const QString& serial, QObject* parent)
    : QObject(parent)
    , m_serial(serial)
{
    m_network = new network::Network(this);
    connect(m_network, &network::Network::receivedVideoMetaData, this, &Session::onReceivedVideoMetaData);
    connect(m_network, &network::Network::receivedVideoData, this, &Session::onReceivedVideoData);
    connect(m_network, &network::Network::receivedAudioMetaData, this, &Session::onReceivedAudioMetaData);
    connect(m_network, &network::Network::receivedAudioData, this, &Session::onReceivedAudioData);
}
bool Session::open()
{
    FUNC_TRACE;
    if (!m_network->start()) {
        LOGE("Failed to start network for device {}", m_serial.toStdString());
        return false;
    }

    m_deviceWindow = new view::DeviceWindow();
    connect(m_deviceWindow, &view::DeviceWindow::windowClosed, this, &Session::onWindowClosed, Qt::QueuedConnection);
    m_deviceWindow->show();

    const QString localServerPath = getScrcpyServerLocalPath();
    if (!QFile::exists(localServerPath)) {
        LOGE("Scrcpy server file not found: {}", localServerPath.toStdString());
    }

    device::AdbHelper::runCommandAsync(m_serial, {
        "push",
        localServerPath,
        QString::fromLatin1(SCRCPY_SERVER_PATH)
    }).onFailed(this, [this](const device::AdbException& ex) -> QString {
        LOGE("Failed to push scrcpy server to device {}: {}", m_serial.toStdString(), ex.message().toStdString());
        throw ex;
    }).then(this, [this](const QString& output) {
        Q_UNUSED(output);
        return device::AdbHelper::runCommandAsync(m_serial, {
            "reverse",
            "localabstract:scrcpy",
            QString("tcp:%1").arg(m_network->port())
        });
    }).unwrap().onFailed(this, [this](const device::AdbException& ex) -> QString {
        LOGE("Failed to set adb reverse for device {}: {}", m_serial.toStdString(), ex.message().toStdString());
        throw ex;
    }).then(this, [this](const QString& output2) {
        Q_UNUSED(output2);
        startScrcpyServer();
    });

    return true;
}
void Session::startScrcpyServer()
{
    FUNC_TRACE;
    QStringList args;
    args << "shell";
    args << QString("CLASSPATH=%1").arg(SCRCPY_SERVER_PATH);
    args << "app_process";
    args << "/";
    args << "com.genymobile.scrcpy.Server";
    args << SCRCPY_SERVER_VERSION;
    args << "tunnel_forward=false";
    args << "control=false";
    args << "video=true";
    args << "audio=false";
    args << "cleanup=true";
    args << "video_bit_rate=8000000";
    args << "video_codec=h264";
    m_adbProcess = device::AdbHelper::runBackgroundCommand(m_serial, args);
    if (!m_adbProcess) {
        LOGE("Failed to start scrcpy server for device {}", m_serial.toStdString());
        return;
    }
    connect(m_adbProcess, &QProcess::errorOccurred, this, &Session::onAdbProcessError, Qt::QueuedConnection);
    connect(m_adbProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Session::onAdbProcessFinished, Qt::QueuedConnection);
    connect(m_adbProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        const auto out = m_adbProcess->readAllStandardOutput();
        if (!out.isEmpty()) {
            LOGI("[server stdout] {}", QString::fromUtf8(out).toStdString());
        }
    }, Qt::QueuedConnection);
    connect(m_adbProcess, &QProcess::readyReadStandardError, this, [this]() {
        const auto err = m_adbProcess->readAllStandardError();
        if (!err.isEmpty()) {
            LOGE("[server stderr] {}", QString::fromUtf8(err).toStdString());
        }
    }, Qt::QueuedConnection);
    m_adbProcess->start();
    LOGI("Started scrcpy server for device {}", m_serial.toStdString());
}
void Session::onVideoFrameDecoded(codec::FramePtr&& frame) const
{
    if (!m_deviceWindow) return;
    QMetaObject::invokeMethod(m_deviceWindow, [dw = m_deviceWindow, f = std::move(frame)]() mutable {
        dw->renderFrame(std::move(f));
    }, Qt::QueuedConnection);
}
void Session::onWindowClosed()
{
    FUNC_TRACE;
    if (m_adbProcess) {
        m_adbProcess->terminate();
        m_adbProcess->waitForFinished(3000);
        m_adbProcess->deleteLater();
        m_adbProcess = nullptr;
    }

    {
        QMutexLocker locker(&m_videoDecoderMutex);
        m_videoDecoder.reset();
    }

    m_network->stop();
    m_deviceWindow->deleteLater();
    m_deviceWindow = nullptr;

    emit sessionClosed(m_serial);
}
void Session::onReceivedDeviceName(const QString& deviceName)
{
    qInfo() << "Connected to device:" << deviceName;
    if (m_deviceWindow) {
        m_deviceWindow->setWindowTitle(deviceName);
    }
}
void Session::onReceivedVideoMetaData(int codec, int width, int height)
{
    LOGI("Received video metadata: codec={}, width={}, height={}", codec, width, height);
    if (m_videoDecoder) {
        return;
    }

    codec::VideoDecoder::CreateParam param;
    param.width = width;
    param.height = height;
    if (codec == VIDEO_CODEC_H264_ID) {
        param.codecType = codec::VideoDecoder::CodecType::h264;
    } else if (codec == VIDEO_CODEC_HEVC_ID) {
        param.codecType = codec::VideoDecoder::CodecType::hevc;
    } else if (codec == VIDEO_CODEC_AV1_ID) {
        param.codecType = codec::VideoDecoder::CodecType::av1;
    } else {
        LOGE("Unsupported video codec ID: {}", codec);
        return;
    }
    param.frameCallback = std::bind(&Session::onVideoFrameDecoded, this, std::placeholders::_1);
    param.swDecode = false;

    {
        QMutexLocker locker(&m_videoDecoderMutex);
        m_videoDecoder = std::make_unique<codec::VideoDecoder>(param);
    }
}
void Session::onReceivedVideoData(bool configFlag, bool keyFrameFlag, int64_t pts, const QByteArray& data)
{
    auto packet = codec::Packet::fromData(configFlag, keyFrameFlag, pts, reinterpret_cast<const uint8_t*>(data.constData()), data.size());
    if (!packet) {
        LOGE("Failed to create video packet");
        return;
    }
    QMutexLocker locker(&m_videoDecoderMutex);
    if (m_videoDecoder) {
        m_videoDecoder->decode(std::move(packet));
    }
}
void Session::onReceivedAudioMetaData(int codecId)
{
    LOGI("Received audio metadata: codecId={}", codecId);
}
void Session::onReceivedAudioData(bool configFlag, bool keyFrameFlag, int64_t pts, const QByteArray& data)
{
    // TODO: Handle audio data
    LOGD("Received audio data: pts={}, size={}", pts, data.size());
    if (configFlag) {
        LOGI("Is config frame");
    }
    if (keyFrameFlag) {
        LOGI("Is key frame");
    }
}

void Session::onAdbProcessError(QProcess::ProcessError error) const
{
    qCritical() << "ADB process error:" << error;
    if (m_deviceWindow) m_deviceWindow->close();
}
void Session::onAdbProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) const
{
    if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
        qCritical() << "ADB process finished with error. Exit code:" << exitCode << "Exit status:" << exitStatus;
    } else {
        LOGI("ADB process finished successfully for device {}", m_serial.toStdString());
    }
    // 关闭reverse转发
    device::AdbHelper::runCommandAsync(m_serial, {
        "reverse",
        "--remove",
        "localabstract:scrcpy"
    }).then([](const QString& output) {
        Q_UNUSED(output);
        LOGI("Removed adb reverse for scrcpy");
    }).onFailed([](const device::AdbException& ex) {
        LOGE("Failed to remove adb reverse for scrcpy: {}", ex.message().toStdString());
    });

    if (m_deviceWindow) m_deviceWindow->close();
}
