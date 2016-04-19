//
// Created by Michal on 04/03/16.
//

#ifndef BPM_FACEDETECTOR_H
#define BPM_FACEDETECTOR_H


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "../config.h"
#include "imageOperation.h"

using namespace cv;
using namespace std;

class Detector {
    vector<Rect> faces;
    Rect biggestFace;
    Rect forehead;
    Mat frame;
    CascadeClassifier faceCascade;
    bool working;

    float faceHeightScale; // Increase height
    float faceYOffset; // Move face up

    public:
        Detector();
        // Non static function for main loop purposes
        void detectFace(Mat frame);
        void adjustFaceSize();
        Rect &determineBiggestFace();

        static int detectEyes(Mat face, vector<Rect>& eyes) {
            vector<Rect> eyesAll;

            CascadeClassifier eyesCascade;
            eyesCascade.load((string) DATA_DIR+"/haarcascades/haarcascades_eye.xml");
            eyesCascade.detectMultiScale(face, eyesAll, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, cv::Size(20,20));

            // We need 2 eyes
            if (eyesAll.size() < 2) {
                return 0;
            }

            // Return 2 eyes with closest y diff
            int eyeFirstIndex = 0;
            int eyeSecondIndex = 1;
            float yDiff = abs(eyesAll[eyeFirstIndex].y - eyesAll[eyeSecondIndex].y);

            for (int i = 0; i < eyesAll.size(); i++) {
                for (int j = 0; j < eyesAll.size() / 2; j++) {
                    if (i == j) continue;
                    if (abs(eyesAll[i].y - eyesAll[j].y) < yDiff) {
                        eyeFirstIndex = i;
                        eyeSecondIndex = j;
                        yDiff = abs(eyesAll[i].y - eyesAll[j].y);
                    }
                }
            }

            // Face too rotated or incorrect detection
            if (yDiff > face.rows * 0.3) return 0;

            // Push to output
            eyes.push_back(eyesAll[eyeFirstIndex]);
            eyes.push_back(eyesAll[eyeSecondIndex]);

            eyesAll.clear();

            return 1;
        }

        static int detectForeheadFromFaceViaEyesDetection(Mat face, Rect &roi) {
            Mat upperFace = face(Rect(0, 0, face.cols, face.rows / 2));

            vector<Rect> eyes;
            // Cannot set forehead via eyes detection
            if (!Detector::detectEyes(upperFace, eyes)) {
                return 0;
            }

            Rect eyeL, eyeR;
            eyeL = (eyes[0].x < eyes[1].x) ? eyes[0] : eyes[1];
            eyeR = (eyes[1].x >= eyes[0].x) ? eyes[1] : eyes[0];

            Point2f center((eyeL.x + eyeL.width) + (eyeR.x - eyeL.x - eyeL.width) / 2.0f, (int) round(eyeL.y - face.rows * 0.11f));
            Size size((int) round(face.cols * 0.55), (int) round(face.rows * 0.16));

            Rect tmp = Rect((int) round(center.x - size.width/2.0f), (int) round(center.y - size.height/2.0f), size.width, size.height);

            if (!Detector::shouldAcceptForehead(Rect(0,0, face.cols, face.rows), tmp)) {
                return 0;
            }

            handleRoiPlacement(tmp, face.size());
            roi = tmp;

            return 1;
        }

        // Accept only fine detected forehead in center
        static bool shouldAcceptForehead(Rect face, Rect forehead, float diff = 1.5*DETECTOR_UPDATE_VARIATION) {
            double foreheadCenter = forehead.x + (forehead.width / 2.0f);
            double faceCenter = face.width / 2.0f;
            return (abs(foreheadCenter - faceCenter) < (diff * face.width));
        };

        static Rect defaultForehead(Mat face) {
            return Rect(
                (int) round(face.cols * 0.25f),
                (int) round(face.rows * 0.05f),
                (int) round(face.cols * 0.5f),
                (int) round(face.rows * 0.15f)
            );
        }

        const Rect &getBiggestFace() const {
            return biggestFace;
        }

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

        void detectForehead(Mat face);

        bool isDetected();

        const Rect &getForehead() const {
            return forehead;
        }
};


#endif //BPM_FACEDETECTOR_H
