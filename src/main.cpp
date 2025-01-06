#include "texturefontcreatorgui.h"

#include <QtGui>
#include <QApplication>
#include <iostream>

int main(int argc, char *argv[])
{
    try {
        QApplication a(argc, argv);
        TextureFontCreatorGUI w;
        w.show();
        return a.exec();
    } catch (std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 1;
    }
}
