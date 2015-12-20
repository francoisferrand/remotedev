#ifndef DEVICESYNCHRONIZER_H
#define DEVICESYNCHRONIZER_H

#include <QObject>

#include <coreplugin/id.h>
#include <projectexplorer/devicesupport/idevice.h>

QT_BEGIN_NAMESPACE
class QStandardItemModel;
class QStandardItem;
QT_END_NAMESPACE


namespace RemoteDev {
namespace Internal {

class RemoteDevPlugin;

/**
 * @brief The DeviceSyncHelper class
 * Utility class for keeping devices synchronized between Devicemanager and
 * plugin's internal storage
 */
class DeviceSyncHelper : public QObject
{
    Q_OBJECT
private:
    friend class RemoteDevPlugin;

    explicit DeviceSyncHelper(QStandardItemModel *devices, QObject *parent = 0);
    void startDeviceSync();

private slots:
    void onDeviceAdded(Core::Id id);
    void onDeviceUpdated(Core::Id id);
    void onDeviceRemoved(Core::Id id);

    void onDeviceListReplaced();

private:
    QStandardItem * createNameItem(ProjectExplorer::IDevice::ConstPtr device);
    QStandardItem * createIdItem(ProjectExplorer::IDevice::ConstPtr device);

private:
    QStandardItemModel *m_devices;
};

} // namespace Internal
} // namespace RemoteDev

#endif // DEVICESYNCHRONIZER_H
