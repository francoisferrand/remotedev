#include "optionspage.h"

#include "optionswidget.h"

using namespace RemoteDev::Internal;

OptionsPage::OptionsPage(QObject *parent) :
    IOptionsPage(parent),
    m_widget(nullptr)
{
    setId("remoteoptions");
    setCategory("remoteoptions");
    setDisplayName(tr("General Settings"));
    setDisplayCategory(tr("Remote Development"));

    setCategoryIcon(QString::fromLatin1(""));
}


QWidget *OptionsPage::widget()
{
    if (! m_widget) {
        m_widget = new Internal::OptionsWidget;
    }
    return m_widget;
}


void OptionsPage::apply()
{
    // TODO: implement
}


void OptionsPage::finish()
{
    // TODO: implement: options before dialog is closed
    delete m_widget;
}
