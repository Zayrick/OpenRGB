/*---------------------------------------------------------*\
| DeviceListModel.cpp                                       |
|                                                           |
|   OpenRGB QML Device List Model - provides device data   |
|   to QML interface                                        |
|                                                           |
|   OpenRGB Custom UI Development                           |
\*---------------------------------------------------------*/

#include "DeviceListModel.h"
#include <QDebug>
#include "RGBController.h"
#include "ResourceManager.h"

DeviceListModel::DeviceListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_resourceManager(nullptr)
    , m_autoRefreshTimer(new QTimer(this))
    , m_autoRefreshEnabled(false)
{
    // 获取 ResourceManager 实例
    m_resourceManager = ResourceManager::get();

    // 设置自动刷新定时器
    m_autoRefreshTimer->setInterval(1000); // 每秒刷新一次
    connect(m_autoRefreshTimer, &QTimer::timeout, this, &DeviceListModel::onAutoRefreshTimer);

    // 初始化设备列表
    updateDeviceList();

    // 默认启用自动刷新
    startAutoRefresh();
}

DeviceListModel::~DeviceListModel()
{
    stopAutoRefresh();
}

int DeviceListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return static_cast<int>(m_devices.size());
}

QVariant DeviceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_devices.size()))
        return QVariant();

    RGBController* device = m_devices[index.row()];
    if (!device)
        return QVariant();

    switch (role) {
    case NameRole:
        return QString::fromStdString(device->name);
    case VendorRole:
        return QString::fromStdString(device->vendor);
    case TypeRole:
        return deviceTypeToString(device->type);
    case LocationRole:
        return QString::fromStdString(device->location);
    case DescriptionRole:
        return QString::fromStdString(device->description);
    case ConnectedRole:
        return true; // 如果设备在列表中，说明已连接
    case SerialRole:
        return QString::fromStdString(device->serial);
    case VersionRole:
        return QString::fromStdString(device->version);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> DeviceListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[VendorRole] = "vendor";
    roles[TypeRole] = "type";
    roles[LocationRole] = "location";
    roles[DescriptionRole] = "description";
    roles[ConnectedRole] = "connected";
    roles[SerialRole] = "serial";
    roles[VersionRole] = "version";
    return roles;
}

void DeviceListModel::refreshDeviceList()
{
    updateDeviceList();
}

void DeviceListModel::startAutoRefresh()
{
    if (!m_autoRefreshEnabled) {
        m_autoRefreshEnabled = true;
        m_autoRefreshTimer->start();
        qDebug() << "DeviceListModel: 自动刷新已启用";
    }
}

void DeviceListModel::stopAutoRefresh()
{
    if (m_autoRefreshEnabled) {
        m_autoRefreshEnabled = false;
        m_autoRefreshTimer->stop();
        qDebug() << "DeviceListModel: 自动刷新已停用";
    }
}

void DeviceListModel::onAutoRefreshTimer()
{
    updateDeviceList();
}

void DeviceListModel::updateDeviceList()
{
    if (!m_resourceManager) {
        qWarning() << "DeviceListModel: ResourceManager 未初始化";
        return;
    }

    // 获取当前设备列表
    std::vector<RGBController*>& controllers = m_resourceManager->GetRGBControllers();

    // 检查设备列表是否有变化
    bool hasChanged = (controllers.size() != m_devices.size());
    if (!hasChanged) {
        for (size_t i = 0; i < controllers.size(); ++i) {
            if (controllers[i] != m_devices[i]) {
                hasChanged = true;
                break;
            }
        }
    }

    // 如果有变化，更新模型
    if (hasChanged) {
        beginResetModel();
        m_devices = controllers;
        endResetModel();

        qDebug() << "DeviceListModel: 设备列表已更新，当前设备数量:" << m_devices.size();
    }
}

QString DeviceListModel::deviceTypeToString(device_type type) const
{
    switch (type) {
    case DEVICE_TYPE_MOTHERBOARD:
        return "主板";
    case DEVICE_TYPE_DRAM:
        return "内存";
    case DEVICE_TYPE_GPU:
        return "显卡";
    case DEVICE_TYPE_COOLER:
        return "散热器";
    case DEVICE_TYPE_LEDSTRIP:
        return "LED灯带";
    case DEVICE_TYPE_KEYBOARD:
        return "键盘";
    case DEVICE_TYPE_MOUSE:
        return "鼠标";
    case DEVICE_TYPE_MOUSEMAT:
        return "鼠标垫";
    case DEVICE_TYPE_HEADSET:
        return "耳机";
    case DEVICE_TYPE_HEADSET_STAND:
        return "耳机架";
    case DEVICE_TYPE_GAMEPAD:
        return "手柄";
    case DEVICE_TYPE_LIGHT:
        return "灯光";
    case DEVICE_TYPE_SPEAKER:
        return "音箱";
    case DEVICE_TYPE_VIRTUAL:
        return "虚拟设备";
    case DEVICE_TYPE_STORAGE:
        return "存储设备";
    case DEVICE_TYPE_CASE:
        return "机箱";
    case DEVICE_TYPE_MICROPHONE:
        return "麦克风";
    case DEVICE_TYPE_ACCESSORY:
        return "配件";
    case DEVICE_TYPE_KEYPAD:
        return "数字键盘";
    case DEVICE_TYPE_LAPTOP:
        return "笔记本";
    case DEVICE_TYPE_MONITOR:
        return "显示器";
    case DEVICE_TYPE_UNKNOWN:
    default:
        return "未知设备";
    }
}
