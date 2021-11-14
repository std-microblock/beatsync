#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include "global.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile profileFile("profile.json");
    profileFile.open(QFile::ReadWrite);
    auto profiley=profileFile.readAll();
    if(profileFile.isOpen()&&profiley.length()!=0)
    {
        try{
        global::profile=nljson::parse(profiley);
        }catch(nljson::parse_error pe){
            qDebug()<<pe.what();
        }
    }else{
        QFile presetProfileFile(":/presets/profile.json");
        presetProfileFile.open(QFile::ReadOnly);
        if(presetProfileFile.isOpen())
        {
            try{
            global::profile=nljson::parse(presetProfileFile.readAll());
            }catch(nljson::parse_error pe){
                qDebug()<<pe.what();
            }
            presetProfileFile.close();
        }else{
            global::profile=nljson::parse("{}");
        }
    }


    QFile presetProfileFile(":/presets/profile.json");
    presetProfileFile.open(QFile::ReadOnly);
    if(presetProfileFile.isOpen())
    {
        try{
        global::profilePreset=nljson::parse(presetProfileFile.readAll());
        }catch(nljson::parse_error pe){
            qDebug()<<pe.what();
        }
        presetProfileFile.close();
    }


    MainWindow w;
    QString qss;
    QFile qssFile(":/styles/style.qss");
    qssFile.open(QFile::ReadOnly);
    if(qssFile.isOpen())
    {
        qss = QLatin1String(qssFile.readAll());
        qApp->setStyleSheet(qss);
        qssFile.close();
    }




    w.show();
    auto result=a.exec();
    profileFile.remove();
    profileFile.open(QFile::ReadWrite);
    profileFile.write(QString::fromStdString(global::profile.dump()).toUtf8());
    return result;
}
