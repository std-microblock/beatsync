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
#include <QPair>

using nljson=nlohmann::json;

#define maxDownloadSametime getSetting("maxDownloadNum").toInt()
#define CustomLevelsPath getSetting("customLevels")

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


    {
        //        qDebug()<<QString::fromStdString(global::profile["cachePath"].get<std::string>());
        if(global::profile["cachePath"].is_string())readCache(QString::fromStdString(global::profile["cachePath"].get<std::string>()));
    }


    {
        QStringList downloadHeader;
        downloadHeader<<"文件名"<<"进度"<<"速度";
        ui->downloadList->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->downloadList->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->downloadList->setSelectionMode(QAbstractItemView::SingleSelection);
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
    //    ui->downloadList->setVerticalScrollMode(ScrollMode::);

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
        ui->subscribes->setColumnWidth(1,275);
        ui->subscribes->setColumnWidth(2,100);
        ui->subscribes->setColumnWidth(3,129);
        ui->subscribes->horizontalHeader()->setDisabled(true);
        ui->subscribes->verticalHeader()->setDisabled(true);

        connect(ui->subscribes,
                &QTableWidget::itemSelectionChanged,
                [=](){
            int currentRow=ui->subscribes->currentRow();
            qDebug()<<currentRow;
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
            ui->subscribes->
                    setItem(row-1,1,new QTableWidgetItem(nlJsonStr(pset["description"])));
            ui->subscribes->
                    setItem(row-1,2,new QTableWidgetItem(nlJsonStr(pset["author"])));
            QWidget* pWidget = new QWidget();
            QPushButton* btnSync = new QPushButton();
            btnSync->setText("同步");
            btnSync->setStyleSheet("padding:2px 10px;border-radius:2px");
            connect(btnSync,&QPushButton::clicked,[=](){
                for(int x=0;x<maps.length();x++){
                    if(maps[x].matchJSONFilter(pset["filter"])){
                        downloadList.push_back(maps[x]);
                    }
                }
            });

            QPushButton* btn_edit2 = new QPushButton();
            btn_edit2->setText("编辑");
            QPushButton* btn_edit3 = new QPushButton();
            btn_edit3->setText("刷新");
            btn_edit3->setStyleSheet("padding:2px 10px;border-radius:2px");
            QHBoxLayout* pLayout = new QHBoxLayout(pWidget);
            pLayout->addWidget(btnSync);
            //            pLayout->addWidget(btn_edit2);
            pLayout->addWidget(btn_edit3);
            pLayout->setAlignment(Qt::AlignCenter);
            pLayout->setContentsMargins(0, 0, 0, 0);
            pWidget->setLayout(pLayout);
            ui->subscribes->setCellWidget(row-1, 3, pWidget);

        }
    }

    // Download list
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
    MDownload *down=new MDownload(cacheAddr,QDir::currentPath(),nullptr);
    down->unzip();
    connect(down,&MDownload::finishedUnzipping,[=](MDownload *down){
        global::profile["cachePath"]=down->getUnzipPath().toStdString();
        readCache(down->getUnzipPath());
    });
}


void MainWindow::on_settingsBtn_clicked()
{
    settingWin.show();
    settingWin.setFocus();
}

