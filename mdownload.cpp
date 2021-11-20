#include "mdownload.h"
#include <QString>
#include <QObject>
#include <iostream>
#include <QDebug>
#include <QDateTime>
#include <iostream>
#include "global.h"
using namespace std;
#define ARIA2C_Path "tools/aria2c/aria2c.exe"
#define SZIP_Path "tools/7zip/7z.exe"

#define putIntoData(a,b,c) if(dt[0]==a)b=c;
#define putIntoDataD(a,b) if(dt[0]==a)b=dt[1];

long long MDownload::getUsedMS(){
    return max(this->endTime,QDateTime::currentMSecsSinceEpoch())-this->startTime;
}

long long CovertToBytes(QString c){

    QString number;
    for(int x=0;x<c.length()&&(c[x].isNumber()||c[x]=='.');x++)
        number+=c[x];
    double value=number.toDouble();

    if(c.endsWith("KiB"))
        return value*1024;
    if(c.endsWith("MiB"))
        return value*1048576;
    if(c.endsWith("GiB"))
        return value*1073741824;
    if(c.endsWith("GB"))
        return value*1000000000;
    if(c.endsWith("MB"))
        return value*1000000;
    if(c.endsWith("B"))
        return value;
    return -1;
}


void MDownload::unzip(){
    needToUnzip=1;
    if(isFinished())_unzip();
}

void MDownload::_unzip(){
    QObject::connect(&szipProcess,&QProcess::readyReadStandardOutput ,[&](){
        auto out=szipProcess.readAllStandardOutput();
        if(out.replace("\r","\n").indexOf("Everything is Ok\n")!=-1){
//           if(finishedCallBack)finishedCallBack(this);
            emit finishedUnzipping(this);
            QFile downloadFile(realFilePath);
            downloadFile.remove();
        }
    });
    QStringList args;

    if(realFilePath.endsWith(".zip"))unzipPath=realFilePath.left(realFilePath.length()-4);
    else unzipPath=realFilePath+"_extracted";
    args<<"x"<<QString(realFilePath).replace("//","/")<<"-o"+unzipPath<<"-y";
    this->szipProcess.start(SZIP_Path,args);
//    qDebug()<<"Unzip args:"<<args.join(" ");

}

MDownload::MDownload(QString url,QString path,QObject* pa=nullptr)
                     :QObject(pa)
{
    if(!path.size())path=QString("/tmp/");
    url=url.replace("\\","/");
    path=path.replace("\\","/");
    this->startTime=QDateTime::currentMSecsSinceEpoch();
    qDebug()<<"Downloading - "<<url;

    QDir downloadDir(path);
    auto ent=downloadDir.entryList(QStringList("*.aria2"));
    for(auto i:ent){
        qDebug()<<i;
        QFile(i).remove();
    }

    QObject::connect(&this->aria2cProcess, &QProcess::readyReadStandardOutput ,[&](){
        auto output=QString(this->aria2cProcess.readAllStandardOutput())
                .replace("\r"," ")
                .replace("\n"," ")
                .replace("["," ")
                .replace("]"," ");
        auto match=output.split(" ");
        if(match.length()<3||output=="")return;

        while(match.length()!=0&&match[0]=="")match.removeFirst();
        if(match.length()==0)return;
//        qDebug()<<"[Aria2c] "+output;

        if(match[0].startsWith("#")){
            size=match[1].split("/")[1];
            if(this->size.lastIndexOf("(")>=0)size=size.split("(")[0];
            downloaded=match[1].split("/")[0];
            realDownload=CovertToBytes(downloaded);
            realSize=CovertToBytes(size);
            for(auto i:match){
                if(i.indexOf(":")>=0){
                    auto dt=i.split(":");
                    putIntoData("CN",connections,dt[1].toInt());
                    putIntoData("DL",speed,CovertToBytes(dt[1]));
                    putIntoDataD("ETA",eta);
                }
            }
//            std::cout<<realDownload<<"B/"<<realSize<<"B  "<<"  --  "<<speed/1024<<"KiB/s\n";
            emit process(this);
            return;
        }
        if(match.length()>7&&match[8]=="complete:"&&!isFinished()){
            finished=true;
            realSize=realDownload;
            QStringList pathList;
            for(int x=9;x<match.length();x++){
                if(match[x].length())pathList.push_back(match[x]);
                else break;
            }
            realFilePath=pathList.join(" ");
            endTime=QDateTime::currentMSecsSinceEpoch();
            if(needToUnzip)_unzip();
            emit finishedDownloading(this);

//            else if(finishedCallBack)finishedCallBack(this);

        }
        if(match.length()>2&&match[1]=="complete:"&&!isFinished()){
            finished=true;
            realSize=realDownload;
            QStringList pathList;
            for(int x=2;x<match.length();x++){
                if(match[x].length())pathList.push_back(match[x]);
                else break;
            }
            realFilePath=pathList.join(" ");
            endTime=QDateTime::currentMSecsSinceEpoch();
            if(needToUnzip)_unzip();
            emit finishedDownloading(this);
//            else if(finishedCallBack)finishedCallBack(this);

        }
        //        if(match[0].startsWith())
    });
    QObject::connect(&this->aria2cProcess, &QProcess::readyReadStandardError ,[=](){
        qDebug()<<this->aria2cProcess.readAllStandardError();
    });
    QStringList args;
    args<<"-s"<<"1"<<"-x"<<"1"<<url<<"--dir="+path<<"--allow-overwrite=true";
    if(getSetting("enableProxy").toInt())args<<"--all-proxy="+getSetting("proxyAddr");
    //    args<<"/c"<<"echo"<<"12345";
    this->aria2cProcess.start(ARIA2C_Path,args);
}
