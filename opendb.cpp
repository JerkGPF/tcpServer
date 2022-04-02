#include "opendb.h"

#include <QMessageBox>
#include <QString>
#include <QDebug>
OpenDB::OpenDB(QObject *parent) : QObject(parent)
{
    mDB = QSqlDatabase::addDatabase("QSQLITE");

}

OpenDB &OpenDB::getInstance()
{
    static OpenDB instance;
    return  instance;
}

void OpenDB::init()
{
    mDB.setHostName("localhost");
    mDB.setDatabaseName("D:\\Projects\\QtProjects\\tcpServer\\cloud.db");
    if(mDB.open())
    {
        QSqlQuery query;
        query.exec("select * from usrInfo");
        while (query.next())
        {
            QString data = QString("%1,%2,%3").arg(query.value(0).toString())
                    .arg(query.value(1).toString()).arg(query.value(2).toString());
            qDebug()<<data;

        }
    }
    else
    {
        QMessageBox::critical(NULL,"打开数据库","数据库打开失败");

    }
}

OpenDB::~OpenDB()
{
    mDB.close();
}

bool OpenDB::handleRegist(const char *name, const char *pwd)
{
    if(name==NULL || pwd==NULL)
        return false;
    QString data = QString("insert into usrInfo(name, pwd) values(\'%1\',\'%2\')").arg(name).arg(pwd);
    QSqlQuery query;
    return query.exec(data);
}

bool OpenDB::handleLogin(const char *name, const char *pwd)
{
    if(name==NULL || pwd==NULL)
        return false;
    QString data = QString("select * from usrInfo where name = \'%1\' and pwd = \'%2\' and online = 0").arg(name).arg(pwd);
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        data = QString("update usrInfo set online = 1 where name = \'%1\' and pwd = \'%2\'").arg(name).arg(pwd);
        QSqlQuery query;
        query.exec(data);
        return true;
    }
    else
        return false;
}

void OpenDB::handleOffLine(const char *name)
{
    if(name==NULL)
        return;
    QString data = QString("update usrInfo set online = 0 where name = \'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
}

QStringList OpenDB::handleAllOnline()
{
    QString data = QString("select * from usrInfo where online=1");
    QSqlQuery query;
    query.exec(data);
    QStringList result;
    result.clear();
    while (query.next())
    {
        result.append(query.value(1).toString());
        qDebug() << query.value(1).toString();
    }
    return result;
}

int OpenDB::handleSearchUsr(const char *name)
{
    if(name==NULL)
        return -1;
    QString data = QString("select online from usrInfo where name = \'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        //在线值待定
        int ret = query.value(0).toInt();
        if(ret==1)//在线返回1
            return 1;
        else if (ret==0)//不在线返回0
            return 0;
    }
    else
        return -1;
}

int OpenDB::handleAddFriend(const char *perName, const char *name)
{
    if(perName == NULL || name == NULL)
        return -1;
    QString data = QString("select * from friend where (id=(select id from usrInfo where name=\'%1\') and friendId = (select id from usrInfo where name=\'%2\')) "
                           "or (id=(select id from usrInfo where name=\'%3\') and friendId = (select id from usrInfo where name=\'%4\'))").arg(perName).arg(name).arg(name).arg(perName);
    qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    if(query.next())
        return 0;//双方已是好友
    else
    {
        data = QString("select online from usrInfo where name = \'%1\'").arg(perName);
        QSqlQuery query;
        query.exec(data);
        if(query.next())
        {
            //在线值待定
            int ret = query.value(0).toInt();
            if(ret==1)//在线
                return 1;
            else if (ret==0)//不在线
                return 2;
        }
        else
            return 3;//不存在
    }
}

void OpenDB::handleAgreeAddFriend(const char *pername, const char *name)
{
    if (NULL == pername || NULL == name)
    {
        return;
    }
    QString data = QString("insert into friend(id, friendId) values((select id from usrInfo where name=\'%1\'), (select id from usrInfo where name=\'%2\'))").arg(pername).arg(name);
    QSqlQuery query;
    query.exec(data);
}

QStringList OpenDB::handleFlushFriend(const char *name)
{
    QStringList strFriendList;
    strFriendList.clear();
    if(name == NULL)
    {
        return strFriendList;
    }
    QString data = QString("select name from usrInfo where online=1 and id in (select id from friend where friendId=(select id from usrInfo where name=\'%1\'))").arg(name);
    QSqlQuery query;
    query.exec(data);
    while (query.next())
    {
        strFriendList.append(query.value(0).toString());
        qDebug()<<"==============="<<query.value(0).toString();
    }
    data = QString("select name from usrInfo where online=1 and id in (select friendId from friend where id=(select id from usrInfo where name=\'%1\'))").arg(name);
    query.exec(data);
    while (query.next())
    {
        strFriendList.append(query.value(0).toString());
        qDebug()<<"==============="<<query.value(0).toString();

    }
    return strFriendList;

}

bool OpenDB::handleDelFriend(const char *name, const char *friendName)
{
    if(name ==NULL || friendName==NULL)
        return false;
    QString data = QString("delete from friend where id=(select id from usrInfo where name=\'%1\') and friendId=(select id from usrInfo where name=\'%2\')").arg(name).arg(friendName);
    QSqlQuery query;
    query.exec(data);

    data = QString("delete from friend where id=(select id from usrInfo where name=\'%1\') and friendId=(select id from usrInfo where name=\'%2\')").arg(friendName).arg(name);
    query.exec(data);

    return true;
}
