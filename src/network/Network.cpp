//
// Created by neapu on 2025/12/10.
//

#include "Network.h"
#include <logger.h>
#include <QTcpSocket>
#include <QtEndian>

#define PACKET_FLAG_CONFIG (UINT64_C(1) << 63)
#define PACKET_FLAG_KEYFRAME (UINT64_C(1) << 62)
#define PACKET_PTS_MASK (~(PACKET_FLAG_CONFIG | PACKET_FLAG_KEYFRAME))

namespace network {
static uint32_t read32be(const char* data)
{
    return qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(data));
}

static uint64_t read64be(const char* data)
{
    return qFromBigEndian<quint64>(reinterpret_cast<const uchar*>(data));
}

Network::Network(QObject* parent)
    : QObject(parent)
{
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &Network::onNewConnection);
}
bool Network::start() const
{
    FUNC_TRACE;
    if (m_server->isListening()) {
        LOGW("Server is already running");
        return false;
    }

    if (!m_server->listen(QHostAddress::Any, 0)) {
        LOGE("Failed to start server: {}", m_server->errorString().toStdString());
        return false;
    }

    return true;
}

void Network::stop()
{
    FUNC_TRACE;
    if (m_server->isListening()) {
        m_server->close();
    }

    if (m_videoSocket) {
        m_videoSocket->disconnectFromHost();
        m_videoSocket->deleteLater();
        m_videoSocket = nullptr;
    }

    if (m_audioSocket) {
        m_audioSocket->disconnectFromHost();
        m_audioSocket->deleteLater();
        m_audioSocket = nullptr;
    }

    if (m_controlSocket) {
        m_controlSocket->disconnectFromHost();
        m_controlSocket->deleteLater();
        m_controlSocket = nullptr;
    }

    m_videoBuffer.clear();
    m_audioBuffer.clear();
    m_controlBuffer.clear();

    m_deviceNameReceived = false;
    m_videoMetaDataReceived = false;
    m_audioMetaDataReceived = false;
}
int Network::port() const
{
    if (m_server->isListening()) {
        return m_server->serverPort();
    }
    return -1;
}
void Network::sendControlData(const QByteArray& data) const
{
    if (!m_controlSocket || m_controlSocket->state() != QTcpSocket::ConnectedState) {
        LOGW("Control socket is not connected");
        return;
    }
    m_controlSocket->write(data);
}
void Network::onNewConnection()
{
    FUNC_TRACE;

    // 连接顺序一定是：视频->音频->控制
    if (!m_videoSocket) {
        m_videoSocket = m_server->nextPendingConnection();
        connect(m_videoSocket, &QTcpSocket::disconnected, this, &Network::onSocketDisconnected);
        connect(m_videoSocket, &QTcpSocket::readyRead, this, &Network::onReadData);
        LOGI("Video socket connected from {}", m_videoSocket->peerAddress().toString().toStdString());
        return;
    }

    if (!m_audioSocket) {
        m_audioSocket = m_server->nextPendingConnection();
        connect(m_audioSocket, &QTcpSocket::disconnected, this, &Network::onSocketDisconnected);
        connect(m_audioSocket, &QTcpSocket::readyRead, this, &Network::onReadData);
        LOGI("Audio socket connected from {}", m_audioSocket->peerAddress().toString().toStdString());
        return;
    }

    if (!m_controlSocket) {
        m_controlSocket = m_server->nextPendingConnection();
        connect(m_controlSocket, &QTcpSocket::disconnected, this, &Network::onSocketDisconnected);
        connect(m_controlSocket, &QTcpSocket::readyRead, this, &Network::onReadData);
        LOGI("Control socket connected from {}", m_controlSocket->peerAddress().toString().toStdString());
        return;
    }

    LOGW("Received extra connection, closing it");
    QTcpSocket* extraSocket = m_server->nextPendingConnection();
    extraSocket->disconnectFromHost();
    extraSocket->deleteLater();
}

