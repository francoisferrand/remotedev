#include "remotedevplugin.h"
#include "remotedevconstants.h"

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

#include <projectexplorer/devicesupport/devicemanager.h>
#include <projectexplorer/project.h>
#include <projectexplorer/session.h>
#include <projectexplorer/projectpanelfactory.h>

#include "connectionmanager.h"
#include "connectionspage.h"
#include "connection/sftpoptionspage.h"
#include "projectsettingswidget.h"

using namespace RemoteDev::Internal;

//using Core::EditorManager;
//using Core::ActionManager;
//using Core::MessageManager;

RemoteDevPlugin::RemoteDevPlugin()
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
    auto *editorManager = Core::EditorManager::instance();
    // connect(editorManager, SIGNAL(editorOpened(Core::IEditor*)), this, SLOT(onEditorOpened(Core::IEditor*)));
    connect(editorManager, SIGNAL(currentEditorChanged(Core::IEditor*)), this, SLOT(onEditorOpened(Core::IEditor*)));

    QAction *saveAction = Core::ActionManager::command(Core::Constants::SAVE)->action();
    connect(saveAction, SIGNAL(triggered(bool)), this, SLOT(onSaveAction()));

    ConnectionManager *connectionManager = ConnectionManager::instance();
    connect(connectionManager, &ConnectionManager::connectionError,
            this, &RemoteDevPlugin::onConnectionError);

    createOptionsPage();
    createProjectSettingsPage();

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

    // TODO: disconnect all existing connections?
    // Should it be done asynchronously?

    disconnect(this, SLOT(onConnectionError(Connection::Ptr)));
    disconnect(this, SLOT(onEditorOpened(Core::IEditor*)));

    return SynchronousShutdown;
}

// FIXME: this is only for testing DeviceManager
//#include <ssh/sshconnection.h>

void RemoteDevPlugin::uploadCurrentDocument()
{
    Core::IDocument *document = Core::EditorManager::currentDocument();
    if (! document) return;

    const auto &local = document->filePath();

    Utils::FileName relPath;
    auto remote = Utils::FileName::fromString(QStringLiteral("/tmp"));

    // TODO: move this to initialize(), depend on ProjectExplorer
    auto projects = ProjectExplorer::SessionManager::projects();
    for (auto &project : projects) {
        Utils::FileName dir = project->projectDirectory();
        if (local.isChildOf(dir)) {
            // FIXME: only for debug
            remote.appendPath(project->displayName());

            remote.appendPath(local.relativeChildPath(dir).toString());
            break;
        }
    }

    showDebug(QStringLiteral("Upload %1 to %2").arg(local.fileName(), remote.toString()));

    auto *mgr = ProjectExplorer::DeviceManager::instance();
    for (int i = 0; i < mgr->deviceCount(); i++) {
        auto device = mgr->deviceAt(i);
//        bool canCreateProcess() const { return true; }
//        ProjectExplorer::DeviceProcess *createProcess(QObject *parent) const;
//        bool canAutoDetectPorts() const;
//        ProjectExplorer::PortsGatheringMethod::Ptr portsGatheringMethod() const;
//        bool canCreateProcessModel() const { return true; }
//        ProjectExplorer::DeviceProcessList *createProcessListModel(QObject *parent) const;

        auto connection = ConnectionManager::connectionForDevice(device.data());
        if (connection.isNull())
            continue;

        static QSet<QString> hasHandler;
        if (! hasHandler.contains(connection->alias())) {
            connect(connection.data(), &Connection::uploadFinished,
                [this, connection, local] (RemoteJobId job) -> void {
                    auto timer = m_timers.take(job);
                    int elapsed = timer ? timer->elapsed() : 0;

                    this->showDebug(QString::fromLatin1("%1: %2 [%3 ms]")
                                    .arg(connection->alias(), tr("success"), QString::number(elapsed)));
                }
            );
            connect(connection.data(), &Connection::uploadError,
                [this, connection, local] (RemoteJobId job, const QString &reason) -> void {
                    auto timer = m_timers.take(job);
                    int elapsed = timer ? timer->elapsed() : 0;

                    this->showDebug(QString::fromLatin1("%1: %2 [%3 ms]: %4")
                                    .arg(connection->alias(), tr("failure"), QString::number(elapsed), reason));
                }
            );
            hasHandler.insert(connection->alias());
        }

        auto timer = QSharedPointer<QTime>(new QTime);
        timer->start();
        RemoteJobId job = connection->uploadFile(local, remote, OverwriteExisting);
        m_timers.insert(job, timer);
    }
}

