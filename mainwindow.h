#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Song.h"
#include "until.h"
#include "DataBaseManger.h"

#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QListWidget>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void readLyricsFromFile(const QString&);

    void updataAllLrc();

    void updataLyricsTime();

    const Song& getSongInfoFromMp3File(const QString& filePath);

    //定义数据库访问的相关方法
    void initSql();
    void addSongToSql(const Song& song);
    void queryAllSongsFromSql();
    void clearSongsToSql();
    void destroySql();

    void initLyrics();

    void musicUi();
private slots:
    void on_add_music_btn_clicked();

    void on_palye_btn_clicked();

    void handSliderValue(qint64);

    void on_song_slider_sliderReleased();

    void handMediaChanged(const QMediaContent& content);
    void on_pre_btn_clicked();

    void on_next_btn_clicked();

    void on_palyerMode_btn_clicked();

    void on_volume_sliber_sliderMoved(int position);

    //void on_add_some_song_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void handTimerOut();

    void handlePlaylistPlaybackModeChanged(QMediaPlaylist::PlaybackMode mode);

    void handlePlayerStateChanged(QMediaPlayer::State state);


    void on_clear_btn_clicked();

    void handReadLyricOver(QMap<qint64,QString>& lyrics);

signals:
    void readLyrics(const QString& file);

private:
    Ui::MainWindow *ui;
    QMediaPlayer *m_play;//媒体播放器
    QMediaPlaylist *m_list;//后台播放列表

    QMap<qint64,QString> m_lyrics;//歌词

    QMap<QString,Song*> m_currentPlayList;//歌曲列表

    DataBaseManger& instance;//数据库管理类

};
#endif // MAINWINDOW_H
