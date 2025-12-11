#pragma once
#include <QObject>
#include <QProcess>
#include "network/Network.h"
#include "view/DeviceWindow.h"
#include "codec/VideoDecoder.h"

#include <QMutex>

class Session : public QObject {
    Q_OBJECT
public:
    explicit Session(const QString& serial, QObject* parent = nullptr);
    ~Session() override = default;

    bool open();

signals:
    void sessionClosed(const QString& serial);

private:
    void startScrcpyServer();

    void onVideoFrameDecoded(codec::FramePtr&& frame) const;

private slots:
    void onWindowClosed();

    void onReceivedDeviceName(const QString& deviceName);
    void onReceivedVideoMetaData(int codec, int width, int height);
    void onReceivedVideoData(bool configFlag, bool keyFrameFlag, int64_t pts, const QByteArray& data);
    void onReceivedAudioMetaData(int codecId);
    void onReceivedAudioData(bool configFlag, bool keyFrameFlag, int64_t pts, const QByteArray& data);

    void onAdbProcessError(QProcess::ProcessError error) const;
    void onAdbProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) const;

private:
    QString m_serial;
    network::Network* m_network{nullptr};
    view::DeviceWindow* m_deviceWindow{nullptr};
    QProcess* m_adbProcess{nullptr};
    std::unique_ptr<codec::VideoDecoder> m_videoDecoder;
    QMutex m_videoDecoderMutex;
};