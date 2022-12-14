#ifndef DATABASEMANGER_H
#define DATABASEMANGER_H
#include "until.h"
#include "Song.h"

#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
class DataBaseManger
{
private:
    DataBaseManger();
    DataBaseManger(const DataBaseManger&);
    ~DataBaseManger();

    QString m_sqlTpye;
    QString m_dbName;
    QSqlDatabase m_sqlDataBase;

    bool initSong();//初始化歌曲列表
public:
    static DataBaseManger& getInstance()
    {
        static DataBaseManger instance;
        return instance;
    }

    bool init();//初始化
    void destory();//释放
    bool addSong(const Song& song);//添加歌曲
    bool querySong(QVector<Song*>& reSong);//查询歌曲
    bool clearSong();

};

#endif // DATABASEMANGER_H
