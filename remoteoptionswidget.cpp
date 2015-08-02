#include "remoteoptionswidget.h"
#include "ui_remoteoptionswidget.h"

using namespace RemoteDev::Internal;

RemoteOptionsWidget::RemoteOptionsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RemoteOptionsWidget)
{
    ui->setupUi(this);
}

RemoteOptionsWidget::~RemoteOptionsWidget()
{
    delete ui;
}
