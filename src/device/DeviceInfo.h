//
// Created by neapu on 2025/12/9.
//

#pragma once
#include <QString>
#include <memory>

namespace device {

class DeviceInfo {
public:
    DeviceInfo() = default;
    ~DeviceInfo() = default;

    QString name;
    QString serial;
    QString androidVersion;
    QString status;
};
using DeviceInfoPtr = std::shared_ptr<DeviceInfo>;
} // namespace device
