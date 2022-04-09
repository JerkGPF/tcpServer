#include "mytcpsocket.h"
#include <QDebug>
#include "mytcpserver.h"
#include <QDir>
#include <QFileInfoList>
MyTcpSocket::MyTcpSocket()
{
    connect(this,&QTcpSocket::readyRead,[=]{
        recvMsg();
    });
    connect(this,&QTcpSocket::disconnected,[=]{
        clientOffLine();
    });
}

QString MyTcpSocket::getName()
{
    return mStrName;
}

void MyTcpSocket::recvMsg()
{
    qDebug()<<this->bytesAvailable();
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen,sizeof (uint));
    uint uiMsgLen = uiPDULen - sizeof (PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    this->read((char*)pdu+sizeof (uint),uiPDULen-sizeof (uint));
    switch (pdu->uiMsgType)
    {
    case ENUM_MSG_TYPE_REGIST_REQUEST:
    {
        char caName[32] ={'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName,pdu->caData,32);
        strncpy(caPwd,pdu->caData+32,32);
        //判断是否成功写入数据库，并发送返回数据
        bool ret = OpenDB::getInstance().handleRegist(caName,caPwd);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
        if(ret)
        {
            strcpy(respdu->caData,REGIST_OK);
            QDir dir;
            dir.mkdir(QString("./%1").arg(caName));
        }
        else
        {
            strcpy(respdu->caData,REGIST_FAILED);
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_REQUEST:
    {
        char caName[32] ={'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName,pdu->caData,32);
        strncpy(caPwd,pdu->caData+32,32);
        bool ret = OpenDB::getInstance().handleLogin(caName,caPwd);

        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
        if(ret)
        {
            strcpy(respdu->caData,LOGIN_OK);
            mStrName = caName;
        }
        else
        {
            strcpy(respdu->caData,LOGIN_FAILED);
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
    {
        QStringList ret = OpenDB::getInstance().handleAllOnline();
        uint uiMsgLen = ret.size()*32;
        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
        for(int i = 0;i<ret.size();i++)
        {
            memcpy((char*)(respdu->caMsg)+i*32,
                   ret[i].toStdString().c_str(),ret[i].size());
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:
    {
        int ret = OpenDB::getInstance().handleSearchUsr(pdu->caData);
        PDU *respdu =  mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
        if(ret==-1)
            strcpy(respdu->caData,SEARCH_USR_NO);
        else if (ret==1)
            strcpy(respdu->caData,SEARCH_USR_ONLINE);
        else if(ret==0)
            strcpy(respdu->caData,SEARCH_USR_OFFLINE);
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
    {
        char caPerName[32] ={'\0'};
        char caName[32] = {'\0'};
        strncpy(caPerName,pdu->caData,32);
        strncpy(caName,pdu->caData+32,32);
        int ret = OpenDB::getInstance().handleAddFriend(caPerName,caName);
        PDU *respdu = NULL;
        if(ret==-1)
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData,UNKNOW_ERROR);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if (ret==0)
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData,EXISTED_FRIEND);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if (ret==1)
        {
            MyTcpServer::getInstance().reSend(caPerName,pdu);
        }
        else if (ret==2)
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData,ADD_FRIEND_OFFINE);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if (ret==3)
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData,ADD_FRIEND_NO_EXIST);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
    {
        char caPerName[32] = {'\0'};
        char caName[32] = {'\0'};
        strncpy(caPerName, pdu->caData, 32);
        strncpy(caName, pdu->caData+32, 32);
        OpenDB::getInstance().handleAgreeAddFriend(caPerName, caName);
        MyTcpServer::getInstance().reSend(caName,pdu);
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
    {
        char caName[32] = {'\0'};
        strncpy(caName, pdu->caData+32, 32);
        MyTcpServer::getInstance().reSend(caName, pdu);
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
    {
        char caName[32] = {'\0'};
        strncpy(caName,pdu->caData,32);
        QStringList ret = OpenDB::getInstance().handleFlushFriend(caName);
        uint uiMsgLen = ret.size()*32;
        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
        for(int i = 0;i<ret.size();i++)
            memcpy((char*)(respdu->caMsg)+i*32,ret[i].toStdString().c_str(),ret[i].size());
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;

        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
    {

        char caSelfName[32] = {'\0'};
        char caFriendName[32] = {'\0'};
        strncpy(caSelfName, pdu->caData, 32);
        strncpy(caFriendName, pdu->caData+32, 32);
        OpenDB::getInstance().handleDelFriend(caSelfName, caFriendName);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
        strcpy(respdu->caData, DEL_FRIEND_OK);
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;

        MyTcpServer::getInstance().reSend(caFriendName, pdu);

        break;
    }
    {
        char caPerName[32] = {'\0'};
        memcpy(caPerName, pdu->caData+32, 32);
        char caName[32] = {'\0'};
        memcpy(caName, pdu->caData, 32);
        qDebug() << caName << "-->" << caPerName << (char*)(pdu->caMsg);
        MyTcpServer::getInstance().reSend(caPerName, pdu);
        break;
    }
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
    {
        char caPerName[32] = {'\0'};
        memcpy(caPerName, pdu->caData+32, 32);
        char caName[32] = {'\0'};
        memcpy(caName, pdu->caData, 32);
        qDebug() << caName << "-->" << caPerName << (char*)(pdu->caMsg);
        MyTcpServer::getInstance().reSend(caPerName, pdu);
        break;
    }
    case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
    {
        char caName[32] = {'\0'};
        strncpy(caName,pdu->caData,32);
        QStringList onlineFriend = OpenDB::getInstance().handleFlushFriend(caName);
        QString tmp;
        for(int i = 0;i<onlineFriend.size();i++)
        {
            tmp = onlineFriend.at(i);
            MyTcpServer::getInstance().reSend(tmp.toStdString().c_str(),pdu);
        }
        break;
    }
    case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
    {
        QDir dir;
        QString strCurPath = QString("%1").arg((char*)(pdu->caMsg));
        bool ret = dir.exists(strCurPath);
        PDU *respdu = NULL;
        if(ret)  //当前目录存在
        {
            char caNewDir[32] = {'\0'};
            memcpy(caNewDir,pdu->caData+32,32);
            QString strNewPath = strCurPath+"/"+caNewDir;
            ret = dir.exists(strNewPath);
            if(ret)  //创建的文件名已存在
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                strcpy(respdu->caData,FILE_NAME_EXIST);
            }
            else //创建的文件名不存在
            {
                dir.mkdir(strNewPath);
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                strcpy(respdu->caData,CREAT_DIR_OK);

            }
        }
        else    //当前目录不存在
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
            strcpy(respdu->caData,DIR_NO_EXIST);
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
    {
        char *pCurPath = new char[pdu->uiMsgLen];
        memcpy(pCurPath,pdu->caMsg,pdu->uiMsgLen);
        QDir dir(pCurPath);
        QFileInfoList fileInfoList = dir.entryInfoList();
        int iFileCount = fileInfoList.size();
        PDU* respdu = mkPDU(sizeof (FileInfo)*iFileCount);
        respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
        FileInfo *pFileInfo = NULL;
        QString strFileName;
        for(int i = 0;i<iFileCount;i++)
        {
            pFileInfo = (FileInfo*)(respdu->caMsg)+i;
            strFileName = fileInfoList[i].fileName();
            memcpy(pFileInfo->caFileName,strFileName.toStdString().c_str(),strFileName.size());
            if(fileInfoList[i].isDir())
                pFileInfo->iFileType = 0;
            else if (fileInfoList[i].isFile()) {
                pFileInfo->iFileType = 1;
            }
        }
//        foreach(QFileInfo s,fileInfoList){
//            qDebug()<<s.fileName()<<","<<s.size()
//                    <<"文件夹:"<<s.isDir()
//                    <<"文件:"<<s.isFile();
//        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_DEL_DIR_REQUEST:
    {
        char caName[32] = {'\0'};
        strcpy(caName,pdu->caData);
        char* pPath = new char[pdu->uiMsgLen];
        memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
        QString strPath = QString("%1/%2").arg(pPath).arg(caName);
        qDebug()<<strPath;

        QFileInfo fileInfo(strPath);
        bool ret = false;
        if(fileInfo.isDir())//如果是目录则删除
        {
            QDir dir;
            dir.setPath(strPath);
            ret = dir.removeRecursively();
        }
        else if(fileInfo.isFile()) // 常规文件不删除
            ret = false;
        PDU *respdu = NULL;  //回复pdu
        if(ret)
        {
            respdu = mkPDU(strlen(DEL_DIR_OK)+1);
            respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
            memcpy(respdu->caData,DEL_DIR_OK,strlen(DEL_DIR_OK));
        }
        else
        {
            respdu = mkPDU(strlen(DEL_DIR_FAILURED)+1);
            respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
            memcpy(respdu->caData,DEL_DIR_FAILURED,strlen(DEL_DIR_FAILURED));
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    default:
        break;
    }
    free(pdu);
    pdu = NULL;

}

void MyTcpSocket::clientOffLine()
{

    OpenDB::getInstance().handleOffLine(mStrName.toStdString().c_str());
    emit offLine(this);

}
