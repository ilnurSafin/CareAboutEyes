#include "faceeyeslookerV2.h"
#include <dlib/opencv.h>
#include <opencv2/highgui/highgui.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <iostream>

#include<opencv2/opencv.hpp>
#include<opencv2/opencv_modules.hpp>
#define FACE_DOWNSAMPLE_RATIO 4
#define SKIP_FRAMES 2
#define DLIB_PNG_SUPPORT
#define DLIB_JPEG_SUPPORT
using namespace cv;
using namespace std;
using namespace dlib;

double dist(point a, point b)
{
    point r;
    r.x() = a.x() - b.x();
    r.y() = a.y() - b.y();
    return sqrt(r.x()*r.x() + r.y()*r.y());
}

FaceEyesLookerV2::FaceEyesLookerV2(QObject* parent, double koeficient_for_def_openes_eyes)
    :QObject(parent),
      detector(get_frontal_face_detector()),
      k(koeficient_for_def_openes_eyes > 0? koeficient_for_def_openes_eyes:1)
{
    window_name = "Capture - Face detection";
    face_cascade_name = ".\\lbpcascades\\lbpcascade_frontalface.xml";//parser.get<string>("face_cascade");
    eyes_cascade_name = ".\\haarcascades\\haarcascade_eye_tree_eyeglasses.xml";//parser.get<string>("eyes_cascade");

    //-- 1. Load the cascades
    if (!face_cascade.load(face_cascade_name) && !face_cascade.load(("D:\\opencv\\sources\\data" + cv::String(face_cascade_name.c_str()+1))))
        throw CascadeOpenException("--(!)Error loading face cascade\n");
    if (!eyes_cascade.load(eyes_cascade_name) && !eyes_cascade.load("D:\\opencv\\sources\\data" + cv::String(eyes_cascade_name.c_str()+1)))
        throw CascadeOpenException("--(!)Error loading eyes cascade\n");

    try{
    deserialize(".\\shape_predictor_68_face_landmarks.dat") >> pose_model;
    }catch(serialization_error& e)
    {
        throw CascadeOpenException("--(!)Error loading eyes cascade\n");
        //ПЕРЕДЕЛАЙ ильнур
    }
}

FaceEyesLookerV2::~FaceEyesLookerV2()
{
    stop();
}


void execV2(FaceEyesLookerV2* l)
{
    while ( l->thread_activated && l->capture.read(l->frame))
    {

        if (l->frame.empty())
        {
            printf(" --(!) No captured frame -- Break!");
            break;
        }
        //resize(frame, frame, Size(320, 240));

        //cv::Rect myROI(100, 50, 400, 380);

        //l->frame = l->frame(myROI);
        //-- 3. Apply the classifier to the frame

        /*l->m.try_lock();
        if( ! l->thread_activated)
        {
            imshow(l->window_name,l->frame);
            continue;
        }
        l->m.unlock();*/


        l->detecting();

        char c = (char)waitKey(10);
        if (c == 27) { break; } // escape
    }
}

