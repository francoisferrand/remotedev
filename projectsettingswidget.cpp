#include "projectsettingswidget.h"
#include "ui_projectsettingswidget.h"

#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QDataWidgetMapper>

#include "remotedevconstants.h"

#include <QDebug>

using namespace RemoteDev::Internal;

class MappingSettingsDelegate : public QStyledItemDelegate
{
public:
    MappingSettingsDelegate(Ui::ProjectSettingsWidget *ui,
                            QObject *parent = 0) :
        QStyledItemDelegate(parent),
        ui(ui)
    {}

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        if (editor == qobject_cast<QWidget *>(ui->cbxDevice)) {
            auto idStr = index.data().toString();

            auto model = qobject_cast<QStandardItemModel *>(ui->cbxDevice->model());
            auto items = model->findItems(idStr, Qt::MatchExactly,
                                          RemoteDev::Constants::DEV_ID_COLUMN);
            if (items.count() > 0) {
                qDebug() << "setEditorData(cbxDevice):" << idStr << "index:" << items.at(0)->row();
                // FIXME: warn if items.count() > 1
                ui->cbxDevice->setCurrentIndex(items.at(0)->row());
            } else {
                qDebug() << "setEditorData(cbxDevice): -1";
                ui->cbxDevice->setCurrentIndex(-1); // clear the combo
            }
        } else {
            QStyledItemDelegate::setEditorData(editor, index);
        }
    }

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const
    {
        if (editor == qobject_cast<QWidget *>(ui->cbxDevice)) {
            auto id = qobject_cast<QStandardItemModel *>(ui->cbxDevice->model())
                        ->item(ui->cbxDevice->currentIndex(),
                               RemoteDev::Constants::DEV_ID_COLUMN)
                        ->data(RemoteDev::Constants::DEV_ID_ROLE);

            qDebug() << "setModelData(cbxDevice):" << id;

            auto item = qobject_cast<QStandardItemModel *>(model)->itemFromIndex(index);
            if (id.isValid()) {
                item->setText(id.toString());
                item->setData(id, RemoteDev::Constants::DEV_ID_ROLE);
            }
        } else {
            QStyledItemDelegate::setModelData(editor, model, index);
        }
    }

private:
    Ui::ProjectSettingsWidget *ui;
};

ProjectSettingsWidget::ProjectSettingsWidget(ProjectExplorer::Project *prj) :
    ui(new Ui::ProjectSettingsWidget),
    m_project(prj),
    m_mapper(new QDataWidgetMapper(this))
{
    ui->setupUi(this);

    auto model = new QStandardItemModel(0, Constants::MAP_COLUMNS_COUNT);

    m_mapper->setModel(model);
    m_mapper->setItemDelegate(new MappingSettingsDelegate(ui, m_mapper));

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

    this->createMapping(QStringLiteral("<mapping name>"),
                        true,
                        Core::Id::fromString(QStringLiteral("<device>")),
                        QStringLiteral("<path>"));
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
                            Core::Id::fromSetting(config[QStringLiteral("device")]),
                            config[QStringLiteral("path")].toString());
    }
}

void ProjectSettingsWidget::saveSettings()
{
    qDebug() << "Saving mappings for" << m_project->displayName();

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
            { QStringLiteral("device"),  model->item(i, 2)->data(Constants::DEV_ID_ROLE) },
            { QStringLiteral("path"),    model->item(i, 3)->text() }
        });

        // TODO: check if device exists
    }

    auto settings = m_project->namedSettings(QLatin1String(Constants::SETTINGS_GROUP)).toMap();
    settings[QLatin1String(Constants::MAPPINGS_GROUP)] = mappings;
    m_project->setNamedSettings(QLatin1String(Constants::SETTINGS_GROUP), settings);
}

QStandardItemModel * ProjectSettingsWidget::mappingsModel()
{
    return qobject_cast<QStandardItemModel *>(ui->tblMappings->model());
}

void ProjectSettingsWidget::setDevicesModel(QStandardItemModel *devices)
{
    ui->cbxDevice->setModel(devices);
    ui->cbxDevice->setModelColumn(Constants::DEV_NAME_COLUMN);
    ui->cbxDevice->setCurrentIndex(-1); // clear the combo
}

void ProjectSettingsWidget::createMapping(const QString &name, bool enabled,
                                          const Core::Id &device, const QString &path)
{
    auto nameItem = new QStandardItem(name);

    auto enabledItem = new QStandardItem();
    enabledItem->setData(enabled, Qt::CheckStateRole);

    auto deviceItem = new QStandardItem(device.toString());
    deviceItem->setData(device.toSetting(), Constants::DEV_ID_ROLE);

    auto pathItem = new QStandardItem(path);

    auto model = qobject_cast<QStandardItemModel *>(ui->tblMappings->model());
    model->appendRow({ nameItem, enabledItem, deviceItem, pathItem });
}
