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

    m_deviceWidget = new view::DeviceWidget();
    connect(m_deviceWidget, &view::DeviceWidget::windowClosed, this, &Session::onWindowClosed, Qt::QueuedConnection);
    m_deviceWidget->show();

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
    args << "audio=true";
    args << "cleanup=true";
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
void Session::onWindowClosed()
{
    FUNC_TRACE;
    if (m_adbProcess) {
        m_adbProcess->terminate();
        m_adbProcess->waitForFinished(3000);
        m_adbProcess->deleteLater();
        m_adbProcess = nullptr;
    }

    m_network->stop();
    m_deviceWidget->deleteLater();
    m_deviceWidget = nullptr;

    emit sessionClosed(m_serial);
}
void Session::onReceivedDeviceName(const QString& deviceName)
{
    qInfo() << "Connected to device:" << deviceName;
    if (m_deviceWidget) {
        m_deviceWidget->setWindowTitle(deviceName);
    }
}
void Session::onReceivedVideoMetaData(int codec, int width, int height)
{
    LOGI("Received video metadata: codec={}, width={}, height={}", codec, width, height);
}
void Session::onReceivedVideoData(bool configFlag, bool keyFrameFlag, int64_t pts, const QByteArray& data)
{
    // DEBUG
    LOGD("Received video data: pts={}, size={}", pts, data.size());
    if (configFlag) {
        LOGI("Is config frame");
    }
    if (keyFrameFlag) {
        LOGI("Is key frame");
    }
    static QFile testFile("video_output.h264");
    if (!testFile.isOpen()) {
        if (!testFile.open(QIODevice::WriteOnly)) {
            LOGE("Failed to open video output file");
            return;
        }
    }
    testFile.write(data);
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
    if (m_deviceWidget) m_deviceWidget->close();
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

    if (m_deviceWidget) m_deviceWidget->close();
}
