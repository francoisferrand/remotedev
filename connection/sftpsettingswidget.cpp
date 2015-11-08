#include "sftpsettingswidget.h"
#include "ui_sftpsettingswidget.h"

#include <QStandardItemModel>

using namespace RemoteDev;

SftpSettingsWidget::SftpSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SftpSettingsWidget)
{
    ui->setupUi(this);

    // FIXME: password, host-based and keyboard authentification types are not implemented
    auto *model = qobject_cast<const QStandardItemModel *>(ui->comboAuthType->model());
    for (int i : { 0, 2, 3 }) {
        QStandardItem *item = model->item(i);
        item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
        item->setData(ui->comboAuthType->palette().color(QPalette::Disabled, QPalette::Text),
                      Qt::TextColorRole);
    }
}

SftpSettingsWidget::~SftpSettingsWidget()
{
    delete ui;
}

QString SftpSettingsWidget::hostname() const { return ui->editHost->text();   }
quint16 SftpSettingsWidget::port()     const { return ui->spinPort->value();  }
int     SftpSettingsWidget::tmout()    const { return ui->spinTmout->value(); }
QString SftpSettingsWidget::user()     const { return ui->editUser->text();   }
