#include <QCoreApplication>
#include "mainclass.h"

#define MAIN_LOGSIZE 1024 * 10000 * 10

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
   QHash<QtMsgType,QString> msgLevelHash;
   msgLevelHash[QtDebugMsg] = "Debug";
   msgLevelHash[QtInfoMsg] = "Info";
   msgLevelHash[QtWarningMsg] = "Warning";
   msgLevelHash[QtCriticalMsg] = "Critical";
   msgLevelHash[QtFatalMsg] = "Fatal";
   QByteArray localMsg = msg.toLocal8Bit();
   QDateTime time = QDateTime::currentDateTime();
   QString formattedTime = time.toString("yyyy-MM-dd hh:mm:ss:zzz");
   QString fileName_1 =  QString("./log/InfoLog_%1_.txt").arg(QDate::currentDate().toString("yyyy_MM_dd"));
   QString fileName_2 =  QString("./log/DebugLog_%1_.txt").arg(QDate::currentDate().toString("yyyy_MM_dd"));
   QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();
   QString logLevelName = msgLevelHash[type];
   QByteArray logLevelMsg = logLevelName.toLocal8Bit();
   QDir dir;
   if (!dir.exists("./log"))
       dir.mkpath("./log");
   QFile outFileCheck_1(fileName_1);
   QString NewFileName_1 = fileName_1;

   QFile outFileCheck_2(fileName_2);
   QString NewFileName_2 = fileName_2;
   int size_1 = outFileCheck_1.size();
   int counter_1 = 0 ;

   int size_2 = outFileCheck_2.size();
   int counter_2 = 0 ;

   while(size_1 > MAIN_LOGSIZE){
       counter_1++;
       NewFileName_1 = fileName_1 + QString::number(counter_1);
       QFile outFileCheck(NewFileName_1);
       size_1 = outFileCheck.size();
       QApplication::processEvents(QEventLoop::AllEvents);
   }

   while(size_2 > MAIN_LOGSIZE){
       counter_2++;
       NewFileName_2 = fileName_2 + QString::number(counter_2);
       QFile outFileCheck(NewFileName_2);
       size_2 = outFileCheck.size();
       QApplication::processEvents(QEventLoop::AllEvents);
   }

   if(msg != ""){
       QString txt = QString("%1").arg(msg);
       if(type == QtInfoMsg){
           QFile outFile(NewFileName_1);
           outFile.open(QIODevice::WriteOnly | QIODevice::Append);
           QTextStream ts(&outFile);
           ts << txt << endl;
           outFile.close();
       }
       else{
           QFile outFile(NewFileName_2);
           outFile.open(QIODevice::WriteOnly | QIODevice::Append);
           QTextStream ts(&outFile);
           ts << txt << endl;
           outFile.close();
       }


   }
   else{
       QString txt = "";
       QFile outFile(NewFileName_2);
       outFile.open(QIODevice::WriteOnly | QIODevice::Append);
       QTextStream ts(&outFile);
       ts << txt << endl;
       outFile.close();
   }

   return;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    #ifdef QT_DEBUG
        qDebug().noquote() << "debug to console";
    #endif
    #ifndef QT_DEBUG
        qInstallMessageHandler(myMessageOutput);
        qDebug().noquote() << "debug to file";
    #endif




//    if(argc > 1){
//        mainclass* m{};
//        m = new mainclass(nullptr, argv[1]);
//    }
//    else{
////        mainclass* m{};
////        m = new mainclass(nullptr, "update.hex");
////        m->f_writeToSerial("test");
//        qDebug() << "please provide file name as first option . . .";
//        return 10;
//    }

    mainClass *M;
    M = new mainClass;

    for(int i{0}; i < argc; ++i){
        qDebug().noquote() << "arg["  << QString::number(i) << "] = " << argv[i];
        if(i == 1){ //host name
            M->hName = QString::fromUtf8(argv[i]);
            M->hn = true;
        }
        else if(i == 2){ //host port
            M->hPort = static_cast<quint16>(QString::fromUtf8(argv[i]).toInt());
            M->hp = true;
        }
        else if(i == 3){ //user name
            M->uName = QString::fromUtf8(argv[i]);
            M->un = true;
        }
        else if(i == 4){ //pass word
            M->pWord = QString::fromUtf8(argv[i]);
            M->pw = true;
        }
    }
    M->RunClass();

    return a.exec();
}
