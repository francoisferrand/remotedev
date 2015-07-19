#ifndef REMOTEDEV_H
#define REMOTEDEV_H

#include "remotedev_global.h"

#include <extensionsystem/iplugin.h>

#include <ssh/sshconnection.h>

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

private slots:
    void triggerAction();

    void onSshConnected();
    void onSshDisconnected();
    void onSshError(QSsh::SshError error);
private:
    QSsh::SshConnection *m_connection;
};

} // namespace Internal
} // namespace RemoteDev

#endif // REMOTEDEV_H

