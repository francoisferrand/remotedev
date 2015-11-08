#ifndef ICONNECTIONOPTIONSPAGE_H
#define ICONNECTIONOPTIONSPAGE_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QWidget;
class QSettings;
QT_END_NAMESPACE

namespace Core {
    class IOptionsPage;
}

namespace RemoteDev {

/**
 * @brief The IConnectionOptionsPage class is similar to Core::IOptionsPage
 * except it provides interface for pages, that represent settings for the
 * remote Connection (like SSH).
 * It is not directly inherited from Core::IOptionsPage because instances of
 * this class should not be confused with real IOptionsPage objects and, consequently,
 * not put into "Options" dialog directly
 */
class IConnectionOptionsPage : public QObject
{
    Q_OBJECT
public:
    explicit IConnectionOptionsPage(QObject *parent = 0);
    virtual ~IConnectionOptionsPage();

    /**
     * @brief matches - search for keyword within this page
     * @param Is used by the Options dialog search filter to match searchKeyWord to this options page.
     * This defaults to take the widget and then looks for all child labels, check boxes, push buttons, and group boxes.
     * Should return true when a match is found.
     * @return true if matches, false otherwise
     */
    virtual bool matches(const QString &searchKeyWord) const;

    /**
     * @brief widget - Returns the widget to show in the Options dialog.
     * You should create a widget lazily here, and delete it again in the finish() method.
     * This method can be called multiple times, so you should only create a new widget if the old one was deleted.
     * @return Widget, representing this page
     */
    virtual QWidget *widget() = 0;

    /**
     * @brief apply - store connection's settings
     * This is called when selecting the Apply button on the options page dialog.
     * It should detect whether any changes were made and store those.
     * @param settings  Object, where settings should be stored, the proper group
     *                  is already set (by calling the beginGroup() method)
     */
    virtual void apply(QSettings *settings) = 0;

    /**
     * @brief finish - clean up page's resouces and widgets
     * Is called directly before the Options dialog closes.
     * Here you should delete the widget that was created in widget() to free resources.
     */
    virtual void finish() = 0;

private:
    // lay initialisation
    mutable Core::IOptionsPage *m_optionsPage;
};

} // namespace RemoteDev

#endif // ICONNECTIONOPTIONSPAGE_H
