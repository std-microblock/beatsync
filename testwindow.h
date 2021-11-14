#ifndef TESTWINDOW_H
#define TESTWINDOW_H
#include <mdownload.h>
#include <QDialog>

namespace Ui {
class TestWindow;
}

class TestWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TestWindow(QWidget *parent = nullptr);
    ~TestWindow();
    static void OnDownloadProgress(MDownload* md);
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::TestWindow *ui;

};

#endif // TESTWINDOW_H
