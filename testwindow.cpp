#include "testwindow.h"
#include "ui_testwindow.h"
#include "json.hpp"
#include <string>
QString downloadProgress;
QString size;
long long ms;
MDownload* down;
using json=nlohmann::json;
TestWindow::TestWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestWindow)
{
    ui->setupUi(this);

}

void callBack2(MDownload* d){

}
void callBack(MDownload* d){
    downloadProgress=d->MDownload::getDownloaded();
    size=d->MDownload::getSize();
    ms=d->MDownload::getUsedMS();
}
TestWindow::~TestWindow()
{
    delete ui;
}

void TestWindow::on_pushButton_clicked()
{
    down=new MDownload(ui->lineEdit->text(),"E:/test/",nullptr);
    connect(down,&MDownload::process,[=](MDownload*a){
        ui->download->setText(QString::number(down->getDownloadedBytes()));
        ui->size->setText(QString::number(down->getSizeBytes()));
        ui->time->setText(" Speed:"+
                          QString::number(down->getSpeed()));
    });
    down->unzip();
}


void TestWindow::on_pushButton_2_clicked()
{
    auto jso=ui->textEdit->toPlainText().toStdString();
    auto j=json::parse(jso);
    auto str=j[0]["name"];
    qDebug()<<str.type_name()<<QString::fromStdString(jso);

    ui->data0name->setText(QString::fromStdString(str.get<std::string>()));
}

