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

    ui->maxDownloadNum->setRange(1,10);
    ui->maxDownloadNum->setValue(getSetting("maxDownloadNum").toInt());

    ui->customLevelsPosition->setText(getSetting("customLevels"));

    ui->cacheDownload->setDisabled(true);
}

SettingsWin::~SettingsWin()
{
    delete ui;
}

void SettingsWin::on_pushButton_clicked()
{
    fillSetting("downloadServer",ui->comboBox->currentText());


    QApplication::closeAllWindows();
    QProcess::startDetached(QCoreApplication::applicationFilePath());
}

