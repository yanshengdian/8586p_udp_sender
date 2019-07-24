#include <QtGui/QApplication>
#include "sender.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    sender w;
    w.show();

    return a.exec();
}
