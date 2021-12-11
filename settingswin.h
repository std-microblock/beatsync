#ifndef SETTINGSWIN_H
#define SETTINGSWIN_H

#include <QDialog>

namespace Ui {
class SettingsWin;
}

class SettingsWin : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWin(QWidget *parent = nullptr);
    ~SettingsWin();
void reInitUI();
private slots:
    void on_pushButton_clicked();

    void on_enableHTTP_stateChanged(int arg1);

    void on_pushButton_2_clicked();

private:
    Ui::SettingsWin *ui;
};

#endif // SETTINGSWIN_H
