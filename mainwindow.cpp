#include "mainwindow.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QWidget>
#include "ui_mainwindow.h"
#include "json.hpp"
#include <QTimer>
#include <QDir>
#include "global.h"
#include "bsmap.hpp"
#include <QCryptographicHash>
#include <QPair>


using nljson=nlohmann::json;

#define maxDownloadSametime getSetting("maxDownloadNum").toInt()
#define CustomLevelsPath getSetting("gamePath")+"/Beat Saber_Data/CustomLevels"
#define PlistPath getSetting("gamePath")+"/Playlists"
#define cacheAddr "https://share.wgzeyu.top/savercache/getcache.php?type=csv"
#define nlJsonStr(a) QString::fromStdString(a.get<std::string>())

QList<BSMap> maps;
QList<BSMap> downloadList;
QList<QPair<BSMap,MDownload*>> downloadingMap;

void readCache(QString path){
    QFile csv(path+"/LocalCache.csv");
    csv.open(QFile::ReadOnly);
    auto tm=QDateTime::currentMSecsSinceEpoch();
    qDebug()<<"Reading tmp";
    auto csvstr=QString::fromUtf8(csv.readAll());
    qDebug()<<"Reading tmp "+QString::number(QDateTime::currentMSecsSinceEpoch()-tm)+"ms";
    maps=BSMap::fromCSVMaps(&csvstr);
    qDebug()<<"Parse finished "+QString::number(QDateTime::currentMSecsSinceEpoch()-tm)+"ms";
}

int findInDownloading(BSMap map){
    for(int x=0;x<downloadingMap.length();x++){
        if(map.id==downloadingMap[x].first.id)return x;
    }
    return -1;
}

