#include "texturefontcreatorgui.h"

#include <QtGui>
#include <QApplication>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TextureFontCreatorGUI w;
    w.show();
    return a.exec();
}
