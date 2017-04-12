#include "mapping.h"

#include "../remotedevconstants.h"

#include "../mappingsmanager.h"

#include <QDebug>
#include <Qt>
#include <QStandardItem>

namespace RemoteDev {
namespace Internal {

namespace {
const auto NAME_KEY = QStringLiteral("name");
const auto ENABLED_KEY = QStringLiteral("enabled");
const auto DEVICE_KEY = QStringLiteral("device");
const auto PATH_KEY = QStringLiteral("path");
}

Mapping::Mapping(int index, QStandardItemModel &storage) :
    m_index(index),
    m_storage(&storage)
{}

Mapping::Mapping(const QString &name,
                 bool isEnabled,
                 const Core::Id &deviceId,
                 const QString &remotePath,
                 QStandardItemModel &storage) :
    m_index(storage.rowCount()),
    m_storage(&storage)
{
    auto nameItem = new QStandardItem(name);
    qDebug() << "Restoring mapping" << nameItem->text();

    auto enabledItem = new QStandardItem;
    enabledItem->setData(isEnabled, Qt::CheckStateRole);

    auto deviceItem = new QStandardItem(deviceId.toString());
    deviceItem->setData(deviceId.toSetting(), Constants::DEV_ID_ROLE);

    auto pathItem = new QStandardItem(remotePath);

    storage.appendRow({nameItem, enabledItem, deviceItem, pathItem});
}

Mapping Mapping::fromMap(const QVariantMap &map,
                         QStandardItemModel &storage)
{
    const auto mappingName = map.value(NAME_KEY).toString();
    qDebug() << "Restoring mapping" << mappingName;

    return Mapping(mappingName,
                   map.value(ENABLED_KEY).toBool(),
                   Core::Id::fromSetting(map.value(DEVICE_KEY)),
                   map.value(PATH_KEY).toString(),
                   storage);
}

QVariantMap Mapping::toMap() const
{
    return {
        {NAME_KEY, name()},
        {ENABLED_KEY, isEnabled()},
        {DEVICE_KEY, deviceId().toSetting()},
        {PATH_KEY, remotePath().toString()}
    };
}

bool Mapping::isEnabled() const
{
    return m_storage->item(m_index, Constants::MAP_ENABLED_COLUMN)
            ->data(Qt::CheckStateRole).toBool();
}

QString Mapping::name() const
{
    return m_storage->item(m_index, Constants::MAP_NAME_COLUMN)->text();
}

Core::Id Mapping::deviceId() const
{
    return Core::Id::fromSetting(
        m_storage->item(m_index, Constants::MAP_DEVICE_COLUMN)->data(Constants::DEV_ID_ROLE)
    );
}

Utils::FileName Mapping::remotePath() const
{
    return Utils::FileName::fromString(
        m_storage->item(m_index, Constants::MAP_PATH_COLUMN)->text()
    );
}

} // namespace Internal
} // namespace RemoteDev
