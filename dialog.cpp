#include "dialog.h"
#include "ui_dialog.h"
#include "sidepanel.h"
#include "QMessageBox"
#include "QDir"
#include "QFileDialog"
#include "QSettings"
#include <iostream>
using namespace std;



QSettings settings("EmptySet", "CareAboutEyes");
int breaktime = settings.value("breaktime",600000).toInt();
int interval = settings.value("interval",70000).toInt();

SidePanel::Pos pos_panel = (SidePanel::Pos)settings.value("pos_panel",3).toInt();//read
float sliding_panel = settings.value("sliding_panel",0.5f).toFloat();

bool is_user_song =  settings.value("is_user_song",false).toBool();
QString user_song =  settings.value("user_song","D:/Downloads/immortals.mp3").toString();
QString default_song =  settings.value("default_song","samsung.mp3").toString();
int valume =  settings.value("valume",50).toInt();//read
bool is_muted =  settings.value("is_muted",true).toBool();

bool notif_face_out_of_camera =  settings.value("notif_face_out_of_camera",true ).toBool();
bool cameraActivated =  settings.value("cameraActivated",true).toBool();

double k = settings.value("k",1.).toDouble();

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    icon(new QSystemTrayIcon(this))
{
    ui->setupUi(this);

    icon->setIcon(QIcon(":/img/eye_icon.png"));
    icon->show();
    menu = new QMenu(this);


    icon->setContextMenu(menu);
    connect(menu,SIGNAL(aboutToShow()),this,SLOT(showContexMenuInIcon()));

    setWindowIcon(QIcon(":/img/eye_icon.png"));
    panel = new SidePanel(0,sliding_panel, pos_panel);
    setModal(false);
    connect(panel->getOptionsButtonPointer(),SIGNAL(released()),this,SLOT(show()));

    player = new QMediaPlayer(this);

    try
    {
        looker = new FaceEyesLookerV2(this,k);
        if(cameraActivated)
            looker->start();
    }
    catch(FaceEyesLookerV2::CascadeOpenException e)
    {
        panel->setText("Ошибка открытия\nфайлов программы");
        panel->getButtonPointer()->setText("Закрыть программу");
        connect(panel->getButtonPointer(),SIGNAL(released()), this, SLOT(myexit()));
        panel->show();
    }
    catch(FaceEyesLookerV2::CameraOpenException e)
    {
        panel->setText("Ошибка открытия камеры");
        panel->getButtonPointer()->setText("Закрыть программу");
        connect(panel->getButtonPointer(),SIGNAL(released()), this, SLOT(myexit()));
        panel->show();
    }

    connect(&timer,SIGNAL(timeout()), this, SLOT(timeout()));
    connect(&rest_timer, SIGNAL(timeout()),this,SLOT(comebackFromBrake()));
    connect(looker,SIGNAL(numberOfFacesChanged(int)), this, SLOT(nOfFacesChanged(int)));
    connect(looker,SIGNAL(eyesChangedState(bool)), this, SLOT(eyesStateChanged(bool)));

    connect(ui->slider_of_panel,SIGNAL(sliderMoved(int)),this, SLOT(movePanel(int)));
    ui->combo_pos_of_panel->addItem("снизу");
    ui->combo_pos_of_panel->addItem("сверху");
    ui->combo_pos_of_panel->addItem("слева");
    ui->combo_pos_of_panel->addItem("справа");

    ui->combo_pos_of_panel->setCurrentIndex(pos_panel);
    connect(ui->combo_pos_of_panel,SIGNAL(currentIndexChanged(int)), panel,SLOT(setPos(int)));
    ui->slider_of_panel->setValue(sliding_panel*ui->slider_of_panel->maximum());

    ui->sound_groupBox->setChecked(!is_muted);
    player->setMuted(is_muted);

    QDir dir(QApplication::applicationDirPath()+"\\sounds\\","*.mp3");
    QFileInfoList songs = dir.entryInfoList(QStringList("*.mp3"));

    int index_of_song = -1;
    int i;
    for(i = 0; i < songs.size(); i++)
    {
        ui->combo_song->addItem(songs.at(i).fileName());
        if(default_song == songs.at(i).fileName())
        {
            ui->combo_song->setCurrentIndex(index_of_song = i);
            if(!is_user_song)
                player->setMedia(QUrl(songs.at(i).absoluteFilePath()));//setMedia(QUrl::fromLocalFile(QDir::toNativeSeparators(QApplication::applicationDirPath()+"/sounds/"+songs.at(i).fileName())));
        }
    }

    if(index_of_song == -1 && !is_user_song)
    {
        ui->combo_song->setCurrentIndex(0);
        default_song = songs.at(0).fileName();
        index_of_song = 0;
        player->setMedia(QUrl(songs.at(0).absoluteFilePath()));
    }


    if(is_user_song)
    {
        ui->radioB_user_songs->setChecked(true);

        player->setMedia(QUrl::fromLocalFile(QDir::toNativeSeparators(user_song)));
    }
    else
        ui->radioB_default_songs->setChecked(true);

    connect(ui->valume,SIGNAL(sliderMoved(int)), player, SLOT(setVolume(int)));
    connect(ui->sound_groupBox,SIGNAL(toggled(bool)), this, SLOT(onoffSound(bool)));
    ui->valume->setValue(valume);
    connect(ui->radioB_default_songs,SIGNAL(toggled(bool)), this, SLOT(setDefaultSong(bool)));
    connect(ui->combo_song,SIGNAL(currentIndexChanged(QString)), this, SLOT(changeDefaultSong(QString)));

    connect(ui->choice_user_song_button,SIGNAL(released()),this,SLOT(chooseUserSong()));



    ui->interval_m_spin->setValue(interval/1000/60);
    ui->interval_m_spin->valueChanged(interval/1000/60);
    ui->interval_s_spin->setValue(interval/1000%60);

    ui->rest_h_spin->setValue(breaktime/1000/60/60);
    ui->rest_h_spin->valueChanged(breaktime/1000/60/60);
    ui->rest_m_spin->setValue(breaktime/1000/60%60);

    ui->cb_notif_face_out_of_camera->setChecked(notif_face_out_of_camera);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::movePanel(int a)
{
    panel->move_by_sliding(sliding_panel = float(a)/ui->slider_of_panel->maximum());
}

void Dialog::myexit()
{
    interval = ui->interval_s_spin->value()*1000 + ui->interval_m_spin->value()*60*1000;
    breaktime = ui->rest_m_spin->value()*60*1000 + ui->rest_h_spin->value()*60*60*1000;

    settings.setValue("breaktime",breaktime);
    settings.setValue("interval",interval);

    settings.setValue("pos_panel",ui->combo_pos_of_panel->currentIndex());//read
    settings.setValue("sliding_panel",sliding_panel);

    settings.setValue("is_user_song",is_user_song);
    settings.setValue("user_song",user_song);
    settings.setValue("default_song",default_song);
    settings.setValue("valume",ui->valume->value());//read
    settings.setValue("is_muted",is_muted);
    settings.setValue("notif_face_out_of_camera",notif_face_out_of_camera);

    settings.setValue("k", k);
    QApplication::setQuitLockEnabled(true);
    close();
}

void Dialog::showContexMenuInIcon()
{
    menu->clear();
    menu->addSeparator();
    menu->addAction(looker->isActivated()?"Приостановить работу программы":"Возобновить работу программы" , this, SLOT(turnCamera()))->setEnabled(this->isHidden());
    menu->addAction("Обновить порог определения открытости глаз" , this, SLOT(updateValueOfOpenedEyes()))->setEnabled(this->isHidden());

    menu->addAction("Настройки", this, SLOT(show()));
    menu->addSeparator();
    menu->addAction("О программе", this, SLOT(about()));
    menu->addAction("Выход" , this, SLOT(myexit()));
}

void Dialog::about()
{
    QMessageBox::about(this,"О программе \'Забота о глазах\'","Версия 1.0.0\n"
                             "\'Забота о глазах\' - программа предназначенная для людей, проводящих длительное время за компьютером. Программа будет напоминать Вам о статичности глаз, если Вы не моргали более указанного интервала времени. Промежуток времени выбираете самостоятельно. Если у Вас возникнут предложения или пожелания по приложению, пожалуйста, пишите нам на контактную почту.\n"
                       "Адрес электронный почты разработчиков:\n"
                       "rocknrollmgn@gmail.com");
}

void Dialog::turnCamera()
{
    try{
        if(looker->isActivated())
        {
            turnOffCamera();
        }
        else
        {
            //if(isHidden())
                cameraActivated = true;
            if(rest_timer.isActive())
                rest_timer.stop();
            looker->start();
        }
    }catch(FaceEyesLookerV2::CameraOpenException e)
    {
        panel->setText("Ошибка открытия камеры");
        panel->getButtonPointer()->setText("Закрыть программу");
        connect(panel->getButtonPointer(),SIGNAL(released()), this, SLOT(myexit()));
        panel->show();
    }
}

void Dialog::turnOffCamera()
{
    if(isHidden())
       cameraActivated = false;
    looker->stop();
    timer.stop();
    panel->hide();
}


void Dialog::eyesStateChanged(bool state)
{
    if(state == false)
    {
        panel->hide();
        timer.stop();
        player->stop();
    }
    else
    {
        timer.start(interval);
    }
}

void Dialog::nOfFacesChanged(int n)
{
    if(n==1)
    {
        timer.start(interval);
        panel->getButtonPointer()->setText("перерыв");
        panel->getButtonPointer()->show();
        panel->setText("Моргните");
        connect(panel->getButtonPointer(),SIGNAL(released()), this, SLOT(rest()));
        panel->hide();
    }
    else{
        timer.stop();
        if(notif_face_out_of_camera)
        {
            if(n==0)
                panel->setText("Ваше лицо не\nпопадает в кадр\n или под углом");
            else
                panel->setText("В кадре более\nчем одно лицо");
            panel->getButtonPointer()->hide();
            panel->show();
        }
    }
}

void Dialog::rest()
{
    rest_timer.start(breaktime);
    turnOffCamera();
}

void Dialog::timeout()
{
    panel->show();
    player->play();

}

void Dialog::comebackFromBrake()
{
    rest_timer.stop();
    turnCamera();
}

void Dialog::show()
{
    QDialog::show();
    turnOffCamera();

    panel->setText("Открыты настройки");
    panel->getButtonPointer()->setVisible(false);
    panel->show();
}

void Dialog::onoffSound(bool on)
{
    player->setMuted(!on);
    is_muted = !on;
}

void Dialog::setDefaultSong(bool def)
{
    if(def)
    {
        player->setMedia(QUrl::fromLocalFile(QDir::toNativeSeparators(QApplication::applicationDirPath()+"/sounds/"+default_song)));
    }else
    {
        player->setMedia(QUrl::fromLocalFile(QDir::toNativeSeparators(user_song)));
    }
    is_user_song = !def;
}

void Dialog::changeDefaultSong(QString s)
{
    default_song = s;
    if(!is_user_song)
        player->setMedia(QUrl::fromLocalFile(QDir::toNativeSeparators(QApplication::applicationDirPath()+"/sounds/"+default_song)));
}

void Dialog::chooseUserSong()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Выберите звуковой файл", "С:/","*.mp3");
    if(!fileName.isEmpty())
    {
        user_song = fileName;
        if(is_user_song)
            player->setMedia(QUrl::fromLocalFile(QDir::toNativeSeparators(user_song)));
    }
}

void Dialog::updateValueOfOpenedEyes()
{
    looker->updateValueOfOpenedEyes();
}

void Dialog::closeEvent(QCloseEvent * a)
{

    panel->hide();
    panel->setText("Моргните");
    panel->getButtonPointer()->setVisible(true);

    interval = ui->interval_s_spin->value()*1000 + ui->interval_m_spin->value()*60*1000;
    breaktime = ui->rest_m_spin->value()*60*1000 + ui->rest_h_spin->value()*60*60*1000;
    if(cameraActivated)
        turnCamera();
    QDialog::closeEvent(a);
}

void Dialog::on_interval_m_spin_valueChanged(int m_interval)
{
    if(m_interval>0)
        ui->interval_s_spin->setMinimum(0);
    else
        ui->interval_s_spin->setMinimum(5);
}

void Dialog::on_rest_h_spin_valueChanged(int h_rest)
{
    if(h_rest>0)
        ui->rest_m_spin->setMinimum(0);
    else
        ui->rest_m_spin->setMinimum(10);
}

void Dialog::on_cb_notif_face_out_of_camera_toggled(bool checked)
{
    notif_face_out_of_camera = checked;
}
