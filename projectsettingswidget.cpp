#include "projectsettingswidget.h"
#include "ui_projectsettingswidget.h"

#include "mappingsmanager.h"
#include "remotedevconstants.h"

#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QDataWidgetMapper>


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
                // FIXME: warn if items.count() > 1
                ui->cbxDevice->setCurrentIndex(items.at(0)->row());
            } else {
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

            if (id.isValid()) {
                auto item = qobject_cast<QStandardItemModel *>(model)->itemFromIndex(index);
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

ProjectSettingsWidget::ProjectSettingsWidget(ProjectExplorer::Project &project,
                                             MappingsManager &mapManager) :
    ui(new Ui::ProjectSettingsWidget),
    m_mapper(new QDataWidgetMapper(this)),
    m_project(project),
    m_mapManager(mapManager)
{
    ui->setupUi(this);

    qDebug() << "Settings widget for project" << project.displayName();

    m_mapper->setItemDelegate(new MappingSettingsDelegate(ui, m_mapper));
    m_mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

    setMappingsModel(m_mapManager.storageForProject(m_project));
}

ProjectSettingsWidget::~ProjectSettingsWidget()
{
    delete m_mapper;
    delete ui;
}

void ProjectSettingsWidget::newMapping()
{
    qDebug() << "new mapping for project:" << m_project.displayName();

    static const auto initialName = QStringLiteral("<mapping name>");
    static const auto initialDevice = Core::Id::fromString(QStringLiteral("<device>"));
    static const auto initialPath = QStringLiteral("<path>");

    m_mapManager.addMapping(m_project, initialName, true, initialDevice, initialPath);
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

void ProjectSettingsWidget::setMappingsModel(QStandardItemModel &mappings)
{
    m_mapper->setModel(&mappings);
    m_mapper->addMapping(ui->cbxDevice, Constants::MAP_DEVICE_COLUMN);
    m_mapper->addMapping(ui->edtPath, Constants::MAP_PATH_COLUMN);

    ui->tblMappings->setModel(&mappings);
    ui->tblMappings->setColumnHidden(Constants::MAP_DEVICE_COLUMN, true);
    ui->tblMappings->setColumnHidden(Constants::MAP_PATH_COLUMN, true);

    connect(ui->tblMappings->selectionModel(), &QItemSelectionModel::currentRowChanged,
            m_mapper, &QDataWidgetMapper::setCurrentModelIndex);
}

void ProjectSettingsWidget::setDevicesModel(QStandardItemModel *devices)
{
    ui->cbxDevice->setModel(devices);
    ui->cbxDevice->setModelColumn(Constants::DEV_NAME_COLUMN);
    ui->cbxDevice->setCurrentIndex(-1); // clear the combo
}

void ProjectSettingsWidget::createMapping(const QString &name,
                                          bool enabled,
                                          const Core::Id &device,
                                          const QString &path)
{
    m_mapManager.addMapping(m_project, name, enabled, device, path);
}
