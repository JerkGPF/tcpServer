#include "tcpserver.h"
#include "ui_tcpserver.h"

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QHostAddress>
#include <QMessageBox>

TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);
    loadConfig();
    MyTcpServer::getInstance().listen(QHostAddress(mStrIp),mUsPort);


}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    QFile file(":/server.config");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray baData = file.readAll();
        QString strData = baData.toStdString().c_str();//转换为字符串
        file.close();//读取完毕，关闭流
        strData.replace("\r\n"," ");//替换文件中的字符
        QStringList strList = strData.split(" ");//按照空格切分；
        //        foreach (auto info, strList) {//遍历输出
        //            qDebug()<<"==>>"<<info;
        //        }
        mStrIp = strList[0];
        mUsPort = strList[1].toUShort();
//        qDebug()<<mStrIp;
//        qDebug()<<mUsPort;

    }
    else
    {
        QMessageBox::critical(this,"配置文件","配置文件读取失败");

    }
}


