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

// 静态回调函数，用于注册到ResourceManager
static void DeviceListModelCallback(void* this_ptr)
{
    DeviceListModel* model = static_cast<DeviceListModel*>(this_ptr);
    if (model) {
        model->onDeviceListChanged();
    }
}

static void DeviceListModelDetectionStartCallback(void* this_ptr)
{
    DeviceListModel* model = static_cast<DeviceListModel*>(this_ptr);
    if (model) {
        model->onDetectionStarted();
    }
}

static void DeviceListModelDetectionEndCallback(void* this_ptr)
{
    DeviceListModel* model = static_cast<DeviceListModel*>(this_ptr);
    if (model) {
        model->onDetectionEnded();
    }
}

DeviceListModel::DeviceListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_resourceManager(nullptr)
    , m_loading(false)
{
    // 获取 ResourceManager 实例
    m_resourceManager = ResourceManager::get();

    // 注册回调函数到 ResourceManager
    if (m_resourceManager) {
        m_resourceManager->RegisterDeviceListChangeCallback(DeviceListModelCallback, this);
        m_resourceManager->RegisterDetectionStartCallback(DeviceListModelDetectionStartCallback, this);
        m_resourceManager->RegisterDetectionEndCallback(DeviceListModelDetectionEndCallback, this);
    }

    // 初始化设备列表
    updateDeviceList();
}

DeviceListModel::~DeviceListModel()
{
    // 注销回调函数
    if (m_resourceManager) {
        m_resourceManager->UnregisterDeviceListChangeCallback(DeviceListModelCallback, this);
        m_resourceManager->UnregisterDetectionStartCallback(DeviceListModelDetectionStartCallback, this);
        m_resourceManager->UnregisterDetectionEndCallback(DeviceListModelDetectionEndCallback, this);
    }
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

bool DeviceListModel::loading() const
{
    return m_loading;
}

void DeviceListModel::setLoading(bool loading)
{
    if (m_loading != loading) {
        m_loading = loading;
        emit loadingChanged();
        qDebug() << "DeviceListModel: 加载状态变更为:" << (loading ? "加载中" : "已完成");
    }
}

void DeviceListModel::refreshDeviceList()
{
    updateDeviceList();
}

void DeviceListModel::onDeviceListChanged()
{
    qDebug() << "DeviceListModel: 收到设备列表变更通知";
    updateDeviceList();
}

void DeviceListModel::onDetectionStarted()
{
    qDebug() << "DeviceListModel: 设备检测开始";
    setLoading(true);
}

void DeviceListModel::onDetectionEnded()
{
    qDebug() << "DeviceListModel: 设备检测结束";
    setLoading(false);
    // 检测结束后刷新一次设备列表，确保显示最新状态
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