void Network::onSocketDisconnected()
{
    FUNC_TRACE;
    auto senderSocket = qobject_cast<QTcpSocket*>(sender());
    if (!senderSocket) {
        LOGE("Sender is not a QTcpSocket");
        return;
    }

    if (senderSocket == m_videoSocket) {
        LOGI("Video socket disconnected");
    } else if (senderSocket == m_audioSocket) {
        LOGI("Audio socket disconnected");
    } else if (senderSocket == m_controlSocket) {
        LOGI("Control socket disconnected");
    } else {
        LOGW("Unknown socket disconnected");
        senderSocket->deleteLater();
    }
}
void Network::onReadData()
{
    auto senderSocket = qobject_cast<QTcpSocket*>(sender());
    if (!senderSocket) {
        LOGE("Sender is not a QTcpSocket");
        return;
    }
    if (senderSocket == m_videoSocket) {
        onVideoDataReceived();
    } else if (senderSocket == m_audioSocket) {
        onAudioDataReceived();
    } else if (senderSocket == m_controlSocket) {
        onControlDataReceived();
    } else {
        LOGW("Unknown socket data received");
        senderSocket->readAll(); // 清空数据
    }
}
void Network::onVideoDataReceived()
{
    m_videoBuffer += m_videoSocket->readAll();
    // LOGD("Buffer Size: {}", m_videoBuffer.size());
    if (!m_deviceNameReceived) {
        // 读取设备名称，64字节utf-8编码，不足补0
        if (m_videoBuffer.size() < 64) {
            return; // 数据不完整，继续等待
        }
        QByteArray nameData = m_videoBuffer.left(64);
        m_videoBuffer.remove(0, 64);
        QString deviceName = QString::fromUtf8(nameData).trimmed();
        m_deviceNameReceived = true;
        emit receivedDeviceName(deviceName.toUtf8());
    }

    if (!m_videoMetaDataReceived) {
        // 读取视频元数据 (big-endian)
        // CodecID: 4 bytes
        // Width: 4 bytes
        // Height: 4 bytes
        if (m_videoBuffer.size() < 12) {
            return; // 数据不完整，继续等待
        }
        const int codec = static_cast<int>(read32be(m_videoBuffer.constData()));
        const int width = static_cast<int>(read32be(m_videoBuffer.constData() + 4));
        const int height = static_cast<int>(read32be(m_videoBuffer.constData() + 8));
        m_videoBuffer.remove(0, 12);
        m_videoMetaDataReceived = true;
        emit receivedVideoMetaData(codec, width, height);
    }

    while (true) {
        // 读取帧头 (big-endian)
        // 1bit config flag
        // 1bit key frame flag
        // 62bits pts
        // 4bytes data length
        if (m_videoBuffer.size() < 12) {
            return; // 数据不完整，继续等待
        }
        const uint64_t pts_flags = read64be(m_videoBuffer.constData());
        const uint32_t dataLength = read32be(m_videoBuffer.constData() + 8);
        // LOGD("Data Length: {}", dataLength);
        const bool configFlag = (pts_flags & PACKET_FLAG_CONFIG) != 0;
        const bool keyFrameFlag = (pts_flags & PACKET_FLAG_KEYFRAME) != 0;
        const int64_t pts = static_cast<int64_t>(pts_flags & PACKET_PTS_MASK);
        if (dataLength == 0) {
            LOGW("Received video frame with zero length, skipping");
            m_videoBuffer.remove(0, 12);
            continue;
        }
        if (m_videoBuffer.size() < 12 + dataLength) {
            return; // 数据不完整，继续等待
        }
        QByteArray frameData = m_videoBuffer.mid(12, dataLength);
        m_videoBuffer.remove(0, 12 + dataLength);
        emit receivedVideoData(configFlag, keyFrameFlag, pts, frameData);
    }
}
void Network::onAudioDataReceived()
{
    m_audioBuffer += m_audioSocket->readAll();
    if (!m_audioMetaDataReceived) {
        // 读取音频元数据
        // CodecID: 4 bytes
        if (m_audioBuffer.size() < 4) {
            return; // 数据不完整，继续等待
        }
        const int codecId = static_cast<int>(read32be(m_audioBuffer.constData()));
        m_audioBuffer.remove(0, 4);
        m_audioMetaDataReceived = true;
        emit receivedAudioMetaData(codecId);
    }

    while (true) {
        // 读取帧头 (big-endian)
        // 1bit config flag
        // 1bit key frame flag
        // 62bits pts
        // 4bytes data length
        if (m_audioBuffer.size() < 12) {
            return; // 数据不完整，继续等待
        }
        const uint64_t pts_flags = read64be(m_audioBuffer.constData());
        const uint32_t dataLength = read32be(m_audioBuffer.constData() + 8);
        const bool configFlag = (pts_flags & PACKET_FLAG_CONFIG) != 0;
        const bool keyFrameFlag = (pts_flags & PACKET_FLAG_KEYFRAME) != 0;
        const int64_t pts = static_cast<int64_t>(pts_flags & PACKET_PTS_MASK);
        if (dataLength == 0) {
            LOGW("Received audio frame with zero length, skipping");
            m_audioBuffer.remove(0, 12);
            continue;
        }
        if (m_audioBuffer.size() < 12 + dataLength) {
            return; // 数据不完整，继续等待
        }
        QByteArray frameData = m_audioBuffer.mid(12, dataLength);
        m_audioBuffer.remove(0, 12 + dataLength);
        emit receivedAudioData(configFlag, keyFrameFlag, pts, frameData);
    }
}
void Network::onControlDataReceived()
{
    // TODO: 实现控制数据的处理逻辑
    m_controlSocket->readAll();
}
} // namespace network
