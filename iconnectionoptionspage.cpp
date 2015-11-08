#include "iconnectionoptionspage.h"

#include <coreplugin/dialogs/ioptionspage.h>

using namespace RemoteDev;

/**
 * @brief The OptionsPageWrapper class is wrapper of IConnectionOptionsPage
 * to IOptionsPage and is used only for providing code of the
 * Core::IOptionsPage::matches() to the IConnectionOptionsPage class
 */
class OptionsPageWrapper : public Core::IOptionsPage {
public:
    explicit OptionsPageWrapper(IConnectionOptionsPage *page) :
        m_connOptions(page)
    {}

    QWidget *widget() { return m_connOptions->widget(); }

    void apply() {}
    void finish() {}
private:
    IConnectionOptionsPage *m_connOptions;
};

IConnectionOptionsPage::IConnectionOptionsPage(QObject *parent) :
    QObject(parent),
    m_optionsPage(nullptr)
{}

IConnectionOptionsPage::~IConnectionOptionsPage()
{
    delete m_optionsPage;
}

bool IConnectionOptionsPage::matches(const QString &searchKeyWord) const
{
    // avoid creating IOptionsPage object before this method gets called directly
    // (as it may be overrriden)
    if (! m_optionsPage) {
        // the original IOptionsPage::matches also uses const_cast
        m_optionsPage = new OptionsPageWrapper(const_cast<IConnectionOptionsPage *>(this));
    }

    return m_optionsPage->matches(searchKeyWord);
}

