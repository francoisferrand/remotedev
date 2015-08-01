#include "remotedevplugin.h"
#include "remotedevconstants.h"

#include "connectionmanager.h"

#include <QAction>
#include <QMenu>
#include <QTime>

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/documentmanager.h>
#include <coreplugin/idocument.h>
#include <coreplugin/messagemanager.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>

//#include <texteditor/texteditor.h>
//#include <texteditor/textdocument.h>

using namespace RemoteDev::Internal;

//using TextEditor::BaseTextEditor;
using Core::EditorManager;
using Core::DocumentManager;
using Core::ActionManager;
using Core::MessageManager;

RemoteDevPlugin::RemoteDevPlugin() :
    m_saveAction(nullptr),
    m_saveAsAction(nullptr)
{
    (void) new ConnectionManager(this);
}

RemoteDevPlugin::~RemoteDevPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
    delete ConnectionManager::instance();
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
//    connect(editorManager, SIGNAL(editorOpened(Core::IEditor*)), this, SLOT(onEditorOpened(Core::IEditor*)));
    connect(editorManager, SIGNAL(currentEditorChanged(Core::IEditor*)), this, SLOT(onEditorOpened(Core::IEditor*)));

    m_saveAction = ActionManager::command(Core::Constants::SAVE)->action();
    connect(m_saveAction, SIGNAL(triggered(bool)), this, SLOT(onSaveAction()));

    ConnectionManager *connectionManager = ConnectionManager::instance();
    connect(connectionManager, &ConnectionManager::connectionError, this, &RemoteDevPlugin::onConnectionError);

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

    // TODO: read configuration, create connections, install handlers

    return false;
}

ExtensionSystem::IPlugin::ShutdownFlag RemoteDevPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    // If required to wait, until external process finishes
    // return AsynchronousShutdown;
    // emit asynchronousShutdownFinished();

    // FIXME: remote disconnect asynchronous?
    disconnect(this, SLOT(onConnectionError(RemoteConnection::SharedPointer)));
    disconnect(this, SLOT(onEditorOpened(Core::IEditor*)));

    return SynchronousShutdown;
}

void RemoteDevPlugin::uploadCurrentDocument()
{
    Core::IDocument *document = EditorManager::currentDocument();

    if (document) {
        QString name = document->displayName();
        this->showDebug(tr("Starting file upload") + QString::fromLatin1(": ") + name);

        const auto &local = document->filePath();
        const auto remote = Utils::FileName::fromString(QString::fromLatin1("/tmp/") + local.fileName());

        auto connection = ConnectionManager::connectionForAlias(QString::fromLatin1("localhost"));

        // TODO: time these

        // FIXME: move handler installation to the delayedInitialize()
        // since now there is only one connection -> this is okay
        static bool handlerInstalled = false;
        if (! handlerInstalled) {
            handlerInstalled = true;
            connect(connection.data(), &RemoteConnection::uploadFinished,
                    [this, name] () -> void {
                        this->showDebug(QString::fromLatin1("%1: %2").arg(name, tr("success")));
                    });
            connect(connection.data(), &RemoteConnection::uploadError,
                    [this, name] () -> void {
                        this->showDebug(QString::fromLatin1("%1: %2").arg(name, tr("failure")));
                    });
        }

        connection->uploadFile(local, remote, OverwriteExisting);
    }
}

void RemoteDevPlugin::triggerAction()
{
    this->showDebug(tr("Action triggered"));
}

void RemoteDevPlugin::onConnectionError(RemoteConnection::SharedPointer connection)
{
    if (connection) {
        this->showDebug(tr("Remote connection error")
                        + QString::fromLatin1(": ")
                        + connection->errorString());
    } else {
        this->showDebug(tr("Remote connection error"));
    }
}

void RemoteDevPlugin::onEditorOpened(Core::IEditor *editor)
{
    if (editor) {
        QString className = QString::fromLatin1(editor->metaObject()->className());
        this->showDebug(tr("Editor opened: ") + className);

        uploadCurrentDocument();
    }
}

void RemoteDevPlugin::onSaveAction()
{
    uploadCurrentDocument();
}

void RemoteDevPlugin::showDebug(const QString &string) const
{
    MessageManager *messageManager = qobject_cast<MessageManager *>(MessageManager::instance());
    messageManager->write(string, { MessageManager::NoModeSwitch });
}
