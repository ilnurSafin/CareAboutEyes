#include "dialog.h"
#include <QApplication>
#undef main

int main(int argc, char *argv[])
try
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
    Dialog w;
    w.hide();

    return a.exec();
}catch(std::exception& e)
{
    std::cerr << e.what() << std::endl;
}
