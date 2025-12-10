#pragma once

#include <QAbstractListModel>
#include "../device/DeviceInfo.h"

namespace model {
class DeviceModel final : public QAbstractListModel
{
    Q_OBJECT
public:
    DeviceModel();
    ~DeviceModel() override = default;
    static DeviceModel* instance();

    enum DeviceRoles {
        NameRole = Qt::UserRole + 1,
        Serial,
        AndroidVersion,
        Status,
    };

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void onDeviceListUpdated(const QList<device::DeviceInfoPtr>& deviceList);

private:
    QList<device::DeviceInfoPtr> m_deviceList;
};
}