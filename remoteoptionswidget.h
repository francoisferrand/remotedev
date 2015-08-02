#ifndef REMOTEOPTIONSWIDGET_H
#define REMOTEOPTIONSWIDGET_H

#include <QWidget>

namespace RemoteDev {
namespace Internal {

namespace Ui { class RemoteOptionsWidget; }

class RemoteOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RemoteOptionsWidget(QWidget *parent = 0);
    ~RemoteOptionsWidget();

private:
    Ui::RemoteOptionsWidget *ui;
};

} // namespace Interal
} // namespace RemoteDev

#endif // REMOTEOPTIONSWIDGET_H
