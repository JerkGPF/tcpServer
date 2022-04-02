#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"
#include "opendb.h"
#include <QDir>



class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();
    QString getName();
signals:
    void offLine(MyTcpSocket *mySocket);

public slots:
    void recvMsg();
    void clientOffLine();//处理客户端下线
private:
    QString mStrName;
};

#endif // MYTCPSOCKET_H
