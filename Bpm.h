//
// Created by Michal on 08/02/16.
//

#ifndef BPM_BPM_H
#define BPM_BPM_H

#include <iostream>
#include <boost/thread.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <highgui.h>
#include <opencv2/imgproc/imgproc_c.h>

// Classes
#include "./AmplificationWorker.h"
#include "./FaceDetectorWorker.h"

// Functions
#include "./imageOperation.h"
#include "./amplify.h"

// Configuration
#include "./config.h"

#include "skinDetect.h"


using namespace cv;
using namespace std;

class Bpm {
    private:
        bool initialWorkerFlag = false;
        int currBpm;
        int mode = FOURIER_MASK_MODE;
        float beatVisibilityFactor = 1;
        Rect face;
        Rect tmpFace;

        VideoCapture cam;
        // Deque for storing captured frames
        vector<Mat> videoBuffer;
        // Orig video buffer
        vector<Mat> origVideoBuffer;
        // Processed mask of blood flow
        vector<Mat> bpmVisualization;

        // OS window
        Mat window;

        // Worker for computing
        AmplificationWorker bpmWorker;

        // Worker for computing
        FaceDetectorWorker faceDetector;

    public:
        Bpm();
        int run();
        void updateFace(Rect face);
        void mergeFaces();
        bool isFaceDetected();
        bool isBufferFull();

        void setMode(int mode) {
            Bpm::mode = mode;
        }
};


#endif //BPM_BPM_H