void RemoteDevPlugin::uploadCurrentDocument1()
{
    Core::IDocument *document = Core::EditorManager::currentDocument();

    if (document) {
        QString name = document->displayName();

        const auto &local = document->filePath();
        const auto remote = Utils::FileName::fromString(QString::fromLatin1("/tmp/") + local.fileName());

        auto connection = ConnectionManager::connectionForAlias(QString::fromLatin1("localhost"));

        // FIXME: move handler installation to the delayedInitialize()
        // since now there is only one connection -> this is okay
        static bool handlerInstalled = false;
        if (! handlerInstalled) {
            handlerInstalled = true;
            connect(connection.data(), &Connection::uploadFinished,
                    [this, name] (RemoteJobId job) -> void {
                        auto timer = m_timers.take(job);
                        int elapsed = timer ? timer->elapsed() : 0;

                        this->showDebug(QString::fromLatin1("%1: %2 [%3 ms]")
                                        .arg(name, tr("success"), QString::number(elapsed)));
                    });
            connect(connection.data(), &Connection::uploadError,
                    [this, name] (RemoteJobId job, const QString &reason) -> void {
                        auto timer = m_timers.take(job);
                        int elapsed = timer ? timer->elapsed() : 0;

                        this->showDebug(QString::fromLatin1("%1: %2 [%3 ms]: %4")
                                        .arg(name, tr("failure"), QString::number(elapsed), reason));
                    });
        }

        auto timer = QSharedPointer<QTime>(new QTime);
        timer->start();
        RemoteJobId job = connection->uploadFile(local, remote, OverwriteExisting);
        m_timers.insert(job, timer);
    }
}

void RemoteDevPlugin::triggerAction()
{
    this->showDebug(tr("Action triggered"));
}

void RemoteDevPlugin::onConnectionError(Connection::Ptr connection)
{
    if (connection) {
        this->showDebug(tr("Remote connection error")
                        + QString::fromLatin1(": ")
                        + connection->errorString());
    } else {
        this->showDebug(tr("Remote connection error"));
    }
}

void RemoteDevPlugin::onEditorOpened(Core::IEditor *)
{
    uploadCurrentDocument();
}

void RemoteDevPlugin::onSaveAction()
{
    uploadCurrentDocument();
}

void RemoteDevPlugin::createOptionsPage()
{
    // register bundled connection types (SSH)
    addAutoReleasedObject(new SftpOptionsPage(this));

    m_optionsPage = new ConnectionsPage(this);
    // TODO: load Core::ICore::settings();
    addAutoReleasedObject(m_optionsPage);
}

#include <QDebug>

void RemoteDevPlugin::createProjectSettingsPage()
{
    auto panelFactory = new ProjectExplorer::ProjectPanelFactory();
    panelFactory->setPriority(100); // FIXME: what does this do?
    panelFactory->setDisplayName(tr("RemoteDev"));

//    QIcon icon /* = QIcon(QLatin1String(":/projectexplorer/images/EditorSettings.png")) */;
//    panelFactory->setSimpleCreateWidgetFunction<ProjectSettingsWidget>(icon);

    panelFactory->setCreateWidgetFunction(
        [this, panelFactory] (ProjectExplorer::Project *project) -> QWidget * {
            auto panel = new ProjectExplorer::PropertiesPanel ();
            panel->setDisplayName(panelFactory->displayName());

            QObject *obj = qobject_cast<QObject *>(project);

            // FIXME at version 3.5.1 project argument is unusable
            // have to work around...
            qDebug() << "creating panel for:" << project->displayName();

            auto widget = new ProjectSettingsWidget(project);
            panel->setWidget(widget);

            auto panelsWidget = new ProjectExplorer::PanelsWidget();
            panelsWidget->addPropertiesPanel(panel);
            panelsWidget->setFocusProxy(widget);

            return panelsWidget;
        }
    );


    ProjectExplorer::ProjectPanelFactory::registerFactory(panelFactory);
}

void RemoteDevPlugin::showDebug(const QString &string) const
{
    auto *messageManager = qobject_cast<Core::MessageManager *>(Core::MessageManager::instance());
    messageManager->write(string, { Core::MessageManager::NoModeSwitch });
}

