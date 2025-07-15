/*---------------------------------------------------------*\
| DeviceListModel.h                                         |
|                                                           |
|   OpenRGB QML Device List Model - provides device data   |
|   to QML interface                                        |
|                                                           |
|   OpenRGB Custom UI Development                           |
\*---------------------------------------------------------*/

#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QString>
#include <QTimer>
#include <vector>

// 前向声明
class RGBController;
class ResourceManager;
typedef int device_type;

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum DeviceRoles {
        NameRole = Qt::UserRole + 1,
        VendorRole,
        TypeRole,
        LocationRole,
        DescriptionRole,
        ConnectedRole,
        SerialRole,
        VersionRole
    };

    explicit DeviceListModel(QObject *parent = nullptr);
    ~DeviceListModel();

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void refreshDeviceList();
    void startAutoRefresh();
    void stopAutoRefresh();

private slots:
    void onAutoRefreshTimer();

private:
    void updateDeviceList();
    QString deviceTypeToString(device_type type) const;

    std::vector<RGBController*> m_devices;
    ResourceManager* m_resourceManager;
    QTimer* m_autoRefreshTimer;
    bool m_autoRefreshEnabled;
};
