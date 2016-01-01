#include "mappingsmanager.h"

#include <QStandardItemModel>

#include <projectexplorer/project.h>

#include "remotedevconstants.h"

#include <QDebug>

using namespace RemoteDev::Internal;

MappingsManager::MappingsManager(QObject *parent) :
    QObject(parent)
{}

QStandardItemModel *MappingsManager::mappingsForProject(ProjectExplorer::Project *project)
{
    QStandardItemModel *mappings = nullptr;

    if (m_mappings.contains(project)) {
        mappings = m_mappings.value(project);
    } else {
        connect(project, &ProjectExplorer::Project::aboutToSaveSettings,
                [this, project] () { this->saveProjectMappings(project); });

        mappings = readProjectMappings(project);
        mappings->setParent(this);
        m_mappings[project] = mappings;
    }

    return mappings;
}

QStandardItemModel *MappingsManager::readProjectMappings(const ProjectExplorer::Project *project) const
{
    auto items = project->namedSettings(QLatin1String(Constants::SETTINGS_GROUP)).toMap()
                        .value(QLatin1String(Constants::MAPPINGS_GROUP)).toList();

    qDebug() << "Restoring project mappings:" << project->displayName();

    auto model = new QStandardItemModel(0, Constants::MAP_COLUMNS_COUNT);
    for (auto &item : items) {
        auto mapping = item.toMap();

        auto nameItem = new QStandardItem(mapping.value(QStringLiteral("name")).toString());
        qDebug() << "Restoring mapping" << nameItem->text();

        auto enabledItem = new QStandardItem();
        enabledItem->setData(mapping.value(QStringLiteral("enabled")), Qt::CheckStateRole);

        auto deviceId = mapping.value(QStringLiteral("device"));
        auto deviceItem = new QStandardItem(deviceId.toString());
        deviceItem->setData(deviceId, Constants::DEV_ID_ROLE);

        auto pathItem = new QStandardItem(mapping.value(QStringLiteral("path")).toString());

        model->appendRow({ nameItem, enabledItem, deviceItem, pathItem });
    }

    return model;
}

void MappingsManager::saveProjectMappings(ProjectExplorer::Project *project)
{
    auto model = m_mappings.value(project);
    if (! model)
        return;

    qDebug() << "Saving project mappings:" << project->displayName();

    QVariantList list;
    for (int i = 0; i < model->rowCount(); i++) {
        list.append(QVariantMap({
            { QStringLiteral("name"),    model->item(i, Constants::MAP_NAME_COLUMN)->text() },
            { QStringLiteral("enabled"), model->item(i, Constants::MAP_ENABLED_COLUMN)->data(Qt::CheckStateRole) },
            { QStringLiteral("device"),  model->item(i, Constants::MAP_DEVICE_COLUMN)->data(Constants::DEV_ID_ROLE) },
            { QStringLiteral("path"),    model->item(i, Constants::MAP_PATH_COLUMN)->text() }
        }));
    }

    auto group = QLatin1String(Constants::SETTINGS_GROUP);
    auto settings = project->namedSettings(group).toMap();
    settings.insert(QLatin1String(Constants::MAPPINGS_GROUP), list);
    project->setNamedSettings(group, settings);
}

