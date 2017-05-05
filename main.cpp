#include "mainwindow.h"
#include <QApplication>
#include <time.h>
#include <QtSerialPort/qserialport.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
        srand((unsigned)time(NULL));
		//MainWindow w;
		//w.ininServer();
		//w.show();
		//return a.exec();
        int Exitcode=0;
    do
        {
            MainWindow w;
            w.ininServer();
            w.show();
            Exitcode=a.exec();
        }while(Exitcode==25);
    return 0;
}
