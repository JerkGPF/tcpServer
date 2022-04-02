#include "mytcpserver.h"

MyTcpServer::MyTcpServer()
{

}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}
//当有客户端连接时，自动调用该函数
void MyTcpServer::incomingConnection(qintptr handle)
{
    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(handle);
    mTcpSocketList.append(pTcpSocket);
    connect(pTcpSocket,SIGNAL(offLine(MyTcpSocket*)),
            this,SLOT(deleteSocket(MyTcpSocket*)));
    //    connect(pTcpSocket,&MyTcpSocket::offLine,[=](MyTcpSocket *mySocket){
    //        deleteSocket(mySocket);
    //    });

}

void MyTcpServer::reSend(const char *perName, PDU *pdu)
{
    if(perName==NULL)
        return;
    QString strN = perName;
    for(int i = 0;i<mTcpSocketList.size();i++)
        if(strN == mTcpSocketList[i]->getName())
        {
            mTcpSocketList[i]->write((char*)pdu,pdu->uiPDULen);
            break;
        }
}

void MyTcpServer::deleteSocket(MyTcpSocket *mySocket)
{
    QList<MyTcpSocket*>::iterator iter= mTcpSocketList.begin();
    for(;iter!=mTcpSocketList.end();iter++)
    {
        if(mySocket == *iter)
        {
            //            delete *iter;
            //            *iter = NULL;
            mTcpSocketList.erase(iter);
            break;
        }
    }
    for(int i = 0;i<mTcpSocketList.size();i++)
        qDebug()<<mTcpSocketList[i]->getName();
}
