//
// Created by Michal on 08/02/16.
//

#ifndef BPM_AMPLIFICATIONWORKER_H
#define BPM_AMPLIFICATIONWORKER_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <highgui.h>

#include "./amplify.h"

// Configuration
#include "./config.h"

using namespace cv;
using namespace std;

class AmplificationWorker {
    int bpm;
    bool working;
    bool initialFlag;

    vector<Mat> videoBuffer;
    vector<Mat> visualization;

    Rect face;
    Size origVideoSize;

    public:
        AmplificationWorker();
        void compute(vector<Mat> videoBuffer);
        void setVideoBuffer(vector<Mat> videoBuffer);
        void clearVisualization();

        bool isWorking() {
            return this->working;
        }
        bool getInitialFlag() {
            return initialFlag;
        }

        vector<Mat> & getVisualization() {
            return this->visualization;
        }

        void setWorking(bool working) {
            AmplificationWorker::working = working;
        }

        void setFace(const Rect &face) {
            AmplificationWorker::face = face;
        }
};


#endif //BPM_AMPLIFICATIONWORKER_H
