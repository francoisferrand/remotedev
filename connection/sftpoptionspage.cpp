#include "sftpoptionspage.h"

#include "../connectionconstants.h"

#include <QSettings>

using namespace RemoteDev;

SftpOptionsPage::SftpOptionsPage(QObject *parent) :
    IConnectionOptionsPage(parent),
    m_widget(nullptr)
{}

SftpOptionsPage::~SftpOptionsPage()
{
    finish();
}

QWidget *SftpOptionsPage::widget()
{
    if (! m_widget) {
        m_widget = new SftpSettingsWidget;
    }

    return m_widget;
}

void SftpOptionsPage::apply(QSettings *settings)
{
//    if (m_widget->ui)
//    settings->setValue(QStringLiteral(KEY_HOSTNAME));
}

void SftpOptionsPage::finish()
{
    delete m_widget;
}

