//
// Created by neapu on 2025/12/10.
//

#pragma once
#include <QTcpServer>

namespace network {

class Network : public QObject {
    Q_OBJECT
public:
    explicit Network(QObject* parent = nullptr);
    ~Network() override = default;

    bool start() const;
    void stop();

    int port() const;

    void sendControlData(const QByteArray& data) const;

signals:
    void receivedDeviceName(const QByteArray& name);
    void receivedVideoMetaData(int codec, int width, int height);
    void receivedVideoData(bool configFlag, bool keyFrameFlag, int64_t pts, const QByteArray& data);
    void receivedAudioMetaData(int codecId);
    void receivedAudioData(bool configFlag, bool keyFrameFlag, int64_t pts, const QByteArray& data);
    // TODO: void receivedControlData(const QByteArray& data);

private slots:
    void onNewConnection();
    void onSocketDisconnected();
    void onReadData();
    void onVideoDataReceived();
    void onAudioDataReceived();
    void onControlDataReceived();

private:
    QTcpServer* m_server{nullptr};
    QTcpSocket* m_videoSocket{nullptr};
    QTcpSocket* m_audioSocket{nullptr};
    QTcpSocket* m_controlSocket{nullptr};

    QByteArray m_videoBuffer;
    QByteArray m_audioBuffer;
    QByteArray m_controlBuffer;

    bool m_deviceNameReceived{false};
    bool m_videoMetaDataReceived{false};
    bool m_audioMetaDataReceived{false};
};

} // namespace network
