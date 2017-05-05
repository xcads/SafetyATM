#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "MD5.h"
#include "SMS4.h"
#include "ProductNum.h"
#include "SM3.h"
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <qdatetime.h>
#include <QTextCodec>
uint32_t key[4] = { 0, 0, 0, 0 };
uint32_t xKey[4] = { 0, 0, 0, 0 };
uint8_t xMess[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t s[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t k[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t b[8]={0,0,0,0,0,0,0,0};
uint32_t bKey[4]={0,0,0,0};
uint32_t xT[4] = { 0, 0, 0, 0 };
uint32_t kT[4] = { 0, 0, 0, 0 };
uint32_t IMEI[8]={0,0,0,0,0,0,0,0};
char Name[20];
uint8_t Money[4]={0,0,0,0};
QByteArray HTTPdata;
int HTTPflag=1;
int SUCCflag=0;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
     qnam = new QNetworkAccessManager();
     memset(Name,0,sizeof(char)*20);

}

MainWindow::~MainWindow()
{
    //listenSocket->destroyed();
	listenSocket->close();
   // delete ui;
}
void MainWindow::ininServer()
{
    this->listenSocket =new QTcpServer;
    this->listenSocket->listen(QHostAddress::Any,10000);

    QObject::connect(this->listenSocket,SIGNAL(newConnection()),this,SLOT(processConnection()));

}
void MainWindow::u8ToStr(uint8_t *input,QString *output,int len,int *outLen)
{
    int i,j;
    int temp;
    for(i=0,j=0;i<len;i++,j+=2)
    {
        temp=input[i]&0x0f;
        if(temp>=10)
        {
            (*output)[j+1]=temp%10+'a';
        }
        else
        {
           (*output)[j+1]=temp+'0';
        }
        temp=(input[i]>>4)&0x0f;
        if(temp>=10)
        {
            (*output)[j]=temp%10+'a';
        }
        else
        {
            (*output)[j]=temp+'0';
        }
    }
    *outLen=j-1;
}
void MainWindow::StrTou8(QString *input,uint8_t *output,int len,int *outLen)
{
    int i,j;
    uint8_t temp;
    char *c;
    QByteArray byte=input->toLatin1();
    c=byte.data();
    for(i=0,j=0;i<len;j++,i+=2)
    {
        if(c[i]>='a')
        {
            temp=c[i]-'a'+10;
        }
        else
        {
            temp=c[i]-'0';
        }
        temp<<=4;
        if(c[i+1]>='a')
        {
            temp|=c[i+1]-'a'+10;
        }
        else
        {
            temp|=c[i+1]-'0';
        }
        output[j]=temp;
    }
    *outLen=j;
}
void MainWindow::MoneyC(QString *input,uint8_t *output,int len)
{
    int i,j;
    uint64_t temp=0;
    int u=0;
    char *c;
    QByteArray byte=input->toLatin1();
    c=byte.data();
    for(i=len-1,j=0;i>=0;i--,j++)
    {
        u=c[i]-'0';
        for(int k=0;k<j;k++)
        {
            u*=10;
        }
        temp+=u;
    }
    for(i=3;i>=0;i--)
    {
        output[i]=temp&0xff;
        temp>>=8;
    }
}
void MainWindow::MoneyS(uint8_t *input,QString *output,int len)
{
    int i,j;
    uint64_t temp=0;
    int u=10;
	temp |= input[0];
    for(i=1;i<len;i++)
    {
		temp <<= 8;
        temp|=input[i];

    }
    for(i=0,j=7;i<8;i++,j--)
    {

        (*output)[j]=(char)(temp%10+'0');
		temp/=10;
    }
}

void MainWindow::Jsonc(uint8_t n)
{
    QString Str;
    for(int i=0;i<HTTPdata.length();i++)
    {
        Str[i]=HTTPdata[i];
    }
        QScriptEngine engine;
        QScriptValue sc=engine.evaluate("value="+Str);
        QString byte;
        int Len;
        if(n==0x07)
        {
            uint8_t mdKey[16];
            SMS4 SMS4_Seed;
            byte=sc.property("name").toString();
			//int a = byte.length();
            //StrTou8(&byte,Name,byte.length(),&Len);
			QByteArray a = byte.toLatin1();
            for(int i=0;i<a.length();i++)
            {
                Name[i]=a[i];
            }
            byte.clear();
            byte=sc.property("k").toString();
            StrTou8(&byte,mdKey,byte.length(),&Len);
            SMS4_Seed.ConvertTo32(key,mdKey,16,&Len);
            byte.clear();
            byte=sc.property("x").toString();
            StrTou8(&byte,mdKey,16,&Len);
            SMS4_Seed.ConvertTo32(xKey,mdKey,16,&Len);
            byte.clear();
        }
        if(n==0x0c)
        {
            char c[5];
			c[4] = 0;
            for(int i=0;i<4;i++)
                c[i]=(char)globalString[i+4];
            if(strcmp(HTTPdata.data(),"0")==0)
            {
                HTTPflag=0;
            }
            else
            {
                HTTPflag=1;
                if(strcmp(c,"CHEK")==0)
                {
                    byte=sc.property("name").toString();
                    QByteArray a=byte.toLatin1();
                    for(int i=0;i<a.length();i++)
                    {
                        Name[i]=a[i];
                    }
                    byte.clear();
                    byte=sc.property("money").toString();
                    //StrTou8(&byte,Money,byte.length(),&Len);
                    MoneyC(&byte,Money,byte.length());
                    byte.clear();
                }
            }


        }
}
void MainWindow::TimeStart()
{
    timer=new QTimer();
    timer->start(2000);
    connect(timer,SIGNAL(timeout()),this,SLOT(finishedTime()));
}
void MainWindow::finishedTime()
{
    memset(b,0,sizeof(uint8_t)*8);
    memset(bKey,0,sizeof(bKey)*8);
    ui->label->setText("Time is out!");
}

void MainWindow::HttpGet(QString redirect_url)
{
    QString url="http://218.244.128.209/airpay/";
    if(redirect_url.length()!=0)
    {
        url=url+redirect_url;
    }
    request.setUrl(QUrl(url));
    reply=qnam->get(request);
    QEventLoop loop;
    connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));
    loop.exec();
    slot_httpFinish();
}
void MainWindow::slot_httpFinish()
{
    HTTPdata.clear();
    HTTPdata=reply->readAll();
   Jsonc(globalString[0]);
   // ui->label->setText(data);
}
void MainWindow::processConnection()
{
     this->readWriteSocket =this->listenSocket->nextPendingConnection();
     QObject::connect(this->readWriteSocket,SIGNAL(readyRead()),this,SLOT(processText()));
}
void MainWindow::BK(uint32_t *key, uint32_t *message)
{
    for (int i = 0; i<4; i++)
    {
        key[i] = message[i];
    }
}
void MainWindow::processText()
{
    //this->globalString+=this->readWriteSocket->readAll();
    char buf[5];
    int numbytes;
    int rs=1;
    int i=0;
    MD5 MD5_Seed;
    SMS4 SMS4_Seed;
	SM3 SM3_Seed;
	state_hash sm3;
    ProductNum Num_Seed;
    uint8_t Message[2000];
    int md5Len;
	int sm3Len;
    int revLen=0;
    len = 0;
    qint8 Tk[5]={0x10,0x10,0x10,0x10,0};
    memset(globalString, 0, sizeof(uint8_t) * 2048);
    memset(Message,0,sizeof(uint8_t)*2000);
    while(rs)
    {

        memset(buf,0,5*sizeof(char));
        numbytes = this->readWriteSocket->read(buf,4);
                if (numbytes < 0)
                {
                    if (errno == EAGAIN)
                    {
                        break;
                    }
                }
                else if (numbytes == 0)
                {

                }
                if (numbytes != sizeof(buf)-1)
                    rs = 0;
                else
                    rs = 1;
                if (strcmp(buf, (char *)Tk) == 0)
                    break;
                if (i == 0)
                {
                    strcpy((char *)this->globalString, buf);
                    i++;
                }
                else
                {
                    for (int i = 0; i < 4; i++)
                        globalString[i + revLen] = buf[i];
                }
                   // strcat((char *)this->globalString, buf);
                revLen += 4;
    }

    if(globalString[0]==0x01)
    {
        if(strcmp((char *)(globalString+4),"REGT")==0)
        {
            ui->label_2->setText("The Register Time");
            ui->label->setText("Welcome to Register in");
           // QDateTime n=QDateTime::currentDateTime();
            //QDateTime now;
            //do
            //{
            //    now=QDateTime::currentDateTime();
            //}while(n.secsTo(now)<=6);
            uint32_t sm3Message[8];
            uint32_t sms4Message[8];
			uint32_t tmpKey[4];
           Num_Seed.numPro(s);
            Num_Seed.numPro(k);
			SM3_Seed.sm3_init(&sm3);
			SM3_Seed.encode(&sm3, s, sm3Message, 8);
			SMS4_Seed.Encode(sm3Message, sm3Message + 4, tmpKey, 4);
           // MD5_Seed.getMd5(s,md5Message,8);
            BK(key, tmpKey);
			SM3_Seed.sm3_init(&sm3);
			SM3_Seed.encode(&sm3, k, sm3Message, 8);
            //MD5_Seed.MD5_init();
            //MD5_Seed.getMd5(k,md5Message,8);
            SMS4_Seed.Encode(sm3Message,key,sms4Message,8);
			SMS4_Seed.Encode(sm3Message, sm3Message + 4, tmpKey, 4);
            for (int i = 0; i < 4; i++)
                key[i] = tmpKey[i];
            Message[0]=0x02;
            for(int i=1;i<4;i++)
            {
                Message[i]=globalString[i];
            }
            MD5_Seed.Dncode(Message+4,sms4Message,32,&md5Len);
            len=md5Len+4;
            send(Message,&len);
            ui->label_2->setText("The num of Registing ");
            QString byte;
            for(int i=0;i<8;i++)
            {
                byte[i]=(char )s[i];
            }
            ui->label->setText(byte);
        }
        else
        {
            for(int i=0;i<4;i++)
            {
                Message[i]=globalString[i];
            }
            len=4;
            send(Message,&len);
        }
    }

    if(globalString[0]==0x03)
    {
        if(revLen!=388)//356MD5
        {
            Message[0]=0x0d;
            for(int i=1;i<4;i++)
            {
                Message[i]=globalString[i];
            }
            len=4;
            send(Message,&len);
        }
        else
        {
            uint32_t tempMessage[100];
            uint8_t tempIMEI[32];
			uint32_t tempKey[4];
            QString com,sendHTTP;
            com.clear();
            int qstrLen;
            memset(tempMessage, 0, sizeof(uint32_t) * 100);
            int tempLen=0;
            int flag=0;
            SMS4_Seed.ConvertTo32(tempMessage,globalString+4,revLen-4,&tempLen);
            SMS4_Seed.SMS4_init();
            SMS4_Seed.Dncode(tempMessage,key,tempMessage,tempLen);
            for(int i=0;i<8;i++)
            {
                IMEI[i]=tempMessage[80+i];//SM3(IMEI)
            }
            MD5_Seed.Dncode(tempIMEI,IMEI,32,&md5Len);
            u8ToStr(tempIMEI,&com,32,&qstrLen);
            sendHTTP="save.php?IMEI="+com;
            uint8_t T[400];
           uint32_t sm3Message[8];
           memset(T, 0, sizeof(uint8_t) * 400);
            MD5_Seed.Dncode(T,tempMessage,352,&sm3Len);//336
			SM3_Seed.sm3_init(&sm3);
			SM3_Seed.encode(&sm3, T, sm3Message, sm3Len);
            //MD5_Seed.MD5_init();
            //MD5_Seed.getMd5(T,md5Message,md5Len);
            for(int i=0;i<8;i++)
            {
                if(tempMessage[i+88]!=sm3Message[i])
                {
                    Message[0]=0x0d;
                    flag=1;
                    break;
                }
            }
            if(flag==0)
            {
                Message[0]=0x04;
            }
            for(int i=1;i<4;i++)
                Message[i]=globalString[i];
            Num_Seed.numPro(k);
            Num_Seed.numPro(xMess);
			SM3_Seed.sm3_init(&sm3);
			SM3_Seed.encode(&sm3, xMess, sm3Message, 8);
           // MD5_Seed.MD5_init();
            //MD5_Seed.getMd5(xMess,md5Message,8);
			SMS4_Seed.Encode(sm3Message, sm3Message + 4, tempKey, 4);
            for(int i=0;i<4;i++)
                xKey[i]=tempKey[i];
            SMS4_Seed.SMS4_init();
            SMS4_Seed.Encode(tempMessage,xKey,tempMessage,88);//84
			SM3_Seed.sm3_init(&sm3);
			SM3_Seed.encode(&sm3, k, sm3Message, 8);
            //MD5_Seed.MD5_init();
            //MD5_Seed.getMd5(k,md5Message,8);
            for(int i=0,j=88;i<8;i++,j++)
                tempMessage[j]=sm3Message[i];
            MD5_Seed.Dncode(T,tempMessage,384,&sm3Len);//352
			SM3_Seed.sm3_init(&sm3);
			SM3_Seed.encode(&sm3, T, sm3Message, sm3Len);
            //MD5_Seed.MD5_init();
            //MD5_Seed.getMd5(T,md5Message,md5Len);
            for(int i=0,j=96;i<8;i++,j++)
            {
                tempMessage[j]=sm3Message[i];
            }
            SMS4_Seed.SMS4_init();
            SMS4_Seed.Encode(tempMessage,key,tempMessage,104);//92
            MD5_Seed.Dncode(Message+4,tempMessage,416,&len);//368
            len+=4;
            send(Message,&len);
			SM3_Seed.sm3_init(&sm3); 
			SM3_Seed.encode(&sm3, k, sm3Message, 8);
            //MD5_Seed.MD5_init();
            //MD5_Seed.getMd5(k,md5Message,8);
			SMS4_Seed.SMS4_init();
			SMS4_Seed.Encode(sm3Message, sm3Message + 4, tempKey, 4);
            for (int i = 0; i < 4; i++)
                key[i] = tempKey[i];
            MD5_Seed.Dncode(tempIMEI,key,16,&md5Len);
            com.clear();
            u8ToStr(tempIMEI,&com,16,&qstrLen);
            sendHTTP=sendHTTP+"&k="+com;
            MD5_Seed.Dncode(tempIMEI,xKey,16,&md5Len);
            com.clear();
            u8ToStr(tempIMEI,&com,16,&qstrLen);
            sendHTTP=sendHTTP+"&x="+com;
            HttpGet(sendHTTP);
        }
    }

    if(globalString[0]==0x0b)
    {
        if (strcmp((char *)(globalString + 4), "SUCE") == 0)
        {
            QMessageBox::information(NULL,"Successful","Register Suecceed",QMessageBox::Ok,QMessageBox::Ok);
            QDateTime n=QDateTime::currentDateTime();
            QDateTime now;
            do
            {
                now=QDateTime::currentDateTime();
            }while(n.secsTo(now)<=3);
        }
    }
    if(globalString[0]==0x05)
    {
        if(strcmp((char *)(globalString+4),"LOGN")==0)
        {
            memset(Name,0,sizeof(char)*20);
            Message[0]=0x06;
            for(int i=1;i<4;i++)
            {
                Message[i]=globalString[i];
            }
            char ac[4]={'A','C','C','P'};
            for(int i=4;i<8;i++)
            {
                Message[i]=(uint8_t)ac[i-4];
            }
            len=8;
            send(Message,&len);
        }
        else
        {
            Message[0]=0x0d;
            for(int i=1;i<4;i++)
            {
                Message[i]=globalString[i];
            }
            len=4;
            send(Message,&len);
        }
    }

    if(globalString[0]==0x07)
    {
        uint32_t sm3Message[8],sms4Message[8];
		uint32_t tempKey[4];
        ui->label_2->setText("Users: ");
        QString byte,HTTPsend;
        int qstrLen;
		uint8_t tempIMEI[32];
       u8ToStr(globalString+4,&byte,32,&qstrLen);
       HTTPsend="getkx.php?IMEI="+byte;
	   StrTou8(&byte, tempIMEI, byte.length(), &qstrLen);
	   SMS4_Seed.ConvertTo32(IMEI, tempIMEI, 32, &qstrLen);
	   HTTPdata.clear();
       HttpGet(HTTPsend);
	   //while (1)
	   //{
		  // if (HTTPdata.length() != 0)
			 //  break;
	   //}
	 //  Jsonc(globalString[0]);
       QTextCodec *gbk=QTextCodec::codecForName("GB18030");
       QTextCodec *utf8=QTextCodec::codecForName("UTF-8");
       QString a;
       a=gbk->toUnicode(Name);
        ui->label->setText(a);
        Num_Seed.numPro(b);
		SM3_Seed.sm3_init(&sm3);
		SM3_Seed.encode(&sm3, b, sm3Message, 8);
        //MD5_Seed.MD5_init();
        //MD5_Seed.getMd5(b,md5Message,4);
        SMS4_Seed.SMS4_init();
        SMS4_Seed.Encode(sm3Message,key,sms4Message,8);
        Message[0]=0x08;
        for(int i=1;i<4;i++)
        {
            Message[i]=globalString[i];
        }
        MD5_Seed.Dncode(Message+4,sms4Message,32,&md5Len);
        len=md5Len+4;
        send(Message,&len);
		SMS4_Seed.SMS4_init();
		SMS4_Seed.Encode(sm3Message, sm3Message + 4, tempKey, 4);
        for(int i=0;i<4;i++)
        {
            bKey[i]=tempKey[i];
        }

    }

    if(globalString[0]==0x09)
    {
		if (revLen != 708)//676
        {
            Message[0]=0x0d;
            for(int i=1;i<4;i++)
            {
                Message[i]=globalString[i];
            }
            len=4;
            send(Message,&len);
        }
        else
        {
        uint32_t tempMessage[300];
        uint32_t sm3Message[8];
		uint32_t tempKey[4];
        memset(Message, 0, sizeof(uint32_t) * 4);
        memset(tempMessage, 0, sizeof(uint32_t) * 300);
        int tempLen=0;
        int flag=0;
        SMS4_Seed.ConvertTo32(tempMessage,globalString+4,revLen-4,&tempLen);
        SMS4_Seed.SMS4_init();
        SMS4_Seed.Dncode(tempMessage,bKey,tempMessage,tempLen);
        MD5_Seed.Dncode(Message+4,tempMessage,(tempLen-8)*4,&sm3Len);
		SM3_Seed.sm3_init(&sm3);
		SM3_Seed.encode(&sm3, Message + 4, sm3Message, sm3Len);
//        MD5_Seed.MD5_init();
//        MD5_Seed.getMd5(Message+4,md5Message,md5Len);
        for(int i=0;i<8;i++)
        {
            if(sm3Message[i]!=tempMessage[tempLen-8+i])
            {
                Message[0]=0x0d;
                flag=1;
                break;
            }
        }
        uint32_t pwCon[80];
        uint8_t pw1Con[320];
        uint8_t pw0Con[320];
        int success=0;
        SMS4_Seed.SMS4_init();
        SMS4_Seed.Dncode(tempMessage+80,xKey,pwCon,80);
        MD5_Seed.Dncode(pw1Con,pwCon,320,&md5Len);
        MD5_Seed.Dncode(pw0Con,tempMessage,320,&md5Len);
        for(int i=0;i<320;i++)
        {
            if (pw1Con[i] + pw0Con[i] == 0)
            {
                success += 0;
            }
            else
            {
                success += (((int)pw1Con[i] - (int)pw0Con[i])*((int)pw1Con[i] - (int)pw0Con[i])) / (pw1Con[i] + pw0Con[i]);
            }

        }
         if(success<=300000&&flag==0)
         {
           //  QMessageBox::information(NULL,"Successful","LOGIN Suecceed",QMessageBox::Ok,QMessageBox::Ok);
             Message[0]=0x0a;
             Num_Seed.numPro(xMess);
             Num_Seed.numPro(k);
             SMS4_Seed.SMS4_init();
             SMS4_Seed.Dncode(tempMessage + 80, xKey, tempMessage, 88);
			 SM3_Seed.sm3_init(&sm3);
			 SM3_Seed.encode(&sm3, xMess, sm3Message, 8);
			 SMS4_Seed.SMS4_init();
			 SMS4_Seed.Encode(sm3Message, sm3Message + 4, tempKey, 4);
			 //MD5_Seed.MD5_init();
             //MD5_Seed.getMd5(xMess, md5Message, 8);
             for (int i = 0; i<4; i++)
                 xT[i] = tempKey[i];
			 SM3_Seed.sm3_init(&sm3);
			 SM3_Seed.encode(&sm3, k, sm3Message, 8);
			 //MD5_Seed.MD5_init();
             //MD5_Seed.getMd5(k, md5Message, 8);
			 SMS4_Seed.SMS4_init();
			 SMS4_Seed.Encode(sm3Message, sm3Message + 4, tempKey, 4);
             for (int i = 0; i<4; i++)
             {
                 kT[i] = tempKey[i];
             }
             SMS4_Seed.SMS4_init();
             SMS4_Seed.Encode(tempMessage, xT, tempMessage, 88);
             for (int i = 0; i<8; i++)
                 tempMessage[i + 88] = sm3Message[i];
             MD5_Seed.Dncode(Message + 4, tempMessage, 384, &sm3Len);//352
			 SM3_Seed.sm3_init(&sm3);
			 SM3_Seed.encode(&sm3, Message + 4, sm3Message, 384);
            // MD5_Seed.MD5_init();
            // MD5_Seed.getMd5(Message + 4, md5Message, 352);
             for (int i = 0; i<8; i++)
             {
                 tempMessage[i + 96] = sm3Message[i];
             }
             SMS4_Seed.SMS4_init();
             SMS4_Seed.Encode(tempMessage, bKey, tempMessage, 104);
             MD5_Seed.Dncode(Message + 4, tempMessage, 416, &len);
             len += 4;
         }
         else
         {
             Message[0]=0x0d;
             len = 4;
         }
         for(int i=1;i<4;i++)
         {
             Message[i]=globalString[i];
         }

         send(Message,&len);
        }

    }
    if (globalString[0] == 0x0c)
    {
        char a[5];
		a[4] = 0;
        for(int i=0;i<4;i++)
            a[i]=globalString[i+4];
        if (strcmp((char *)(globalString + 4), "SUCE") == 0)
        {
			QString HTTPsend,byte;
			int qstrLen;
			uint8_t tempHTTP[16];
            SUCCflag=1;
			HTTPsend.clear();
			MD5_Seed.Dncode(tempHTTP, IMEI, 32, &md5Len);
			u8ToStr(tempHTTP, &byte, 32, &qstrLen);
			HTTPsend = "save.php?IMEI=" + byte;
			byte.clear();
			MD5_Seed.Dncode(tempHTTP, kT, 16, &md5Len);
			u8ToStr(tempHTTP, &byte, 16, &qstrLen);
			HTTPsend = HTTPsend + "&k=" + byte;
			byte.clear();
			MD5_Seed.Dncode(tempHTTP, xT, 16, &md5Len);
			u8ToStr(tempHTTP, &byte, 16, &qstrLen);
			HTTPsend = HTTPsend + "&x=" + byte;
			HttpGet(HTTPsend);
            /*for (int i = 0; i < 4; i++)
            {
                key[i] = kT[i];
                xKey[i] = xT[i];
            }*/
            QMessageBox::information(NULL, "Successful", "Login!", QMessageBox::Ok, QMessageBox::Ok);
            QDateTime n=QDateTime::currentDateTime();
            QDateTime now;
            do
            {
                now=QDateTime::currentDateTime();
            }while(n.secsTo(now)<=3);
        }
        else if(strcmp(a,"CHEK")==0&&SUCCflag==1)
        {

             if(HTTPflag==0||revLen!=40)
            {
                Message[0]=0x0d;
                for(int i=1;i<3;i++)
                {
                    Message[i]=globalString[i];
                }
                strcpy((char*)Message,"WRON");
                len=8;
                send(Message,&len);
                HTTPflag=1;
            }
            else
            {
                QString HTTPsend,byte;
                int qstrLen;
                u8ToStr(globalString+8,&byte,32,&qstrLen);
                HTTPsend="check.php?IMEI="+byte;
                HttpGet(HTTPsend);
                if(HTTPflag==0)
                {
                    Message[0]=0x0d;
                    for(int i=1;i<3;i++)
                    {
                        Message[i]=globalString[i];
                    }
                    strcpy((char*)Message,"WRON");
                    len=8;
                    send(Message,&len);
                    HTTPflag=1;
                }
                else
                {
                    Message[0]=0x0e;
                    for(int i=1;i<4;i++)
                    {
                        Message[i]=globalString[i];
                    }
                    strcpy((char *)Message+4,"MONY");
                    for (int i = 0; i < 4; i++)
                    {
                        Message[i + 8] = Money[i];
                    }
                //strcpy((char *)Message+8,(char *)Money);
                    strcpy((char *)Message+12,"NAME");
                    strcpy((char *)Message+16,Name);
                    len=16+strlen(Name);
                    send(Message,&len);
                }
            }
        }
        else if(strcmp(a,"GETM")==0&&SUCCflag==1)
        {
            if(HTTPflag==0||revLen!=44)//28
            {
                Message[0]=0x0d;
                for(int i=1;i<3;i++)
                {
                    Message[i]=globalString[i];
                }
                strcpy((char*)Message,"WRON");
                len=8;
                send(Message,&len);
                HTTPflag=1;
            }
            else
            {
                QString HTTPsend,byte;
                int qstrLen;
                u8ToStr(globalString+8,&byte,32,&qstrLen);
                HTTPsend="qu.php?IMEI="+byte;
				for (int i = 0; i < 4; i++)
				{
					Money[i] = globalString[revLen - 4 + i];
				}
               // strcpy((char *)Money,(char *)globalString+revLen-4);
                byte.clear();
                MoneyS(Money,&byte,4);
                HTTPsend=HTTPsend+"&money="+byte;
                HttpGet(HTTPsend);
                if(HTTPflag==0)
                {
                    Message[0]=0x0d;
                    for(int i=1;i<3;i++)
                    {
                        Message[i]=globalString[i];
                    }
                    strcpy((char*)Message,"WRON");
                    len=8;
                    send(Message,&len);
                    HTTPflag=1;
                }
                else
                {
                    Message[0]=0x0f;
                    for(int i=1;i<3;i++)
                    {
                        Message[i]=globalString[i];
                    }
                    strcpy((char *)Message+4,"SUBM");
                    len=8;
                    send(Message,&len);
                }
            }
        }
        else if(strcmp(a,"SAVE")==0&&SUCCflag==1)
        {
            if(HTTPflag==0||revLen!=44)
            {
                Message[0]=0x0d;
                for(int i=1;i<3;i++)
                {
                    Message[i]=globalString[i];
                }
                strcpy((char*)Message,"WRON");
                len=8;
                send(Message,&len);
                HTTPflag=1;
            }
            else
            {
                QString HTTPsend,byte;
                int qstrLen;
                u8ToStr(globalString+8,&byte,32,&qstrLen);
                HTTPsend="cun.php?IMEI="+byte;
				for (int i = 0; i < 4; i++)
				{
					Money[i] = globalString[revLen - 4 + i];
				}
                //strcpy((char *)Money,(char *)globalString+revLen-4);
                byte.clear();
                MoneyS(Money,&byte,4);
                HTTPsend=HTTPsend+"&money="+byte;
                HttpGet(HTTPsend);
                if(HTTPflag==0)
                {
                    Message[0]=0x0d;
                    for(int i=1;i<3;i++)
                    {
                        Message[i]=globalString[i];
                    }
                    strcpy((char*)Message,"WRON");
                    len=8;
                    send(Message,&len);
                    HTTPflag=1;
                }
                else
                {
                    Message[0]=0x10;
                    for(int i=1;i<3;i++)
                    {
                        Message[i]=globalString[i];
                    }
                    strcpy((char *)Message+4,"ADDM");
                    len=8;
                    send(Message,&len);
                }
            }
        }
        else if(strcmp(a,"CHAN")==0&&SUCCflag==1)
        {
            if(HTTPflag==0||revLen!=60)//44
            {
                Message[0]=0x0d;
                for(int i=1;i<3;i++)
                {
                    Message[i]=globalString[i];
                }
                strcpy((char*)Message,"WRON");
                len=8;
                send(Message,&len);
                HTTPflag=1;
            }
            else
            {
                QString HTTPsend,byte;
                int qstrLen;
                u8ToStr(globalString+8,&byte,32,&qstrLen);
                HTTPsend="change.php?IMEI1="+byte;
                byte.clear();
                u8ToStr(globalString+24,&byte,32,&qstrLen);
                HTTPsend=HTTPsend+"&IMEI2="+byte;
                byte.clear();
				for (int i = 0; i < 4; i++)
				{
					Money[i] = globalString[revLen - 4 + i];
				}
              //  strcpy((char *)Money,(char *)globalString+revLen-4);
                MoneyS(Money,&byte,4);
                HTTPsend=HTTPsend+"&money="+byte;
                HttpGet(HTTPsend);
                if(HTTPflag==0)
                {
                    Message[0]=0x0d;
                    for(int i=1;i<3;i++)
                    {
                        Message[i]=globalString[i];
                    }
                    strcpy((char*)Message,"WRON");
                    len=8;
                    send(Message,&len);
                    HTTPflag=1;
                }
                else
                {
                    Message[0]=0x11;
                    for(int i=1;i<3;i++)
                    {
                        Message[i]=globalString[i];
                    }
                    strcpy((char *)Message+4,"CHAM");
                    len=8;
                    send(Message,&len);
                }
            }
        }
        else
        {
           Message[0]=0x0d;
           for(int i=1;i<3;i++)
           {
               Message[i]=globalString[i];
           }
           strcpy((char*)Message,"WRON");
           len=8;
           send(Message,&len);
           SUCCflag=0;
        }
    }
    if(globalString[0]==0xff)
    {
        SUCCflag=0;
        ui->label->setText("欢迎使用");
        ui->label_2->setText("");
        ui->label->setText("AIR");

        qApp->exit(25);
    }

 //   send(globalString,&len);

   // this->readWriteSocket->write(byte);
    //ui->label->setText((char *)globalString);
}
void MainWindow::sendReady()
{
    flag=1;
}


int MainWindow::send(uint8_t *Message,int *len)
{
    int total = 0;
    int bytesleft = *len;
    int n;
  /*  QByteArray byte;
    for(int i=0;i<*len;i++)
    {
        byte[i]=Message[i];
    }*/
    while (total < *len) {
        n = this->readWriteSocket->write((char *)Message,4);
      //  this->readWriteSocket->flush();
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
        Message += 4;
    }
    *len = total;
    flag=0;
    return n == -1 ? -1 : 0;
    //QByteArray block;
    //QDataStream out(&block,QIODevice::WriteOnly);
    //out.setVersion(QDataStream::Qt_5_3);
    //out<<(quint16)0;
    //out<<Message;
    //out.device()->seek(0);
    //out<<(quint16)(block.size()-sizeof(qint16));
    //this->readWriteSocket->write(block,4);
    //*len=0;
    //return 0;
}
