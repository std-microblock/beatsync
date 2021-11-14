#ifndef MDOWNLOAD_H
#define MDOWNLOAD_H
#include <QString>
#include <QProcess>
#include <QtCore>
#include <QObject>
#include <math.h>
//typedef void callBackFn(MDownload*);

class MDownload:public QObject
{
    Q_OBJECT
private:

    long long startTime=0,endTime=0,speed=0,
    realSize=0,realDownload=0;
    QProcess aria2cProcess,szipProcess;
    QString url,path,eta;
    double percent;
    int connections;
    bool needToUnzip;bool finished=false;
    QString unzipPath,realFilePath;
    QString size,downloaded;
    void onReadOutput();
    void _unzip();
 signals:
    void process(MDownload*);
    void finishedDownloading(MDownload*);
    void finishedUnzipping(MDownload*);
public:
    bool isFinished(){return finished;}
    long long getUsedMS();
    long long getSizeBytes(){return this->realSize;};
    long long getDownloadedBytes(){return this->realDownload;};
    QString getSize(){return this->size;};
    QString getDownloaded(){return this->downloaded;};
    double getSpeed(){return speed;}
    double getPercent(){return (double)realDownload/(double)fmax(1,realSize);};
    void unzip();
    bool isUnzipped(){return unzipPath.length();};
    QString getUnzipPath(){return unzipPath;};

    MDownload(QString url,QString path,QObject* pa);
};

#endif // MDOWNLOAD_H
