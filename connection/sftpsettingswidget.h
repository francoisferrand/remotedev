#ifndef SFTPSETTINGSWIDGET_H
#define SFTPSETTINGSWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

namespace RemoteDev {

namespace Ui {
class SftpSettingsWidget;
}

class SftpSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SftpSettingsWidget(QWidget *parent = 0);
    ~SftpSettingsWidget();

    void loadSettings(const QSettings &settings);

    QString hostname() const;
    quint16 port() const;
    int     tmout() const;
    QString user() const;

private:
    Ui::SftpSettingsWidget *ui;
};

} // namespace RemoteDev

#endif // SFTPSETTINGSWIDGET_H
