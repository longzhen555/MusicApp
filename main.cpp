#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("音乐播放器");
    QPixmap pix = QPixmap("E:\\QT\\prject\\MUSI\\02.png").scaled(500,300);
    QPalette palette(w.palette());
    palette.setBrush(QPalette::Window,QBrush(pix));
    w.setPalette(palette);

    w.show();

    return a.exec();
}
