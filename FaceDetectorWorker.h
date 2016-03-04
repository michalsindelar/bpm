//
// Created by Michal on 04/03/16.
//

#ifndef BPM_FACEDETECTOR_H
#define BPM_FACEDETECTOR_H


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

class FaceDetectorWorker {
    vector<Rect> faces;
    bool working;
    Mat frame;
    CascadeClassifier faceCascade;

    public:
        FaceDetectorWorker();
        void detectFace(Mat frame);

        const vector<Rect> &getFaces() const {
            return faces;
        }

        void setFrame(const Mat &frame) {
            FaceDetectorWorker::frame = frame;
        }

        void setWorking(bool working) {
            FaceDetectorWorker::working = working;
        }


        bool isWorking() const {
            return working;
        }

        void setFaces(const vector<Rect> &faces) {
            FaceDetectorWorker::faces = faces;
        }
};


#endif //BPM_FACEDETECTOR_H
