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
        // CAMERA_SOURCE_MODE / VIDEO_SOURCE_MODE
        int sourceMode;

        // FOURIER_MASK_MODE / FAKE_BEATING_MODE
        int maskMode;
        bool initialWorkerFlag = false;
        int currBpm;
        float beatVisibilityFactor;
        int fps;
        Rect face;
        Rect tmpFace;

        VideoCapture input;
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
        Bpm(int mode, int maskMode, float beatVisibilityFactor);
        int run();
        void updateFace(Rect face);
        void updateTmpFace(Rect face, float variation);
        void mergeFaces();
        bool isFaceDetected();
        bool isBufferFull();

        void setMode(int mode) {
            Bpm::maskMode = mode;
        }
};


#endif //BPM_BPM_H
