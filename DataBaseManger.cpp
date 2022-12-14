#include "DataBaseManger.h"

DataBaseManger::DataBaseManger():m_sqlTpye("QSQLITE"),m_dbName("play.db")
{
    m_sqlDataBase = QSqlDatabase::addDatabase(m_sqlTpye);
    if(m_sqlDataBase.isValid())
    {
        log3 << "已添加类型为QSQLITE的数据库连接: " << m_sqlDataBase.databaseName();
    }
    else
    {
        log3 << "添加失败";
    }

    m_sqlDataBase.setDatabaseName(m_dbName);
}

DataBaseManger::~DataBaseManger()
{
    destory();
}

bool DataBaseManger::initSong()
{
    bool ret = false;
    ret = m_sqlDataBase.isOpen();
    if(!ret)
    {
        log3 << "数据库未打开，无法创建歌曲表";
        return ret;
    }
    QString sql = "create table if not exists songs(";
            sql += "id integer primary key autoincrement,";
            sql += "url text unique not null,";
            sql += "name text,";
            sql += "artist text,";
            sql += "album text);";
    QSqlQuery query;
    ret = query.exec(sql);
    if(!ret)
    {
        log3 << "创建歌曲表失败: " << query.lastError().text();
        return ret;
    }
    log3 << "创建歌曲表成功";
    return ret;
}

bool DataBaseManger::init()
{
    bool ret = m_sqlDataBase.open();
    if(!ret)
    {
        log3 << "数据库打开失败" << m_sqlDataBase.lastError().text();
        return ret;
    }
    else
    {
        log3 << "数据库打开成功";
    }
    return initSong();
}

void DataBaseManger::destory()
{
    if(m_sqlDataBase.isOpen()){m_sqlDataBase.close();}
    if(m_sqlDataBase.isValid()){m_sqlDataBase.removeDatabase(m_dbName);}
}

bool DataBaseManger::addSong(const Song &song)
{
    bool ret = false;
    ret = m_sqlDataBase.isOpen();
    if(!ret)
    {
        log3 << "数据库未打开，无法创建歌曲表";
        return ret;
    }

    QString sql = "insert into songs(url,name,artist,album) values(";
            sql += "'" + song.url().toString() + "',";
            sql +=  "'" + song.name() + "',";
            sql +=  "'" + song.artist() + "',";
            sql +=  "'" + song.album() + "');";
    QSqlQuery qurey;
    ret = qurey.exec(sql);
    if(!ret)
    {
        log3 << "添加歌曲失败：" << m_sqlDataBase.lastError().text();
        return ret;
    }
    log3 << "添加歌曲成功：";
    return ret;
}

bool DataBaseManger::querySong(QVector<Song *> &reSong)
{
    bool ret = false;
    ret = m_sqlDataBase.isOpen();
    if(!ret)
    {
        log3 << "数据库未打开，无法创建歌曲表";
        return ret;
    }
    QString sql = "select * from songs;";
    QSqlQuery query;
    ret = query.exec(sql);

    if(!ret)
    {
        log3 << "查询失败：" << m_sqlDataBase.lastError().text();
        return ret;
    }
    QUrl url;
    QString name,artist,album;
    while (query.next())
    {
        url = QUrl(query.value("url").toString());
        name = query.value("name").toString();
        artist = query.value("artist").toString();
        album = query.value("album").toString();
        Song *song = new Song(url,name,artist,album);
        reSong.push_back(song);
    }
    return ret;
}

bool DataBaseManger::clearSong()
{
    bool ret = false;
    ret = m_sqlDataBase.isOpen();
    if(!ret)
    {
        log3 << "数据库未打开，无法创建歌曲表";
        return ret;
    }
    QString sql = "delete from songs";

    QSqlQuery query;
    ret = query.exec(sql);
    if(!ret)
    {
        log3 << "数据库删除失败";
        return ret;
    }
     log3 << "数据库删除成功";
    return ret;
}

