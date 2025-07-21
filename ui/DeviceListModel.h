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
#include <vector>

// 前向声明
class RGBController;
class ResourceManager;
typedef int device_type;

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString progressText READ progressText NOTIFY progressTextChanged)

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

    // 属性访问器
    bool loading() const;
    int progress() const;
    QString progressText() const;

public slots:
    void refreshDeviceList();

    // 连接到 ResourceManager 的回调槽函数
    void onDeviceListChanged();
    void onDetectionStarted();
    void onDetectionEnded();
    void onDetectionProgressUpdate();

signals:
    void loadingChanged();
    void progressChanged();
    void progressTextChanged();

private:
    void updateDeviceList();
    QString deviceTypeToString(device_type type) const;
    void setLoading(bool loading);
    void setProgress(int progress);
    void setProgressText(const QString& text);

    std::vector<RGBController*> m_devices;
    ResourceManager* m_resourceManager;
    bool m_loading;
    int m_progress;
    QString m_progressText;
};
