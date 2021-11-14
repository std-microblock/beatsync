#include "settingswin.h"
#include "ui_settingswin.h"
#include "global.h"
#include <QProcess>


SettingsWin::SettingsWin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsWin)
{
    ui->setupUi(this);
    {
        QStringList items;items<<"na."<<"as."<<"eu.";
        ui->comboBox->addItems(items);
        ui->comboBox->setCurrentIndex(items.indexOf(getSetting("downloadServer")));
    }

    ui->maxDownloadNum->setRange(1,16);
    ui->maxDownloadNum->setValue(getSetting("maxDownloadNum").toInt());

    ui->customLevelsPosition->setText(getSetting("customLevels"));

    ui->cacheDownload->setDisabled(true);

    ui->enableHTTP->setChecked(getSetting("enableProxy").toInt());
    ui->HTTPProxyAddr->setText(getSetting("proxyAddr"));
    ui->HTTPProxyAddr->setDisabled(!getSetting("enableProxy").toInt());
}

SettingsWin::~SettingsWin()
{
    delete ui;
}

void SettingsWin::on_pushButton_clicked()
{
    fillSetting("downloadServer",ui->comboBox->currentText());
    fillSetting("customLevels",ui->customLevelsPosition->text());
    fillSetting("maxDownloadNum",ui->maxDownloadNum->text());
    fillSetting("enableProxy",ui->enableHTTP->isChecked()?"1":"0");
    fillSetting("proxyAddr",ui->HTTPProxyAddr->text());

    QApplication::closeAllWindows();
    QProcess::startDetached(QCoreApplication::applicationFilePath());
}


void SettingsWin::on_enableHTTP_stateChanged(int arg1)
{
    ui->HTTPProxyAddr->setDisabled(!ui->enableHTTP->isChecked());
}

