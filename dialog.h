#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QSystemTrayIcon>
#include <QMenu>
#include "sidepanel.h"
#include "faceeyeslookerV2.h"
#include <QTimer>
#include <QMediaPlayer>
#include <qmediaplaylist.h>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    void closeEvent(QCloseEvent *);//void closeEvent();
public slots:
    void movePanel(int a);

    void myexit();
    void showContexMenuInIcon();
    void about();
    void turnCamera();
    void turnOffCamera();
    void eyesStateChanged(bool);
    void nOfFacesChanged(int);
    void rest();
    void timeout();
    void comebackFromBrake();

    void show();

    void onoffSound(bool);
    void setDefaultSong(bool);
    void changeDefaultSong(QString);
    void chooseUserSong();

    void updateValueOfOpenedEyes();
private slots:
    void on_interval_m_spin_valueChanged(int arg1);

    void on_rest_h_spin_valueChanged(int arg1);

    void on_cb_notif_face_out_of_camera_toggled(bool checked);

private:
    Ui::Dialog *ui;
    QSystemTrayIcon* icon;
    SidePanel *panel;
    FaceEyesLookerV2 *looker;
    QMenu* menu;
    QTimer timer;
    QTimer rest_timer;
    QMediaPlayer* player;
};

#endif // DIALOG_H
