#ifndef PROJECTSETTINGSWIDGET_H
#define PROJECTSETTINGSWIDGET_H

#include <QWidget>

#include <projectexplorer/project.h>

namespace RemoteDev {
namespace Internal {

namespace Ui {
class ProjectSettingsWidget;
}

class ProjectSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectSettingsWidget(ProjectExplorer::Project *prj);

    ~ProjectSettingsWidget();

private slots:
    void newMapping();
    void initData();
    void saveSettings();

private:
    void createMapping(const QString &name, bool enabled,
                       const QString &device, const QString &path);

private:
    Ui::ProjectSettingsWidget *ui;
    ProjectExplorer::Project *m_project;
};

} // namespace Internal
} // namespace RemoteDev

#endif // PROJECTSETTINGSWIDGET_H
