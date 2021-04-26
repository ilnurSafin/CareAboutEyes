#ifndef FACEEYESLOOKER_H
#define FACEEYESLOOKER_H

#include <QObject>
#include "faceeyeslooker.h"
#include <opencv2/objdetect.hpp>
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"


#include <iostream>
#include <stdio.h>
#include <thread>
#include <exception>
#include <memory>
#include <mutex>

class FaceEyesLooker : public QObject
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

public: friend void exec(FaceEyesLooker* l);
public:
    FaceEyesLooker(QObject* parent = nullptr);
    ~FaceEyesLooker();
    void detecting();
    bool isActivated();

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
signals:
    void eyesChangedState(bool);
    void numberOfFacesChanged(int);
};

#endif // FACEEYESLOOKER_H
