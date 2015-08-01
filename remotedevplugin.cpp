#include "remotedevplugin.h"
#include "remotedevconstants.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>

//#include <texteditor/texteditor.h>
//#include <texteditor/textdocument.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/documentmanager.h>
#include <coreplugin/idocument.h>
#include <coreplugin/messagemanager.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>

#include <QtPlugin>

using namespace RemoteDev::Internal;

//using TextEditor::BaseTextEditor;
using Core::EditorManager;
using Core::DocumentManager;
using Core::ActionManager;
using Core::MessageManager;

RemoteDevPlugin::RemoteDevPlugin() :
    m_connection(nullptr),
    m_saveAction(nullptr),
    m_saveAsAction(nullptr)
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

    // NOTE: currentEditorChanged is also triggered upon editorOpened
    EditorManager *editorManager = EditorManager::instance();
    connect(editorManager, SIGNAL(editorOpened(Core::IEditor*)), this, SLOT(onEditorChanged(Core::IEditor*)));
//    connect(editorManager, SIGNAL(currentEditorChanged(Core::IEditor*)), this, SLOT(onEditorChanged(Core::IEditor*)));

    m_saveAction = ActionManager::command(Core::Constants::SAVE)->action();
    connect(m_saveAction, SIGNAL(triggered(bool)), this, SLOT(onSaveAction(bool)));


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
    params.privateKeyFile = QString::fromLatin1(
                "/home/elvenfighter/Projects/keys/localhost.rsa");
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

//    DocumentManager *documentManager = DocumentManager::instance();

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

void RemoteDevPlugin::uploadCurrentDocument()
{
//    DocumentManager::instance()->e
    Core::IDocument *document = EditorManager::currentDocument();

    if (document) {
        QString name = document->displayName();
        this->showDebug(tr("Uploading file: ") + name);

//        const Utils::FileName = document->filePath();
//        QSharedPointer<QSsh::SftpChannel> channel = m_connection->createSftpChannel();
    }
}

void RemoteDevPlugin::triggerAction()
{
    this->showDebug(tr("Connecting to host."));

    m_connection->connectToHost();
}

void RemoteDevPlugin::onSshConnected()
{
    this->showDebug(tr("SSH connection successfull."));

    m_connection->disconnectFromHost();
}

void RemoteDevPlugin::onSshDisconnected()
{
    this->showDebug(tr("SSH disconnected."));
}

void RemoteDevPlugin::onSshError(QSsh::SshError error)
{
    Q_UNUSED(error);

    this->showDebug(tr("SSH connection error: ") + m_connection->errorString());

    if (m_connection->state() != QSsh::SshConnection::Unconnected) {
        m_connection->disconnectFromHost();
    }
}

void RemoteDevPlugin::onEditorChanged(Core::IEditor *editor)
{
    QString className = QString::fromLatin1(editor->metaObject()->className());
    this->showDebug(tr("Editor changed: ")  + className);

    this->uploadCurrentDocument();
}

void RemoteDevPlugin::onSaveAction(bool checked)
{
    Q_UNUSED(checked);
    this->uploadCurrentDocument();
}

void RemoteDevPlugin::showDebug(const QString &string) const
{
    MessageManager *messageManager = qobject_cast<MessageManager *>(MessageManager::instance());
    messageManager->write(string, { MessageManager::NoModeSwitch, MessageManager::WithFocus });
}

