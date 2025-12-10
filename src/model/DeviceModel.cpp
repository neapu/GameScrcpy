#include "DeviceModel.h"

namespace model {

DeviceModel::DeviceModel() : QAbstractListModel(nullptr) {}

DeviceModel* DeviceModel::instance()
{
    static DeviceModel instance;
    return &instance;
}
int DeviceModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_deviceList.count();
}

QVariant DeviceModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_deviceList.count())
        return QVariant();

    const auto& device = m_deviceList[index.row()];
    if (!device)
        return QVariant();

    switch (role) {
    case NameRole:
        return device->name;
    case Serial:
        return device->serial;
    case AndroidVersion:
        return device->androidVersion;
    case Status:
        return device->status;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[Serial] = "serial";
    roles[AndroidVersion] = "androidVersion";
    roles[Status] = "status";
    return roles;
}

void DeviceModel::onDeviceListUpdated(const QList<device::DeviceInfoPtr>& deviceList)
{
    beginResetModel();
    m_deviceList = deviceList;
    endResetModel();
}
} // namespace model