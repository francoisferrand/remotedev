#ifndef REMOTEDEV_H
#define REMOTEDEV_H

#include "remotedev_global.h"

#include <QHash>

#include <extensionsystem/iplugin.h>

#include "connection.h"

QT_BEGIN_NAMESPACE
class QTime;
QT_END_NAMESPACE

namespace Core { class IEditor; }

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

private slots:
    void triggerAction();

    // ConnectionManager
    void onConnectionError(Connection::Ptr connection);

    // EditorManager
    void onEditorOpened(Core::IEditor *);

    // ActionManager
    void onSaveAction();

private:
    void createOptionsPage();
    void createProjectSettingsPage();
    void createFileMenus();

    void showDebug(const QString &string) const;

private:
    ConnectionsPage *m_optionsPage;

    QHash<RemoteJobId, QSharedPointer<QTime>> m_timers;

    ConnectionManager *m_connManager;
    MappingsManager   *m_mapManager;
    DeviceManager     *m_devManager;
};

} // namespace Internal
} // namespace RemoteDev

#endif // REMOTEDEV_H

