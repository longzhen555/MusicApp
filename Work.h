#ifndef WORK_H
#define WORK_H

#include "until.h"

#include <QObject>
#include <QFileInfo>

class Work : public QObject
{
    Q_OBJECT
public:
    Work(QObject* parent = nullptr);

public slots:
    void handReadPlayLyrics(const QString& file);

signals:
    void readLyricsOver(QMap<qint64, QString>& lyrics);
};

#endif // WORK_H
