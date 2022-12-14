#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Work.h"

#include <QFileDialog>
#include <QListWidget>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>
#include <QThread>
#include <QHBoxLayout>
#include <QVBoxLayout>
//#define log3 (qDebug() << "[" << __PRETTY_FUNCTION__ << "] " )
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , instance(DataBaseManger::getInstance())
{
    ui->setupUi(this);

    //setWindowOpacity(0.7);
    musicUi();//界面
    m_play = new QMediaPlayer(this);
    //实例化媒体播放列表对象
    m_list = new QMediaPlaylist(this);

    //将媒体播放列表对象设置给媒体播放器，
    m_play->setPlaylist(m_list);

    //默认循环播放
    m_list->setPlaybackMode(QMediaPlaylist::Loop);

    //将音量滑块的最大值设置为100
    ui->volume_sliber->setMaximum(100);
    //将音量滑块的初始位置设置为100
    ui->volume_sliber->setValue(100);
    //连接播放模式  和   对应按钮图标变化的槽函数
    QObject::connect(m_list,&QMediaPlaylist::playbackModeChanged,this,&MainWindow::handlePlaylistPlaybackModeChanged);

    //连接音乐当前播放的位置和滑动块的值
    QObject::connect(m_play,&QMediaPlayer::positionChanged,this,&MainWindow::handSliderValue);

    //歌曲名的显示，通过连接和接收媒体播放列表的当前媒体变化信号，从中读取当前音乐媒体，并显示歌曲名
    QObject::connect(m_list,&QMediaPlaylist::currentMediaChanged,
                     this,&MainWindow::handMediaChanged);

    ui->listWidget_lrc->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//隐藏水平滚动条
    ui->listWidget_lrc->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//隐藏垂直滚动条
    ui->listWidget_lrc->setStyleSheet("background-color: transparent");//背景透明
    ui->listWidget_lrc->setFrameShape(QListWidget::NoFrame);//无边框

    ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//隐藏水平滚动条
    ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//隐藏垂直滚动条
    ui->listWidget->setStyleSheet("background-color: transparent");//背景透明
    ui->listWidget->setFrameShape(QListWidget::NoFrame);//无边框

    QTimer *time = new QTimer(this);
    //连接超时信号函数和自定义槽函数
    QObject::connect(time,&QTimer::timeout,this,&MainWindow::handTimerOut);
    time->start(100);

    //连接播放状态变化信号函数 和  修改播放按钮图标的槽函数
    QObject::connect(m_play, &QMediaPlayer::stateChanged, this, &MainWindow::handlePlayerStateChanged);

    this->setWindowFlag(Qt::WindowStaysOnTopHint); //置顶显示

    initSql();
    queryAllSongsFromSql();
    initLyrics();



}

MainWindow::~MainWindow()
{
    delete ui;

    //遍历容器的每一个value即Song*，进行释放
    for (auto eachValue : m_currentPlayList)
    {
        delete eachValue;
    }

    m_currentPlayList.clear();

    destroySql();
}


void MainWindow::on_add_music_btn_clicked()
{

    QStringList songList = QFileDialog::getOpenFileNames(this,
                                                         "添加多个文件",
                                                         "",
                                                         "mp3(*.mp3)");

    for(auto file : songList)
    {
        log3 << file;

        file = QMediaContent(QUrl(file)).request().url().path();
        log3 << file;

        if(m_currentPlayList.contains(file))
        {
            log3 << "歌曲已存在：" << file;
            continue;
        }
        m_list->addMedia(QMediaContent(QUrl(file)));

        //获取歌曲信息
        const Song& song = getSongInfoFromMp3File(file);


        QListWidgetItem *it = new QListWidgetItem();
        it->setText(song.name() + "-" + song.artist());
        ui->listWidget->addItem(it);

        addSongToSql(song);
    }


}


