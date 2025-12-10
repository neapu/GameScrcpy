//
// Created by neapu on 2025/12/9.
//

#pragma once
#include <QObject>

namespace view {

class QMLAdapter : public QObject {
    Q_OBJECT
public:
    QMLAdapter();
    ~QMLAdapter() override = default;

    static QMLAdapter* instance();

    Q_INVOKABLE void addRemoteDevice(const QString& addr);
    Q_INVOKABLE void updateDeviceList();
    Q_INVOKABLE QString openDevice(const QString& serial);
};

} // namespace view