int findInDownloadList(BSMap map){
    for(int x=0;x<downloadList.length();x++){
        if(map.id==downloadList[x].id)return x;
    }
    return findInDownloading(map);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->subscribeDetail->hide();

    if(getSetting("gamePath").startsWith("unset")){
        fillSetting("gamePath",MainWindow::autoGetGamePath());
        this->settingWin.reInitUI();
    }

    // Update temp
    {
    ui->updateCacheBtn->click();
    {
        //        qDebug()<<QString::fromStdString(global::profile["cachePath"].get<std::string>());
//        if(global::profile["cachePath"].is_string())readCache(QString::fromStdString(global::profile["cachePath"].get<std::string>()));
//        else
    }
    this->setDisabled(true);
    }
    // Initize downloading list
    {
        QStringList downloadHeader;
        downloadHeader<<"文件名"<<"进度"<<"速度";
        ui->downloadList->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->downloadList->setFocusPolicy(Qt::NoFocus);
        ui->downloadList->setSelectionMode(QAbstractItemView::NoSelection);
        ui->downloadList->setShowGrid(false);
        ui->downloadList->setColumnCount(downloadHeader.count());
        ui->downloadList->setHorizontalHeaderLabels(downloadHeader);
        ui->downloadList->horizontalHeader()->setDisabled(true);
        ui->downloadList->verticalHeader()->setDisabled(true);
        ui->downloadList->setColumnWidth(0,140);
        ui->downloadList->setColumnWidth(2,70);
        ui->downloadList->setColumnWidth(1,61);
        ui->downloadList->setRowCount(maxDownloadSametime);
    }
    // Initize presets
    {
        QStringList presetsHeader;
        presetsHeader<<"名称"<<"介绍"<<"作者"<<"操作";
        ui->subscribes->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->subscribes->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->subscribes->setSelectionMode(QAbstractItemView::SingleSelection);


        ui->subscribes->setColumnCount(presetsHeader.count());
        ui->subscribes->setHorizontalHeaderLabels(presetsHeader);

        ui->subscribes->setColumnWidth(0,60);
        ui->subscribes->setColumnWidth(1,233);
        ui->subscribes->setColumnWidth(2,100);
        ui->subscribes->setColumnWidth(3,165);
        ui->subscribes->horizontalHeader()->setDisabled(true);
        ui->subscribes->verticalHeader()->setDisabled(true);

        connect(ui->subscribes,
                &QTableWidget::itemSelectionChanged,
                [=](){
            int currentRow=ui->subscribes->currentRow();
            ui->subscribeName->setText(ui->subscribes->item(currentRow,0)->data(Qt::DisplayRole).toString());
            ui->subscribeDesc->setText(ui->subscribes->item(currentRow,1)->data(Qt::DisplayRole).toString());
            ui->subscribeAuthor->setText(ui->subscribes->item(currentRow,2)->data(Qt::DisplayRole).toString());
            ui->subscribeDetail->show();
        });

        QFile fPresets(":/presets/preset.json");
        fPresets.open(QFile::ReadOnly);
        auto presets=nljson::parse(fPresets.readAll());

        int row=0;
        for(auto pset:presets){
            row++;
            ui->subscribes->setRowCount(row);
            ui->subscribes->
                    setItem(row-1,0,new QTableWidgetItem(nlJsonStr(pset["name"])));
            auto item=new QTableWidgetItem(nlJsonStr(pset["description"]));item->setToolTip(nlJsonStr(pset["description"]));
            ui->subscribes->
                    setItem(row-1,1,item);
            ui->subscribes->
                    setItem(row-1,2,new QTableWidgetItem(nlJsonStr(pset["author"])));
            QWidget* pWidget = new QWidget();
            QPushButton* btnSync = new QPushButton();
            btnSync->setText("同步");
            btnSync->setStyleSheet("padding:2px 10px;border-radius:2px");
            connect(btnSync,&QPushButton::clicked,[=](){
                int counter=0,actualDown=0;
                QDir songsDir(CustomLevelsPath);
                QDir plistDir(PlistPath);
                songsDir.setFilter(QDir::Dirs);
                auto songList=songsDir.entryList();
                auto plists=plistDir.entryList();
                QString plistName=nlJsonStr(pset["description"])+"_beatsync"+".bplist";
                nljson plistJson;
                auto plistFile=QFile(PlistPath+"/"+plistName);


                if(getSetting("autoFillPlaylist")=="1"){

                    plistFile.open(QFile::ReadOnly);
                    try{
                        plistJson=nljson::parse(plistFile.readAll());
                    }catch(...){
                        plistJson=nljson::parse("{}");
                        plistJson["playlistTitle"]=pset["name"];
                        plistJson["playlistAuthor"]=pset["author"];
                        plistJson["PlaylistDescription"]=(QString("[ 由BeatSync(https://github.com/MicroCBer/beatsync)自动生成 ] ")+QString(nlJsonStr(pset["description"]))).toStdString();
                    }

                    plistJson["songs"]=nljson::array();
                    plistFile.close();
                    plistFile.open(QFile::WriteOnly);
                }

                for(int x=0;x<maps.length();x++){
                    try{
                    if(!(!pset["limit"].is_number_integer()||
                         counter<pset["limit"].get<int>())){
                        break;
                    }

                    if(!(!pset["number"].is_number_integer()||
                         actualDown<pset["number"].get<int>())){
                        break;
                    }
                    }catch(nljson::other_error err){
                        qDebug()<<err.what();
                        return 0;
                    }

                    bool flagExists=false;
                    for(int a=0;a<songList.size();a++){
                        //                        qDebug()<<maps[x].id;
                        if(songList[a].startsWith(maps[x].id)){
                            flagExists=true;
                            break;
                        }
                    }



                    if(maps[x].matchJSONFilter(pset["filter"])){
                        auto obj=nljson::object();
                        obj["hash"]=maps[x].hash.toStdString();
                        plistJson["songs"].push_back(obj);
                        counter++;
                        if(flagExists||findInDownloadList(maps[x])!=-1)continue;
                        actualDown++;
                        downloadList.push_back(maps[x]);
                    }
                }
                qDebug()<<PlistPath+"/"+plistName;
                if(getSetting("autoFillPlaylist")=="1"){

                    plistFile.write(QString::fromStdString(plistJson.dump()).toUtf8());
                    plistFile.close();
                }
            });

            QPushButton* btn_edit2 = new QPushButton();
            btn_edit2->setText("编辑");
            QPushButton* btn_edit3 = new QPushButton();
            btn_edit3->setText("刷新");
            btn_edit3->setStyleSheet("padding:2px 10px;border-radius:2px");
            QHBoxLayout* pLayout = new QHBoxLayout(pWidget);
            pLayout->addWidget(btnSync);
            pLayout->addWidget(btn_edit2);
            pLayout->addWidget(btn_edit3);
            pLayout->setAlignment(Qt::AlignCenter);
            pLayout->setContentsMargins(0, 0, 0, 0);
            pWidget->setLayout(pLayout);
            ui->subscribes->setCellWidget(row-1, 3, pWidget);

        }
    }
    // Downloading list events
    {
        QTimer* timer=new QTimer(this);
        connect(timer,&QTimer::timeout,[=](){
#define n2QS(a) QString::number(a)
            ui->dListTips->setText(QString(n2QS(downloadingMap.length())+"/"+n2QS(downloadList.length())));

            ui->downloadList->clearContents();
            for(int x=0;x<downloadingMap.length();x++){
                ui->downloadList->setItem(x,0,new QTableWidgetItem(downloadingMap[x].first.name));
                ui->downloadList->setItem(x,1,new QTableWidgetItem(QString::number(round(downloadingMap[x].second->getPercent()*100),'g')+"%"));
                ui->downloadList->setItem(x,2,new QTableWidgetItem(QString::number(round(downloadingMap[x].second->getSpeed()/1024),'g')+"K/s"));
            }

            for(int x=0;(x<maxDownloadSametime-downloadingMap.length())&&(x<downloadList.length());x++){
                QPair<BSMap,MDownload*> pai;

                pai.first=downloadList[0];
                auto url=downloadList[0].url;
                url=url.replace("na.",getSetting("downloadServer"));
                url=url.replace("as.",getSetting("downloadServer"));
                url=url.replace("eu.",getSetting("downloadServer"));
                pai.second=new MDownload(url,CustomLevelsPath,nullptr);
                downloadList.pop_front();
                pai.second->unzip();
                connect(pai.second,&MDownload::process,[=](MDownload* d){
                    try{
                        //                        auto row=findInDownloading(pai.first);
                        //                        ui->downloadList->setItem(row,0,new QTableWidgetItem(pai.first.name));
                        //                        ui->downloadList->setItem(row,1,new QTableWidgetItem(QString::number(round(pai.second->getPercent()*100),'g')+"%"));
                        //                        ui->downloadList->setItem(row,2,new QTableWidgetItem(QString::number(round(pai.second->getSpeed()/1024),'g')+"K/s"));
                    }catch(...){
                        qDebug()<<"Error";
                    }
                });
                connect(pai.second,&MDownload::finishedDownloading,[=](){
                    //                    for(int x=0;x<3;x++)
                    //                        ui->downloadList->setItem(findInDownloading(pai.first),x,new QTableWidgetItem(""));
                    downloadingMap.removeAt(findInDownloading(pai.first));
                });
                downloadingMap.push_back(pai);
            }
        });
        timer->start(100);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_testsBtn_clicked()
{
    testWin.show();
    testWin.setFocus();
}


void cacheDownloadFinished(MDownload* d){
    auto tempPath=d->getUnzipPath();

};

void cacheDownloadProcess(MDownload* d){

};

void MainWindow::on_updateCacheBtn_clicked()
{
    ui->dCtrlTip->setText("正在下载缓存..");
    MDownload *down=new MDownload(cacheAddr,QDir::currentPath(),nullptr);
    down->unzip();
    connect(down,&MDownload::finishedDownloading,[=](MDownload *down){
        ui->dCtrlTip->setText("正在解析缓存..");
    });
    connect(down,&MDownload::finishedUnzipping,[=](MDownload *down){
        global::profile["cachePath"]=down->getUnzipPath().toStdString();
        readCache(down->getUnzipPath());
        ui->dCtrlTip->setText("缓存解析成功.");
        this->setDisabled(false);
    });
}


void MainWindow::on_settingsBtn_clicked()
{
    settingWin.show();
    settingWin.setFocus();
}