//获取歌曲信息
const Song &MainWindow::getSongInfoFromMp3File(const QString &filePath)
{
    log3 <<filePath;
    QString name,artist,album;
    //打开文件
    QFile qfile(filePath);

    if(qfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        //读取最后的128个字节
        int length = qfile.size();
        qfile.seek(length - 128);
        char buff[128 + 1] = {0};
        int readSize = qfile.read(buff,128);
        if(readSize > 0)
        {
            log3 <<readSize;



            name = QString::fromLocal8Bit(buff + 3, 30); //获取歌曲名
            artist = QString::fromLocal8Bit(buff + 33, 30); //歌手
            album = QString::fromLocal8Bit(buff + 63, 30); //专辑
            log3 << name << " " << artist << " " << album;

            name.truncate(name.indexOf(QChar::Null));
            artist.truncate(artist.indexOf(QChar::Null));
            album.truncate(album.indexOf(QChar::Null));
            log3 << name << " " << artist << " " << album;
        }
        else
        {
            log3 << "读取失败";
        }
        qfile.close();
    }

    //构造歌曲对象
    Song *song = new Song(QUrl(filePath),name,artist,album);
    this->m_currentPlayList.insert(filePath,song);
    return *song;
}




//获取歌曲和歌词文件
void MainWindow::handMediaChanged(const QMediaContent &content)
{
    ui->listWidget->setCurrentRow(m_list->currentIndex());

    if (content.isNull())
    {
        ui->label_song_name->clear();
        ui->listWidget_lrc->clear();
        log3 << "当前媒体数据为空";
        return;
    }
    QString song_name = content.request().url().fileName();
    log3 << song_name;
    const Song *song = m_currentPlayList.constFind(content.request().url().path()).value();
    QString text = song?(song->name() + "-" + song->artist()) : QFileInfo(song_name).baseName();

    ui->label_song_name->setText(text);

    QString mediaFile = content.request().url().path();
    QString lrcPath = mediaFile.replace(".mp3",".lrc");
    qDebug() <<lrcPath;
    if (QFileInfo(lrcPath).isFile())
    {
        qDebug() << "it is a file";
        readLyricsFromFile(lrcPath); //调用读取歌词文件函数
    }
    else
    {
        m_lyrics.clear();//清理map中的内容
        log3 << "it is not a file111111";
    }
    updataAllLrc();

}



//提取歌词文件中的歌词将其放入map容器中
/*
    updata 2022-11-14
    将歌词文件交给Work类进行处理
*/
void MainWindow::readLyricsFromFile(const QString& lrcFile)
{
    m_lyrics.clear();
    emit readLyrics(lrcFile);

}

void MainWindow::initLyrics()
{
    Work *w = new Work();

    QObject::connect(this,&MainWindow::readLyrics,w,&Work::handReadPlayLyrics);

    //QObject::connect(w,&Work::readLyricsOver,this,&MainWindow::handReadLyricOver);
    QObject::connect(w, SIGNAL(readLyricsOver(QMap<qint64, QString>&)),
                        this, SLOT(handReadLyricOver(QMap<qint64, QString>&)),
                        Qt::DirectConnection
                        );
    //委托线程处理
    QThread *th = new QThread(this);
    w->moveToThread(th);
    th->start();

}


void MainWindow::handReadLyricOver(QMap<qint64, QString>& lyrics)
{
    log3 << "1111111111" << lyrics.size();
    m_lyrics.swap(lyrics);
    updataAllLrc();
}

//将歌词显示到列表控件
void MainWindow::updataAllLrc()
{
    ui->listWidget_lrc->clear();
    if (m_lyrics.isEmpty())
    {

        ui->listWidget_lrc->clear();
        QListWidgetItem *it = new QListWidgetItem("无歌词",ui->listWidget_lrc);
        it->setTextAlignment(Qt::AlignCenter);
        return;
    }

    for (auto text : m_lyrics.values())
    {
        QListWidgetItem *it = new QListWidgetItem(text,ui->listWidget_lrc);
        it->setTextAlignment(Qt::AlignCenter);

    }


}

//超时后歌词自动滚动
void MainWindow::handTimerOut()
{
    if (m_play->state() == QMediaPlayer::State::PlayingState)
    {
       updataLyricsTime();
    }
}


