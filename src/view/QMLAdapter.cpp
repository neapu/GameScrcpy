//
// Created by neapu on 2025/12/9.
//

#include "QMLAdapter.h"
#include <logger.h>
#include "../SessionManager.h"

namespace view {
QMLAdapter::QMLAdapter()
    : QObject(nullptr)
{}
QMLAdapter* QMLAdapter::instance()
{
    static QMLAdapter instance;
    return &instance;
}
void QMLAdapter::addRemoteDevice(const QString& addr)
{
    FUNC_TRACE;
    Q_UNUSED(addr);
    // TODO: 实现通过ADB添加远程设备
}
void QMLAdapter::updateDeviceList()
{
    FUNC_TRACE;
    SessionManager::instance()->updateDeviceList();
}
QString QMLAdapter::openDevice(const QString& serial)
{
    FUNC_TRACE;
    if (SessionManager::instance()->isDeviceOpened(serial)) {
        LOGW("Device {} is already opened", serial.toStdString());
        const QString reason = QStringLiteral("设备已打开：%1").arg(serial);
        return reason;
    }
    if (!SessionManager::instance()->openDevice(serial)) {
        LOGE("Failed to open device {}", serial.toStdString());
        const QString reason = QStringLiteral("打开设备失败：%1").arg(serial);
        return reason;
    }
    return QString();
}
} // namespace view
