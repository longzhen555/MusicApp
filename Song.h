#ifndef SONG_H
#define SONG_H
#include <QUrl>
#include "until.h"
class Song
{
public:
    Song();
    Song(const QUrl &url) : m_url(url){}
    Song(const QUrl& url,const QString &name,const QString &artist,const QString &album)
        :m_url(url),m_name(name),m_artist(artist),m_album(album)
    {}

    QUrl url() const{ return m_url;}
    QString name() const { return m_name;}
    QString artist() const { return m_artist;}
    QString album() const{return m_album;}
private:
    QUrl m_url;
    QString m_name;
    QString m_artist;
    QString m_album;
};

#endif // SONG_H
