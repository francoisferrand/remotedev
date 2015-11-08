#ifndef REMOTEOPTIONSWIDGET_H
#define REMOTEOPTIONSWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

namespace RemoteDev {
namespace Internal {

namespace Ui { class ConnectionsWidget; }

class ConnectionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectionsWidget(QWidget *parent = 0);
    ~ConnectionsWidget();

    void updateWithSettings(QSettings &settings);

private:

private:
    Ui::ConnectionsWidget *ui;
};

} // namespace Interal
} // namespace RemoteDev

#endif // REMOTEOPTIONSWIDGET_H
