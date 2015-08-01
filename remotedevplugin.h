#ifndef REMOTEDEV_H
#define REMOTEDEV_H

#include "remotedev_global.h"

#include <extensionsystem/iplugin.h>

#include <ssh/sshconnection.h>

QT_BEGIN_NAMESPACE
class QAction;
QT_END_NAMESPACE

namespace Core { class IEditor; }

namespace RemoteDev {
namespace Internal {

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

    void onSshConnected();
    void onSshDisconnected();
    void onSshError(QSsh::SshError error);

    // EditorManager
    void onEditorChanged(Core::IEditor *editor);

    // ActionManager
    void onSaveAction(bool checked);
private:
    QSsh::SshConnection *m_connection;
    QAction *m_saveAction;
    QAction *m_saveAsAction;

    void showDebug(const QString &string) const;
};

} // namespace Internal
} // namespace RemoteDev

#endif // REMOTEDEV_H

