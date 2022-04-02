#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"


class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    MyTcpServer();

    //单例
    static MyTcpServer &getInstance();
    //重写该虚函数。
    void incomingConnection(qintptr handle) override;

    void reSend(const char* perName,PDU* pdu);

public slots:
    void deleteSocket(MyTcpSocket *mySocket);
private:
    QList<MyTcpSocket*> mTcpSocketList;
};

#endif // MYTCPSERVER_H
