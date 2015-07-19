#include "remotedevplugin.h"
#include "remotedevconstants.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>

#include <QtPlugin>

using namespace RemoteDev::Internal;

RemoteDevPlugin::RemoteDevPlugin()
{
    // Create your members
}

RemoteDevPlugin::~RemoteDevPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
}

bool RemoteDevPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    // Register objects in the plugin manager's object pool
    // Load settings
    // Add actions to menus
    // Connect to other plugins' signals
    // In the initialize function, a plugin can be sure that the plugins it
    // depends on have initialized their members.

    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    QAction *action = new QAction(tr("RemoteDev action"), this);
    Core::Command *cmd = Core::ActionManager::registerAction(action, Constants::ACTION_ID,
//                                                             Core::Context(Core::Constants::C_NAVIGATION_PANE)
                                                             Core::Context(Core::Constants::C_GLOBAL)
                                                             );
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, SIGNAL(triggered()), this, SLOT(triggerAction()));

    Core::ActionContainer *menu = Core::ActionManager::createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("RemoteDev"));
    menu->addAction(cmd);
    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

    return true;
}

void RemoteDevPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
}

bool RemoteDevPlugin::delayedInitialize()
{
    // Perforn non-trivial startup sequence after application startup
    // Return true, if implemented
    QSsh::SshConnectionParameters params;
    params.host = QString::fromLatin1("localhost");
    params.userName = QString::fromLatin1("elvenfighter");

    params.timeout = 30;
    params.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypePublicKey;
    params.port = 22;
//    params.options &= ~QSsh::SshEnableStrictConformanceChecks;
//    params.hostKeyCheckingMode = QSsh::SshHostKeyCheckingStrict;
//    params.hostKeyDatabase = QSsh::SshHostKeyDatabasePtr::create();

    m_connection = new QSsh::SshConnection(params, this);

    connect(m_connection, SIGNAL(connected()), this, SLOT(onSshConnected()));
    connect(m_connection, SIGNAL(error(QSsh::SshError)),
            this, SLOT(onSshError(QSsh::SshError)));
    connect(m_connection, SIGNAL(disconnected()), this, SLOT(onSshDisconnected()));

    return true;
}

ExtensionSystem::IPlugin::ShutdownFlag RemoteDevPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    // If required to wait, until external process finishes
    // return AsynchronousShutdown;
    // emit asynchronousShutdownFinished();
    return SynchronousShutdown;
}

void RemoteDevPlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::mainWindow(),
                             tr("SSH connection"),
                             tr("Connecting to host."));
    m_connection->connectToHost();
}

void RemoteDevPlugin::onSshConnected()
{
    QMessageBox::information(Core::ICore::mainWindow(),
                             tr("SSH connection"),
                             tr("SSH connection successfull."));

    m_connection->disconnectFromHost();
}

void RemoteDevPlugin::onSshDisconnected()
{
    QMessageBox::information(Core::ICore::mainWindow(),
                             tr("SSH connection"),
                             tr("SSH disconnected."));
}

void RemoteDevPlugin::onSshError(QSsh::SshError error)
{
    Q_UNUSED(error);

    QMessageBox::information(Core::ICore::mainWindow(),
                             tr("SSH connection"),
                             tr("SSH connection error: ") + m_connection->errorString());
    if (m_connection->state() != QSsh::SshConnection::Unconnected) {
        m_connection->disconnectFromHost();
    }
}

