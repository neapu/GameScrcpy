//
// Created by neapu on 2025/12/9.
//

#include "SessionManager.h"
#include "device/AdbHelper.h"
#include "logger.h"
#include <QCoreApplication>
#include <QRegularExpression>


SessionManager::SessionManager()
    : QObject(nullptr)
{
}

SessionManager* SessionManager::instance()
{
    static SessionManager instance;
    return &instance;
}

void SessionManager::updateDeviceList()
{
    FUNC_TRACE;
    using namespace device;
    AdbHelper::runCommandAsync({"devices", "-l"}).then([this](const QString& output) {
        QList<DeviceInfoPtr> deviceList;
        QStringList lines = output.split('\n', Qt::SkipEmptyParts);

        for (const QString& line : lines) {
            if (line.startsWith("List of devices attached")) continue;

            QString trimmedLine = line.trimmed();
            if (trimmedLine.isEmpty()) continue;

            QStringList parts = trimmedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (parts.size() < 2) continue;

            auto info = std::make_shared<DeviceInfo>();
            info->serial = parts[0];
            info->status = parts[1];

            for (int j = 2; j < parts.size(); ++j) {
                if (parts[j].startsWith("model:")) {
                    info->name = parts[j].mid(6);
                    info->name = info->name.replace('_', ' ');
                }
            }

            if (info->name.isEmpty()) {
                info->name = info->serial;
            }

            deviceList.append(info);
        }

        if (deviceList.isEmpty()) {
            emit deviceListUpdated(deviceList);
            return;
        }

        // Fetch Android version for each device
        QList<QFuture<QString>> futures;
        for (const auto& device : deviceList) {
            if (device->status == "device") {
                futures.append(AdbHelper::runCommandAsync(device->serial, {"shell", "getprop", "ro.build.version.release"}));
            } else {
                // For offline/unauthorized devices, return empty result
                futures.append(QtFuture::makeReadyValueFuture(QString()));
            }
        }

        QtFuture::whenAll(futures.begin(), futures.end()).then([this, deviceList](const QList<QFuture<QString>>& results) {
            for (int i = 0; i < results.size() && i < deviceList.size(); ++i) {
                const auto& future = results[i];
                try {
                    const QString& version = future.result();
                    if (!version.isEmpty()) {
                        deviceList[i]->androidVersion = QString("Android ") + version;
                    }
                } catch (const device::AdbException&) {
                    LOGE("Failed to get Android version for device {}", qPrintable(deviceList[i]->serial));
                }
            }
            emit deviceListUpdated(deviceList);
        });
    }).onFailed([this](const device::AdbException&) {
        emit deviceListUpdated({});
    });
}
bool SessionManager::openDevice(const QString& serial)
{
    FUNC_TRACE;
    if (m_openedSession.contains(serial)) {
        return false;
    }

    const auto session = new Session(serial, this);
    if (!session->open()) {
        session->deleteLater();
        return false;
    }
    connect(session, &Session::sessionClosed, this, [this, serial]() {
        m_openedSession[serial]->deleteLater();
        m_openedSession.remove(serial);
        LOGI("Session for device {} closed", serial.toStdString());
    });
    m_openedSession.insert(serial, session);

    return true;
}
bool SessionManager::isDeviceOpened(const QString& serial) const
{
    return m_openedSession.contains(serial);
}
