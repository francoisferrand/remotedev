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

class RemoteDevPlugin;

class ProjectSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectSettingsWidget(ProjectExplorer::Project *prj);

    ~ProjectSettingsWidget();

private slots:
    // ui handling
    void newMapping();
    void removeMapping();

    // ProjectExplorer::Project handling
    void initData();
    void saveSettings();

private:
    friend class RemoteDevPlugin;
    QStandardItemModel * mappingsModel();
    void setDevicesModel(QStandardItemModel *devices);

    void createMapping(const QString &name, bool enabled,
                       const Core::Id &device, const QString &path);

private:
    Ui::ProjectSettingsWidget *ui;
    ProjectExplorer::Project *m_project;
    QDataWidgetMapper *m_mapper;
};

} // namespace Internal
} // namespace RemoteDev

#endif // PROJECTSETTINGSWIDGET_H
