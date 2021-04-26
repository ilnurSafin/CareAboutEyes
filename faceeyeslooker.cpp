#include "faceeyeslooker.h"

using namespace std;
using namespace cv;


FaceEyesLooker::FaceEyesLooker(QObject* parent)
    :QObject(parent)
{
    window_name = "Capture - Face detection";
    face_cascade_name = ".\\lbpcascades\\lbpcascade_frontalface.xml";//parser.get<string>("face_cascade");
    eyes_cascade_name = ".\\haarcascades\\haarcascade_eye_tree_eyeglasses.xml";//parser.get<string>("eyes_cascade");

    //-- 1. Load the cascades
    if (!face_cascade.load(face_cascade_name) && !face_cascade.load(("D:\\opencv\\sources\\data" + cv::String(face_cascade_name.c_str()+1))))
        throw CascadeOpenException("--(!)Error loading face cascade\n");
    if (!eyes_cascade.load(eyes_cascade_name) && !eyes_cascade.load("D:\\opencv\\sources\\data" + cv::String(eyes_cascade_name.c_str()+1)))
        throw CascadeOpenException("--(!)Error loading eyes cascade\n");

}

FaceEyesLooker::~FaceEyesLooker()
{
    stop();
}


void exec(FaceEyesLooker* l)
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

void FaceEyesLooker::detecting()
{
    std::vector<Rect> faces;
    Mat frame_gray;
    static bool previousEyesState = false;
    static int numberOfFaces = 0;
    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);

    //-- Detect faces
    face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(100, 120));

    if(numberOfFaces != faces.size())
    {
        emit numberOfFacesChanged(numberOfFaces = faces.size());
    }

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
}

bool FaceEyesLooker::isActivated()
{
    return thread_activated;
}

void FaceEyesLooker::start()
{
    if( ! pthread)
    {
        if(!capture.isOpened())
        {
            capture.open(0);
            if (!capture.isOpened())
                throw CameraOpenException("--(!)Error opening video capture\n");
        }
        capture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
        capture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
        thread_activated = true;
        pthread = std::shared_ptr<std::thread>(new std::thread(exec, this));

    }
}

void FaceEyesLooker::stop()
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

void FaceEyesLooker::setTurnON(bool on)
{
    if(! on)
        stop();
    else
        start();
}
