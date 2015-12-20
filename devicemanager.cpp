#include "devicemanager.h"

#include <QStandardItemModel>

#include <projectexplorer/devicesupport/devicemanager.h>

#include "remotedevconstants.h"

using namespace RemoteDev::Internal;

DeviceManager::DeviceManager(QObject *parent) :
    QObject(parent),
    m_devices(new QStandardItemModel(0, Constants::DEV_COLUMNS_COUNT, this))
{}

void DeviceManager::startDeviceSync()
{
    auto deviceMgr = ProjectExplorer::DeviceManager::instance();

    connect(deviceMgr, &ProjectExplorer::DeviceManager::deviceAdded,
            this, &DeviceManager::onDeviceAdded);

    connect(deviceMgr, &ProjectExplorer::DeviceManager::deviceUpdated,
            this, &DeviceManager::onDeviceUpdated);

    connect(deviceMgr, &ProjectExplorer::DeviceManager::deviceRemoved,
            this, &DeviceManager::onDeviceRemoved);

    connect(deviceMgr, &ProjectExplorer::DeviceManager::deviceListReplaced,
            this, &DeviceManager::onDeviceListReplaced);

    // FIXME: should I handle &ProjectExplorer::DeviceManager::updated?
    onDeviceListReplaced();
}

QStandardItemModel *DeviceManager::devices() const
{
    return m_devices;
}

void DeviceManager::onDeviceAdded(Core::Id id)
{
    auto device = ProjectExplorer::DeviceManager::instance()->find(id);
    if (! device.isNull()) {
        m_devices->appendRow({ createNameItem(device), createIdItem(device) });
    }
}

void DeviceManager::onDeviceUpdated(Core::Id id)
{
    auto device = ProjectExplorer::DeviceManager::instance()->find(id);
    if (! device.isNull()) {
        auto items = m_devices->findItems(id.toString(),
                                          Qt::MatchExactly, Constants::DEV_ID_COLUMN);
        // FIXME: give warning if more than one found
        if (items.count() > 0) {
            m_devices->setItem(items.at(0)->row(), Constants::DEV_NAME_COLUMN,
                               createNameItem(device));
            m_devices->setItem(items.at(0)->row(), Constants::DEV_ID_COLUMN,
                               createIdItem(device));
        }
    }
}

void DeviceManager::onDeviceRemoved(Core::Id id)
{
    auto items = m_devices->findItems(id.toString(),
                                      Qt::MatchExactly, Constants::DEV_ID_COLUMN);
    // FIXME: give warning if more than one found
    if (items.count() > 0) {
        m_devices->removeRow(items.at(0)->row());
    }
}

// bulk update or initialization
void DeviceManager::onDeviceListReplaced()
{
    auto deviceMgr = ProjectExplorer::DeviceManager::instance();

    m_devices->setRowCount(0); // clear only data items
    for (int i = 0; i < deviceMgr->deviceCount(); i++) {
        auto device = deviceMgr->deviceAt(i);

        // FIXME: do I need check device.sshParameters?
        m_devices->appendRow({ createNameItem(device), createIdItem(device) });
    }
}

QStandardItem *DeviceManager::createNameItem(ProjectExplorer::IDevice::ConstPtr device)
{
    return new QStandardItem(device->displayName());
}

QStandardItem *DeviceManager::createIdItem(ProjectExplorer::IDevice::ConstPtr device)
{
    auto idItem = new QStandardItem(device->id().toString()); // string needed for lookup
    idItem->setData(device->id().toSetting(), RemoteDev::Constants::DEV_ID_ROLE);

    return idItem;
}

