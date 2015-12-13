#include "projectsettingswidget.h"
#include "ui_projectsettingswidget.h"

#include <QStandardItemModel>
//#include <QItemDelegate> // TBD: which one to use?
#include <QStyledItemDelegate>
#include <QDataWidgetMapper>

#include <QDebug>

#include "remotedevconstants.h"

using namespace RemoteDev::Internal;

ProjectSettingsWidget::ProjectSettingsWidget(ProjectExplorer::Project *prj) :
    ui(new Ui::ProjectSettingsWidget),
    m_project(prj),
    m_mapper(new QDataWidgetMapper(this))
{
    ui->setupUi(this);

    auto model = new QStandardItemModel(0, Constants::MAP_COLUMNS_COUNT);

    m_mapper->setModel(model);
    // FIXME: implement devices model
    m_mapper->addMapping(ui->cbxDevice, Constants::MAP_DEVICE_COLUMN);
    m_mapper->addMapping(ui->edtPath, Constants::MAP_PATH_COLUMN);
    m_mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

    ui->tblMappings->setModel(model);
    ui->tblMappings->setColumnHidden(Constants::MAP_DEVICE_COLUMN, true);
    ui->tblMappings->setColumnHidden(Constants::MAP_PATH_COLUMN, true);

    connect(ui->tblMappings->selectionModel(), &QItemSelectionModel::currentRowChanged,
            m_mapper, &QDataWidgetMapper::setCurrentModelIndex);

    connect(m_project, &ProjectExplorer::Project::aboutToSaveSettings,
            this, &ProjectSettingsWidget::saveSettings);

    // TBD: is this required?
    //connect(m_project, &ProjectExplorer::Project::settingsLoaded,
    //        this, &ProjectSettingsWidget::initData);
    this->initData();
}

ProjectSettingsWidget::~ProjectSettingsWidget()
{
    delete m_mapper;
    delete ui;
}

void ProjectSettingsWidget::newMapping()
{
    qDebug() << "new mapping for project:" << m_project->displayName();

    static int i = 0;
    this->createMapping(QStringLiteral("<new mapping>"),
                        true,
                        QString(),
                        QStringLiteral("test%1").arg(i++));
}

void ProjectSettingsWidget::removeMapping()
{
    auto model = qobject_cast<QStandardItemModel *>(ui->tblMappings->model());
    auto indexes = ui->tblMappings->selectionModel()->selectedRows();

    // UI restricts selection to one row at a time, but let's support more
    for (const auto &index : indexes) {
        model->removeRow(index.row());
    }
    // TODO: when no items left -> clear form
}

void ProjectSettingsWidget::initData()
{
    qDebug() << "Initializing project settings:" << m_project->displayName();

    auto settings = m_project->namedSettings(QLatin1String(Constants::SETTINGS_GROUP)).toMap();
    auto section = settings.value(QLatin1String(Constants::MAPPINGS_GROUP)).toMap();

    qDebug() << "Settings:" << settings;

    // NOTE: lines below cause setColumnHidden() to lose effect
    //auto model = qobject_cast<QStandardItemModel *>(ui->tblMappings->model());
    //model->clear();

    for (auto &name : section.keys()) {
        qDebug() << "Restoring mapping" << name;
        auto config = section.value(name).toMap();

        this->createMapping(config[QStringLiteral("name")].toString(),
                            config[QStringLiteral("enabled")].toBool(),
                            config[QStringLiteral("device")].toString(),
                            config[QStringLiteral("path")].toString());
    }
}

void ProjectSettingsWidget::saveSettings()
{
    qDebug() << "Saving settings for" << m_project->displayName();

    QVariantMap mappings;
    auto model = qobject_cast<QStandardItemModel *>(ui->tblMappings->model());
    for (int i = 0; i < model->rowCount(); i++) {
        const auto &name = model->item(i, 0)->text();

        qDebug() << "Saving mapping:" << name;

        // FIXME: name is supposed to be unique?
        mappings[name] = QVariantMap({
            { QStringLiteral("name"),    name },
            { QStringLiteral("enabled"), model->item(i, 1)->data(Qt::CheckStateRole) },
            // FIXME: are device ID's fixed?
            { QStringLiteral("device"),  model->item(i, 2)->text() },
            { QStringLiteral("path"),    model->item(i, 3)->text() }
        });
    }

    auto settings = m_project->namedSettings(QLatin1String(Constants::SETTINGS_GROUP)).toMap();
    settings[QLatin1String(Constants::MAPPINGS_GROUP)] = mappings;
    m_project->setNamedSettings(QLatin1String(Constants::SETTINGS_GROUP), settings);
}

QStandardItemModel * ProjectSettingsWidget::mappingsModel()
{
    return qobject_cast<QStandardItemModel *>(ui->tblMappings->model());
}

void ProjectSettingsWidget::createMapping(const QString &name, bool enabled,
                                          const QString &device, const QString &path)
{
    auto nameItem = new QStandardItem(name);

    auto enabledItem = new QStandardItem();
    enabledItem->setData(enabled, Qt::CheckStateRole);

    auto deviceItem = new QStandardItem(device);

    auto pathItem = new QStandardItem(path);

    auto model = qobject_cast<QStandardItemModel *>(ui->tblMappings->model());
    model->appendRow({ nameItem, enabledItem, deviceItem, pathItem });
}
