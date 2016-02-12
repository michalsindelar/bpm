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

#define BUFFER_FRAMES 40
#define FRAME_RATE 15
#define CAMERA_INIT 10


using namespace cv;
using namespace std;

class Bpm {
    private:
        bool initialWorkerFlag = false;
        int currBpm;

        VideoCapture cam;
        // Deque for storing captured frames
        deque<Mat> videoBuffer;
        // Processed mask of blood flow
        vector<Mat> bpmVisualization;

        // OS window
        Mat window;

        // Worker for computing
        AmplificationWorker bpmWorker;

    public:
        Bpm();
        int run();



};


#endif //BPM_BPM_H
