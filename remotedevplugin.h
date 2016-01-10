#ifndef REMOTEDEV_H
#define REMOTEDEV_H

#include "remotedev_global.h"

#include <QHash>

#include <extensionsystem/iplugin.h>

#include "connection.h"

QT_BEGIN_NAMESPACE
class QTime;
class QAction;
QT_END_NAMESPACE

namespace Core { class IEditor; }
namespace ProjectExplorer { class Project; }

namespace RemoteDev {
namespace Internal {

class ConnectionsPage;
class ConnectionManager;
class MappingsManager;
class DeviceManager;

class RemoteDevPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "RemoteDev.json")

public:
    RemoteDevPlugin();
    ~RemoteDevPlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    bool delayedInitialize();
    ShutdownFlag aboutToShutdown();

public slots:
    void uploadCurrentDocument();
    void uploadCurrentNode();

private slots:
    void triggerAction();

    // ConnectionManager
    void onConnectionError(Connection::Ptr connection);

    // EditorManager
    void onEditorOpened(Core::IEditor *);

    // ActionManager
    void onSaveAction();

    void uploadFile(const Utils::FileName &file,
                    ProjectExplorer::Project *project = nullptr);

private:
    void createOptionsPage();
    void createProjectSettingsPage();
    void createFileMenus();

    void showDebug(const QString &string) const;

private:
    ConnectionsPage *m_optionsPage;

    QAction *m_uploadFile;
    QAction *m_uploadDirectory;

    QHash<RemoteJobId, QSharedPointer<QTime>> m_timers;

    ConnectionManager *m_connManager;
    MappingsManager   *m_mapManager;
    DeviceManager     *m_devManager;
};

} // namespace Internal
} // namespace RemoteDev

#endif // REMOTEDEV_H