/*
    updata 2022-11-6 歌词滚动
*/
void  MainWindow::updataLyricsTime()
{
    if(m_lyrics.isEmpty())
    {

        return;
    }
    if (ui->listWidget_lrc->currentRow() == -1)
       {
           ui->listWidget_lrc->setCurrentRow(0); //设置第一行为当前行，高亮显示

           return;
       }

       qint64 position = m_play->position(); //播放进度，毫秒级时刻
       int currentRow = ui->listWidget_lrc->currentRow(); //当前行的行数
       log3 << currentRow;
       QList<qint64> lyricsTimeStamp = m_lyrics.keys(); //获取歌词容器的时间戳部分keys: {time1, time2, time3}
       if (position < lyricsTimeStamp[currentRow]) //如果播放器进度小于当前行的起始播放时间，即还没播放到这一行来，就往前找匹配的行
       {
           while (currentRow > 0) //只要前面还有行，就往前找匹配播放时间的歌词行
           {
               --currentRow;

               if (position >= lyricsTimeStamp[currentRow]) { break; }
           }
       }
       else if (position > lyricsTimeStamp[currentRow]) //如果播放器进度大于当前行的起始播放时间，即放到了这一行或以后的行，往后确定匹配行
       {
           //只要后面还有行，就往后确定匹配行
           //找到播放器进度小于某行n起始播放时间，那么确定所匹配的是上一行n-1
           while (currentRow < lyricsTimeStamp.size() - 1)
           {
               //如果播放器进度小于下一行起始播放时间，匹配的就是这一行
               if (position < lyricsTimeStamp[currentRow + 1]) { break; }
               //否则，这一行不匹配，递增往后找
               ++currentRow;
           }
       }
       else {}

       QListWidgetItem * item = ui->listWidget_lrc->item(currentRow); //获取当前行

       ui->listWidget_lrc->setCurrentItem(item); //根据元素设置当前行
       ui->listWidget_lrc->scrollToItem(item, QAbstractItemView::PositionAtCenter); //滚动居中

       //ui->listWidget_lyrics->setCurrentRow(currentRow); //根据行号设置当前行

       //ui->listWidget_lyrics->scrollToItem(item, QAbstractItemView::PositionAtTop); //滚动居顶
       //ui->listWidget_lyrics->scrollToItem(item, QAbstractItemView::PositionAtBottom); //滚动居底
       //ui->listWidget_lyrics->setAutoScroll(false); //设置自动滚动(默认为true)，确保当前元素可见，效果是滚动居底
}

//updata log 2022/11/08  合并播放和暂停按钮
void MainWindow::on_palye_btn_clicked()
{
    if(m_play->state() != QMediaPlayer::PlayingState)
    {
         m_play->play();
    }
    else
    {
         m_play->pause();
    }

}

//设置进度条
void MainWindow::handSliderValue(qint64 position)
{
    //获取歌曲总时长
    qint64 duration = m_play->duration();
    //获取进度条最大值
    int maxNum = ui->song_slider->maximum();
    qDebug() << maxNum;
    //设置滑动条的位置
    ui->song_slider->setValue((double)position/duration*maxNum);

    //显示歌曲播放时间
    //处理补0: (数值, 位数-字符个数, 进制-使用十进制10, 填充字符-使用0)
    QString song_time;
    song_time = QString("%1").arg(position / 60000,2,10,QChar('0'));
    song_time += ":";
    song_time += QString("%1").arg(position % 60000 / 1000,2,10,QChar('0'));
    song_time += "/";
    song_time += QString("%1").arg(duration /60000,2,10,QChar('0'));
    song_time += ":";
    song_time += QString("%1").arg(duration % 60000 /1000,2,10,QChar('0'));
    ui->song_time_label->setText(song_time);

    qDebug() << position;
    qDebug() << song_time;
}

//让进度条根据歌曲的播放自动变化
void MainWindow::on_song_slider_sliderReleased()
{
    //获取当前进度条的位置
    int sliderValue = ui->song_slider->value();
    //获取进度条的最大值
    int maxValue = ui->song_slider->maximum();
    m_play->setPosition((double)sliderValue/maxValue*m_play->duration());
}


void MainWindow::on_pre_btn_clicked()
{
   m_list->previous();
}

void MainWindow::on_next_btn_clicked()
{
   m_list->next();

}

//点击按钮变化播放模式
void MainWindow::on_palyerMode_btn_clicked()
{
  QMediaPlaylist::PlaybackMode mode = m_list->playbackMode();
  int nextMode = (mode+1)%5;
  m_list->setPlaybackMode(QMediaPlaylist::PlaybackMode(nextMode));
}

/*
   updata 2022/11/08
根据播放模式切换按钮上的图标
*/
void MainWindow::handlePlaylistPlaybackModeChanged(QMediaPlaylist::PlaybackMode mode)
{
   static QStringList modeIcon = {":/icons/currentItemOnce.png",
                                  ":/icons/currentItemLoop.png",
                                  ":/icons/sequential.png",
                                  ":/icons/loop.png",
                                  ":/icons/random.png"
                                 };
   ui->palyerMode_btn->setIcon(QIcon(modeIcon[mode]));
}

