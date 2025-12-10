//
// Created by neapu on 2025/12/9.
//

#pragma once
#include <QObject>
#include "device/DeviceInfo.h"
#include <QString>
#include <QMap>
#include "Session.h"

class SessionManager : public QObject {
    Q_OBJECT
public:
    SessionManager();
    ~SessionManager() override = default;

    static SessionManager* instance();

    void updateDeviceList();

    bool openDevice(const QString& serial);
    bool isDeviceOpened(const QString& serial) const;

signals:
    void deviceListUpdated(const QList<device::DeviceInfoPtr>& deviceList);

private:
    QMap<QString, Session*> m_openedSession;
};

