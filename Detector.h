//
// Created by Michal on 04/03/16.
//

#ifndef BPM_FACEDETECTOR_H
#define BPM_FACEDETECTOR_H


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "config.h"
#include "imageOperation.h"

using namespace cv;
using namespace std;

class Detector {
    vector<Rect> faces;
    bool working;
    Mat frame;
    CascadeClassifier faceCascade;

    float faceHeightScale; // Increase height
    float faceYOffset; // Move face up

    public:
        Detector();
        void detectFace(Mat frame);
        void adjustFacesSize();
        Rect &getBiggestFace();

        // Returns face closest to center
        Rect &getMainFace(Mat frame);

        const vector<Rect> &getFaces() const {
            return faces;
        }

        void setFrame(const Mat &frame) {
            Detector::frame = frame;
        }

        void setWorking(bool working) {
            Detector::working = working;
        }

        bool isWorking() const {
            return working;
        }

        void setFaces(const vector<Rect> &faces) {
            Detector::faces = faces;
        }
};


#endif //BPM_FACEDETECTOR_H
