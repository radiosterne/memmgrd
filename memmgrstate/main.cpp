/*
 *@project memmgrstate
 *@file main.cpp
 *@brief Application entry point.
 *@author Andrew Kurnoskin
 *@date 06/01/2013
 */

#include "mainwindow.h"
#include <QApplication>

/*
 *@brief Creates a window and executes application's loop.
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
