#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
#include <stdint.h>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QTimer>
namespace Ui {
class MainWindow;

}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    uint8_t globalString[2048];

    int len=0;
    int flag=0;
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void ininServer();
    void sendReady();
    void BK(uint32_t *key, uint32_t *message);
   int send(uint8_t *Message,int *len);
       void HttpGet(QString redirect_url);
       void u8ToStr(uint8_t *input,QString *output,int len,int *outLen);
       void StrTou8(QString *input,uint8_t *output,int len,int *outLen);
        void Jsonc(uint8_t n);
        void MoneyC(QString *input,uint8_t *output,int len);
        void MoneyS(uint8_t *input, QString *output,int len);
        void TimeStart();
private:
    Ui::MainWindow *ui;

    QTcpServer   *listenSocket;  // 侦听套接字
    QTcpSocket   *readWriteSocket;
    QNetworkRequest request;
    QNetworkAccessManager *qnam;
    QNetworkReply *reply;
    QTimer *timer;
    //QLabel *ql;
private slots:
    void  processConnection();
    void  processText();
        void slot_httpFinish();
        void finishedTime();
};

#endif // MAINWINDOW_H
