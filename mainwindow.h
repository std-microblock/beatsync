#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <testwindow.h>
#include <QList>
#include <QPair>
#include <mdownload.h>
#include <settingswin.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static QString autoGetGamePath(){
        QSettings reg("HKEY_LOCAL_MACHINE\\SOFTWARE\\Valve\\Steam",QSettings::NativeFormat);
        QSettings reg2("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Valve\\Steam",QSettings::NativeFormat);
        auto steamPath=reg2.value("InstallPath",reg.value("InstallPath").toString()).toString();
        QFile fileSteamLib(steamPath+"\\steamapps\\libraryfolders.vdf");
        fileSteamLib.open(QFile::ReadOnly);
        auto dfile=QString(fileSteamLib.readAll());
        auto paths=dfile.split("\"path\"\t\t\"");
        for(auto path:paths){
            auto libpath=path.split("\"")[0].replace(":\\\\",":\\");
            QFile file(libpath+"\\steamapps\\common\\Beat Saber\\Beat Saber.exe");
            if(file.exists()){
               return(libpath+"\\steamapps\\common\\Beat Saber\\");
            }
        }
    }

private slots:
    void on_testsBtn_clicked();

    void on_updateCacheBtn_clicked();

    void on_settingsBtn_clicked();

private:
    QList<MDownload> dList;
    Ui::MainWindow *ui;
    TestWindow testWin;
    SettingsWin settingWin;
};
#endif // MAINWINDOW_H
