#ifndef PROJECTSETTINGSWIDGET_H
#define PROJECTSETTINGSWIDGET_H

#include <QWidget>

#include <projectexplorer/project.h>

QT_BEGIN_NAMESPACE
class QDataWidgetMapper;
class QStandardItemModel;
QT_END_NAMESPACE

namespace RemoteDev {
namespace Internal {

namespace Ui {
class ProjectSettingsWidget;
}

class MappingsManager;
class RemoteDevPlugin;

class ProjectSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectSettingsWidget(ProjectExplorer::Project &project,
                                   MappingsManager &mapManager);

    ~ProjectSettingsWidget();

private slots:
    // ui handling
    void newMapping();
    void removeMapping();

private:
    friend class RemoteDevPlugin;
    void setDevicesModel(QStandardItemModel *devices);

private:
    void setMappingsModel(QStandardItemModel &mappings);

private:
    Ui::ProjectSettingsWidget *ui;
    QDataWidgetMapper *m_mapper;

    ProjectExplorer::Project &m_project;
    MappingsManager &m_mapManager;
};

} // namespace Internal
} // namespace RemoteDev

#endif // PROJECTSETTINGSWIDGET_H
