#include "mappingsmanager.h"

#include <QStandardItemModel>

#include <projectexplorer/project.h>

#include "remotedevconstants.h"

#include <QDebug>


namespace RemoteDev {
namespace Internal {

MappingsManager::MappingsManager(QObject *parent) :
    QObject(parent)
{}

const std::vector<Mapping> &MappingsManager::mappingsForProject(ProjectExplorer::Project &project)
{
    return mappingsMetaForProject(project).mappings;
}

void MappingsManager::createMapping(const ProjectExplorer::Project &project,
                                    const QString &name,
                                    bool isEnabled,
                                    const Core::Id &deviceId,
                                    const QString &remotePath)
{
    auto &projectMappings = m_mappings[&project];

    projectMappings.mappings.emplace_back(
        Mapping(name, isEnabled, deviceId, remotePath, *projectMappings.storage)
    );
}

void MappingsManager::removeMapping(const ProjectExplorer::Project &project,
                                    std::uint16_t index)
{
    if (!m_mappings.contains(&project))
        return;

    auto &metaMappings = m_mappings[&project];
    if (index >= metaMappings.mappings.size())
        return;

    metaMappings.mappings.erase(metaMappings.mappings.begin() + static_cast<int64_t>(index));
    metaMappings.storage->removeRow(static_cast<int>(index));
}

QStandardItemModel &MappingsManager::storageForProject(ProjectExplorer::Project &project)
{
    return *mappingsMetaForProject(project).storage;
}

MappingsManager::MappingsMeta MappingsManager::readProjectMappings(const ProjectExplorer::Project &project) const
{
    qDebug() << "Restoring project mappings:" << project.displayName();

    MappingsMeta mappings{new QStandardItemModel(0, Constants::MAP_COLUMNS_COUNT), {}};

    auto items = project.namedSettings(QLatin1String(Constants::SETTINGS_GROUP)).toMap()
                        .value(QLatin1String(Constants::MAPPINGS_GROUP)).toList();

    for (auto &item : items) {
        mappings.mappings.emplace_back(Mapping::fromMap(item.toMap(), *mappings.storage));
    }

    return mappings;
}

void MappingsManager::saveProjectMappings(ProjectExplorer::Project &project)
{
    if (!m_mappings.contains(&project)) return;

    qDebug() << "Saving project mappings:" << project.displayName();

    QVariantList list;
    for (const auto &mapping : m_mappings[&project].mappings) {
       list.append(mapping.toMap());
    }

    static const auto group = QLatin1String(Constants::SETTINGS_GROUP);
    auto settings = project.namedSettings(group).toMap();
    settings.insert(QLatin1String(Constants::MAPPINGS_GROUP), list);
    project.setNamedSettings(group, settings);
}

MappingsManager::MappingsMeta &MappingsManager::mappingsMetaForProject(ProjectExplorer::Project &project)
{
    // if project is just being opened we need to read it's settings
    if (!m_mappings.contains(&project)) {
        auto mappings = m_mappings.insert(&project, readProjectMappings(project));
        mappings->storage->setParent(this);

        // make sure the settings are going to be saved for this project
        connect(&project, &ProjectExplorer::Project::aboutToSaveSettings,
                [this, &project]() { this->saveProjectMappings(project); });

    }

    return m_mappings[&project];
}

} // namespace Internal
} // namespace RemoteDev
