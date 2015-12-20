#ifndef MAPPINGSMANAGER_H
#define MAPPINGSMANAGER_H

#include <QObject>
#include <QHash>

#include <coreplugin/id.h>

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

    QStandardItemModel *mappingsForProject(ProjectExplorer::Project *project);

private:
    QStandardItemModel *readProjectMappings(const ProjectExplorer::Project *project) const;
    void saveProjectMappings(ProjectExplorer::Project *project);

private:
    QHash<Core::Id, QStandardItemModel *> m_mappings;
};

} // namespace Internal
} // namespace RemoteDev

#endif // MAPPINGSMANAGER_H
