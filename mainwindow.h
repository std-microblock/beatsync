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
