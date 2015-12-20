#include "remotedevplugin.h"
#include "remotedevconstants.h"

#include <QAction>
#include <QMenu>
#include <QTime>
#include <QStandardItemModel>

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

#include <ssh/sshconnection.h>

#include "connectionmanager.h"
#include "connectionspage.h"
#include "connection/sftpoptionspage.h"
#include "projectsettingswidget.h"
#include "deviceshelper.h"
#include "mappingsmanager.h"

#include <QDebug>

using namespace RemoteDev::Internal;

RemoteDevPlugin::RemoteDevPlugin() :
    m_connManager(new ConnectionManager),
    m_mapManager(new MappingsManager),
    m_devices(new QStandardItemModel(0, Constants::DEV_COLUMNS_COUNT))
{}

RemoteDevPlugin::~RemoteDevPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
    delete m_connManager;
    delete m_mapManager;
    delete m_devices;
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
    Core::Command *cmd = Core::ActionManager::registerAction(
                action, Constants::ACTION_ID,
                //Core::Context(Core::Constants::C_NAVIGATION_PANE)
                Core::Context(Core::Constants::C_GLOBAL)
    );
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, SIGNAL(triggered()), this, SLOT(triggerAction()));

    Core::ActionContainer *menu = Core::ActionManager::createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("RemoteDev"));
    menu->addAction(cmd);
    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

    auto *editorManager = Core::EditorManager::instance();
    // NOTE: currentEditorChanged is also triggered upon editorOpened
    //connect(editorManager, &Core::EditorManager::editorOpened,
    //        this, &RemoteDevPlugin::onEditorOpened);
    connect(editorManager, &Core::EditorManager::currentEditorChanged,
            this, &RemoteDevPlugin::onEditorOpened);

    QAction *saveAction = Core::ActionManager::command(Core::Constants::SAVE)->action();
    connect(saveAction, &QAction::triggered,
            this, &RemoteDevPlugin::onSaveAction);

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

    auto helper = new DevicesHelper(m_devices, this);
    helper->startDeviceSync();

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

    // TODO: disconnect all existing connections?
    // Should it be done asynchronously?

    disconnect(this, SLOT(onConnectionError(Connection::Ptr)));
    disconnect(this, SLOT(onEditorOpened(Core::IEditor*)));

    return SynchronousShutdown;
}

void RemoteDevPlugin::uploadCurrentDocument()
{
    Core::IDocument *document = Core::EditorManager::currentDocument();
    if (! document) return;

    const auto &local = document->filePath();
    const auto project = ProjectExplorer::SessionManager::projectForFile(local);
    if (! project)
        return;

    // TODO: mappings should be created even if widget was not created
    auto mappings = m_mapManager->mappingsForProject(project);
    if (! mappings)
        return;

    auto deviceMgr = ProjectExplorer::DeviceManager::instance();
    for (int i = 0; i < mappings->rowCount(); i++) {
        auto idSetting = mappings->item(i, Constants::MAP_DEVICE_COLUMN)->data(Constants::DEV_ID_ROLE);
        auto device = deviceMgr->find(Core::Id::fromSetting(idSetting));

        auto connection = ConnectionManager::connectionForDevice(device.data());
        if (connection.isNull()) {
            qDebug() << "No connection for mapping \"" << mappings->item(i, Constants::MAP_NAME_COLUMN)->text()
                     << "and device" << device->displayName();
            continue;
        }

        auto remote = Utils::FileName::fromString(mappings->item(i, Constants::MAP_PATH_COLUMN)->text());
        remote.appendPath(project->displayName()); // only for debug!
        remote.appendPath(local.relativeChildPath(project->projectDirectory()).toString());

        showDebug(QStringLiteral("%1: Upload %2 to %3").arg(device->displayName(), local.fileName(), remote.toString()));

        static QSet<QString> hasHandler;
        if (! hasHandler.contains(connection->alias())) {
            connect(connection.data(), &Connection::uploadFinished,
                [this, connection, local] (RemoteJobId job) -> void {
                    auto timer = m_timers.take(job);
                    int elapsed = timer ? timer->elapsed() : -1;

                    this->showDebug(QString::fromLatin1("%1: %2 [%3 ms]")
                                    .arg(connection->alias(), tr("success"), QString::number(elapsed)));
                }
            );
            connect(connection.data(), &Connection::uploadError,
                [this, connection, local] (RemoteJobId job, const QString &reason) -> void {
                    auto timer = m_timers.take(job);
                    int elapsed = timer ? timer->elapsed() : -1;

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

void RemoteDevPlugin::createProjectSettingsPage()
{
    auto panelFactory = new ProjectExplorer::ProjectPanelFactory();
    panelFactory->setPriority(100); // this sets panel order in tabs
    panelFactory->setDisplayName(tr("RemoteDev"));

//    QIcon icon /* = QIcon(QLatin1String(":/projectexplorer/images/EditorSettings.png")) */;
//    panelFactory->setSimpleCreateWidgetFunction<ProjectSettingsWidget>(icon);

    panelFactory->setCreateWidgetFunction(
        [this, panelFactory] (ProjectExplorer::Project *project) -> QWidget * {
            auto panel = new ProjectExplorer::PropertiesPanel ();
            panel->setDisplayName(tr("Remote Mappings"));

            // TODO: pass devices model
            auto widget = new ProjectSettingsWidget(project);

            widget->setMappingsModel(m_mapManager->mappingsForProject(project));
            widget->setDevicesModel(m_devices);

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

