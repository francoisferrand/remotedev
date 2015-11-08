#ifndef SFTPOPTIONSPAGE_H
#define SFTPOPTIONSPAGE_H

#include "../iconnectionoptionspage.h"
#include "sftpsettingswidget.h"

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace RemoteDev {

class SftpOptionsPage : public IConnectionOptionsPage
{
    Q_OBJECT
public:
    explicit SftpOptionsPage(QObject *parent = nullptr);
    ~SftpOptionsPage();

    QWidget *widget();

    void apply(QSettings *settings);
    void finish();

private:
    SftpSettingsWidget *m_widget;
};

} // namespace RemoteDev

#endif // SFTPOPTIONSPAGE_H
