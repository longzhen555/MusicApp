#ifndef UNTIL_H
#define UNTIL_H

#include <QFileInfo>
#include <QDebug>
//格式化函数，将系统文件、行号、函数等宏格式化为指定格式，最终居左显示，宽度50，填充空格
__attribute__((unused)) static QString ___forlmat(QString file, int line, QString func)
{
   file = QFileInfo(file).fileName();
   func = func.mid(func.indexOf(' ') + 1, (func.indexOf('(') - 1 - func.indexOf(' ')));
   return ("[" + file + ":" + QString::number(line) + " " + func).leftJustified(50, ' ') + "]";
}

#define log3 (qDebug() << ___forlmat(__FILE__, __LINE__, __PRETTY_FUNCTION__))

#endif // UNTIL_H
