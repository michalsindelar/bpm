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

#include "./imageOperation.h"
#include "./amplify.h"
#include "./AmplificationWorker.h"
#include "FaceDetectorWorker.h"

#include "skinDetect.h"

#define BUFFER_FRAMES 40
#define FRAME_RATE 15
#define CAMERA_INIT 10
#define RESIZED_FRAME_WIDTH 600


using namespace cv;
using namespace std;

class Bpm {
    private:
        bool initialWorkerFlag = false;
        int currBpm;
        float beatVisibilityFactor = 1;
        Rect face;

        VideoCapture cam;
        // Deque for storing captured frames
        vector<Mat> videoBuffer;
        // Orig video buffer
        vector<Mat> origVideoBuffer;
        // Processed mask of blood flow
        vector<Mat> bpmVisualization;

        // OS window
        Mat window;

        // Detected face
        vector<Rect> faces;

        // Worker for computing
        AmplificationWorker bpmWorker;

        // Worker for computing
        FaceDetectorWorker faceDetector;

    public:
        Bpm();
        int run();
        void updateFace(Rect face);
        bool isFaceDetected();
        bool isBufferFull();


};


#endif //BPM_BPM_H