void FaceEyesLookerV2::detecting()
{
    /*std::vector<Rect> faces;
    Mat frame_gray;
    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);
    */

    static bool previousEyesState = false;
    static int numberOfFaces = 0;


    //-- Detect faces
    //face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(100, 120));

    cv_image<bgr_pixel> cimg(frame);

    // Detect faces
    std::vector<dlib::rectangle> faces = detector(cimg);
    // Find the pose of each face.
    std::vector<full_object_detection> shapes;

    if(numberOfFaces != faces.size())
    {
        emit numberOfFacesChanged(numberOfFaces = faces.size());
    }

    for (unsigned long i = 0; i < faces.size(); ++i)
    {
        //cv::rectangle(temp, cv::Rect(shapes[i]), 255);
        //cv::rectangle(temp, faces[0].tl_corner, faces[0].br_corner);
        shapes.push_back(pose_model(cimg, faces[i]));
    }
    if(shapes.size() > 0)
    {
        full_object_detection& a =shapes[0];

        double heye = dist(a.part(43),a.part(47));
        double weye = dist(a.part(42),a.part(45));


        if(!init_level_of_opened_eyes)
        {
            k = weye/heye;
            init_level_of_opened_eyes = 1;
        }

        /*static int i = 1;
        if(heye/weye*k < 0.6)
        {
            cout << i++ << " closed" << endl;
            emit eyesChangedState(false);
        }else
        {
            emit eyesChangedState(true);
        }*/

        static int ic = 1;
        static int io = 1;
        if(!previousEyesState && heye/weye*k >= 0.6)
        {
            emit eyesChangedState(true);
            previousEyesState = true;
            cout << io++ << " opened" << endl;
        }else if(previousEyesState && heye/weye*k < 0.6)
        {
            emit eyesChangedState(false);
            previousEyesState = false;
            cout << ic++ << " closed" << endl;
        }
        //std::cout << heye<< "\t" << heye/weye*k << std::endl;
    }


/*
    for (size_t i = 0; i < faces.size(); i++)
    {
        Point center(faces[i].x + faces[i].width / 2, faces[i].y + faces[i].height / 2);
        //ellipse(frame, center, Size(faces[i].width / 2, faces[i].height / 2), 0, 0, 360, Scalar(255, 0, 255), 4, 8, 0);

        Mat faceROI = frame_gray(faces[i]);
        std::vector<Rect> eyes;

        //-- In each face, detect eyes
        eyes_cascade.detectMultiScale(faceROI, eyes, 1.1, 4, 0 | CASCADE_SCALE_IMAGE, Size(30, 30), Size(60, 60));


        int n_eyes = 0;
        bool r_eye = 0;
        bool l_eye = 0;
        for (size_t j = 0; j < eyes.size(); j++)
        {
            Point eye_center(faces[i].x + eyes[j].x + eyes[j].width / 2, faces[i].y + eyes[j].y + eyes[j].height / 2);
            //int radius = cvRound((eyes[j].width + eyes[j].height)*0.25);
            if( !l_eye && eye_center.y < center.y && eye_center.x < center.x)
            {
                //circle(frame, eye_center, radius, Scalar(255, 0, 0), 4, 8, 0);
                n_eyes++;
                l_eye = true;
            }
            else if( !r_eye && eye_center.y < center.y && eye_center.x > center.x)
            {
                //circle(frame, eye_center, radius, Scalar(255, 0, 0), 4, 8, 0);
                n_eyes++;
                r_eye = true;
            }
        }

        if(!previousEyesState && n_eyes==2)
        {
            emit eyesChangedState(true);
            previousEyesState = true;
        }else if(previousEyesState && n_eyes!=2)
        {
            emit eyesChangedState(false);
            previousEyesState = false;
        }


    }
    //-- Show what you got
    //imshow(window_name, frame);
*/
}

bool FaceEyesLookerV2::isActivated()
{
    return thread_activated;
}

double FaceEyesLookerV2::getKoefficient()
{
    return k;
}

void FaceEyesLookerV2::start()
{
    if( ! pthread)
    {
        if(!capture.isOpened())
        {
            capture.open(0);
            if (!capture.isOpened())
                throw CameraOpenException("--(!)Error opening video capture\n");
        }
        //capture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
        //capture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
        capture.set(CV_CAP_PROP_FRAME_WIDTH, 352);
        capture.set(CV_CAP_PROP_FRAME_HEIGHT, 264);
        thread_activated = true;
        pthread = std::shared_ptr<std::thread>(new std::thread(execV2, this));

    }
}

void FaceEyesLookerV2::stop()
{
    if(pthread)
    {
        m.lock();
        thread_activated = false;
        m.unlock();

        pthread->join();
        //delete pthread;
        pthread = 0;
        capture.release();
    }
}

void FaceEyesLookerV2::setTurnON(bool on)
{
    if(! on)
        stop();
    else
        start();
}

void FaceEyesLookerV2::updateValueOfOpenedEyes()
{
    init_level_of_opened_eyes = false;
}
