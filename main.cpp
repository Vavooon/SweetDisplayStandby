#include "sweetdisplaystandby.h"
#include <QApplication>





int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SweetDisplayStandby w;

    QStringList arguments = QCoreApplication::arguments();
    if (!arguments.contains("--hide"))
    {
        w.show();
    }
    return a.exec();
}
