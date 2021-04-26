#ifndef FACEEYESLOOKERV2_H
#define FACEEYESLOOKERV2_H

#include <QObject>
#include <dlib/opencv.h>
#include <opencv2/highgui/highgui.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>

#include<opencv2/opencv.hpp>
#include<opencv2/opencv_modules.hpp>


#include <iostream>
#include <stdio.h>
#include <thread>
#include <exception>
#include <memory>
#include <mutex>

class FaceEyesLookerV2 : public QObject
{
    Q_OBJECT
private:
    cv::String face_cascade_name, eyes_cascade_name;
    cv::CascadeClassifier face_cascade;
    cv::CascadeClassifier eyes_cascade;
    cv::String window_name;
    cv::VideoCapture capture;
    cv::Mat frame;
    std::shared_ptr<std::thread> pthread;
    //std::thread* pthread;
    bool thread_activated;
    std::mutex m;

    dlib::frontal_face_detector detector;
    dlib::shape_predictor pose_model;
    bool init_level_of_opened_eyes;
    double k;
public: friend void execV2(FaceEyesLookerV2* l);
public:
    FaceEyesLookerV2(QObject* parent, double koeficient_for_def_openes_eyes);
    ~FaceEyesLookerV2();
    void detecting();
    bool isActivated();

    double getKoefficient();
    class CameraOpenException : std::exception
    {
        std::string wtf;
    public:
        CameraOpenException(std::string s)
            :wtf(s){}
        const char *what()
        {
            return wtf.data();
        }
    };
    class CascadeOpenException : std::exception
    {
        std::string wtf;
    public:
        CascadeOpenException(std::string s)
            :wtf(s){}
        const char *what()
        {
            return wtf.data();
        }
    };
public slots:
    void start();
    void stop();
    void setTurnON(bool);

    void updateValueOfOpenedEyes();
signals:
    void eyesChangedState(bool);
    void numberOfFacesChanged(int);
};

#endif // FACEEYESLOOKERV2_H
