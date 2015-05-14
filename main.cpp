#include "sweetdisplaystandby.h"
#include <QApplication>





int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SweetDisplayStandby w;
    w.show();
    return a.exec();
}
