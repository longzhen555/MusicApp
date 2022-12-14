#include "Work.h"

Work::Work(QObject* parent):QObject(parent)
{

}

void Work::handReadPlayLyrics(const QString &lrcFile)
{
    QMap<qint64, QString> lyrics;
    bool ret = QFileInfo(lrcFile).isFile();
    if (!ret)
    {
       log3 << "it is not a file: " << lrcFile;
       return;
    }

    QFile qfile(lrcFile);
    ret = qfile.open(QIODevice::ReadOnly | QIODevice::Text);//只读和文本模式打开
    if(!ret)
    {
        //log3 << "歌词文件打开失败！" << lrcFile;
        return;
    }
    QTextStream ts(&qfile);
    QString line;
    QStringList lineContents;
    QStringList timeContents;

    while (!ts.atEnd())
    {
        line = ts.readLine();
        qDebug() << line;

        if(line.isEmpty())
        {
            continue;
        }

        lineContents = line.split(']');

        timeContents = lineContents[0].split(':');

        int minutes = timeContents[0].mid(1).toInt();
        int secods = timeContents[1].toDouble();
        lyrics.insert((minutes*60+secods)*1000,lineContents[1]);

        lineContents.clear();
        timeContents.clear();
    }

    emit readLyricsOver(lyrics);
}
