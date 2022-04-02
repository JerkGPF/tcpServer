#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>
#include "mytcpserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TcpServer; }
QT_END_NAMESPACE

class TcpServer : public QWidget
{
    Q_OBJECT

public:
    TcpServer(QWidget *parent = nullptr);
    ~TcpServer();
    void loadConfig();

private:
    Ui::TcpServer *ui;

    QString mStrIp;
    quint16 mUsPort;
};
#endif // TCPSERVER_H