//切换播放按钮图片
void MainWindow::handlePlayerStateChanged(QMediaPlayer::State state)
{
   if (state == QMediaPlayer::PlayingState)
   {
       ui->palye_btn->setIcon(QIcon(":/icons/play.png"));
   }
   else
   {
       ui->palye_btn->setIcon(QIcon(":/icons/pause.png"));
   }
}



//获取音量滑块的移动时值
void MainWindow::on_volume_sliber_sliderMoved(int position)
{
   qDebug() << position;
   m_play->setVolume(position);
   log3 << m_play->volume();
}


/*
   双击播放歌曲
*/
void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
   m_list->setCurrentIndex(ui->listWidget->currentRow());
   m_play->play();

}


/*
    清除歌曲列表
*/
void MainWindow::on_clear_btn_clicked()
{
    ui->listWidget->clear();
    ui->label_song_name->clear();
    m_list->clear();
    for(auto eachValue : m_currentPlayList)
    {
        delete eachValue;
    }
    m_currentPlayList.clear();
    clearSongsToSql();
}



//初始化sql
void MainWindow::initSql()
{
    if (!instance.init())
    {
        QMessageBox::critical(this, "持久化", "数据库不可用");
    }
}

//将歌曲添加到数据库中
void MainWindow::addSongToSql(const Song& song)
{
    if(!instance.addSong(song))
    {
         QMessageBox::critical(this, "持久化", "添加歌曲失败");
    }
}

//查询所有歌曲
void MainWindow::queryAllSongsFromSql()
{
    QVector<Song*> songs;
    bool ret = instance.querySong(songs);

    log3 << "查询到歌曲数量：" << songs.size();
    if(ret && songs.size()>0)
    {
        for(auto song : songs)
        {
            //将数据库中的已有的歌曲添加到，歌曲列表中
            m_currentPlayList.insert(song->url().path(),song);
             m_list->addMedia(QMediaContent(song->url().path()));
            //显示歌曲信息到歌曲列表
            QListWidgetItem *it = new QListWidgetItem();
            it->setText(song->name() + "-" + song->artist());
            ui->listWidget->addItem(it);
        }

    }
}

//清除数据库中的歌曲
void MainWindow::clearSongsToSql()
{
    if(!instance.clearSong())
    {
        QMessageBox::critical(this, "持久化", "清空歌曲列表失败");
    }
}

//释放数据库
void MainWindow::destroySql()
{
    instance.destory();
}


void MainWindow::musicUi()
{
    QPalette p;
    p.setColor(QPalette::Text,Qt::white);//设置字体颜色为白色
    p.setColor(QPalette::WindowText,Qt::white);
    ui->listWidget_lrc->setPalette(p);
    ui->listWidget->setPalette(p);
    ui->song_time_label->setPalette(p);
    ui->label_song_name->setPalette(p);
    QVBoxLayout *v1 = new QVBoxLayout();
    v1->addWidget(ui->volume_sliber);
    v1->addWidget(ui->label);


    QHBoxLayout *h1 = new QHBoxLayout();
    h1->addWidget(ui->listWidget,3);
    h1->addWidget(ui->listWidget_lrc,6);
    h1->addLayout(v1,1);

    QHBoxLayout *h2 = new QHBoxLayout();
    h2->addWidget(ui->clear_btn);
    h2->addWidget(ui->palyerMode_btn);
    h2->addWidget(ui->add_music_btn);
    h2->addWidget(ui->pre_btn);
    h2->addWidget(ui->palye_btn);
    h2->addWidget(ui->next_btn);

    QHBoxLayout *h3 = new QHBoxLayout();
    h3->addWidget(ui->label_song_name);

    QHBoxLayout *h4 = new QHBoxLayout();
    h4->addWidget(ui->song_slider);
    h4->addWidget(ui->song_time_label);

    QVBoxLayout *v2 = new QVBoxLayout();
    v2->addLayout(h1);
    v2->addLayout(h2);
    v2->addLayout(h3);
    v2->addLayout(h4);

    //this->setLayout(v2);
    ui->centralwidget->setLayout(v2);
}









