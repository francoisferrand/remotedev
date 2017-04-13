#ifndef MAPPINGSMANAGER_H
#define MAPPINGSMANAGER_H

#include "mappingsmanager/mapping.h"

#include <coreplugin/id.h>
#include <utils/fileutils.h>

#include <QHash>
#include <QObject>

#include <vector>

QT_BEGIN_NAMESPACE
class QStandardItemModel;
QT_END_NAMESPACE

namespace ProjectExplorer { class Project; }

namespace RemoteDev {
namespace Internal {

class MappingsManager : public QObject
{
    Q_OBJECT
public:
    explicit MappingsManager(QObject *parent = 0);

    const std::vector<Mapping> &mappingsForProject(ProjectExplorer::Project &project);

    void createMapping(const ProjectExplorer::Project &project,
                       const QString &name,
                       bool isEnabled,
                       const Core::Id &deviceId,
                       const QString &remotePath);

    void removeMapping(const ProjectExplorer::Project &project,
                       std::uint16_t index);

private:
    friend class ProjectSettingsWidget;

    QStandardItemModel &storageForProject(ProjectExplorer::Project &project);

private:
    struct MappingsMeta {
        QStandardItemModel *storage;
        // QVector does not allow emplace_back
        std::vector<Mapping> mappings;
    };

    MappingsMeta readProjectMappings(const ProjectExplorer::Project &project) const;
    void saveProjectMappings(ProjectExplorer::Project &project);

    MappingsMeta &mappingsMetaForProject(ProjectExplorer::Project &project);

private:
    QHash<const ProjectExplorer::Project *, MappingsMeta> m_mappings;
};

} // namespace Internal
} // namespace RemoteDev

#endif // MAPPINGSMANAGER_H
