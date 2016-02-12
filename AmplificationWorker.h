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

using namespace cv;
using namespace std;

class AmplificationWorker {
    int bpm;
    bool ready;
    bool initialFlag;

    vector<Mat> videoBuffer;
    vector<Mat> visualization;

    public:
        AmplificationWorker();
        void compute(deque<Mat> videoBuffer);
        void setVideoBuffer(deque<Mat> videoBuffer);
        void clearVisualization();

        bool isReady() {
            return this->ready;
        }
        bool getInitialFlag() {
            return initialFlag;
        }

        const vector<Mat> &getVisualization() const {
            return this->visualization;
        }

        void setReady(bool ready) {
            AmplificationWorker::ready = ready;
        }
};


#endif //BPM_AMPLIFICATIONWORKER_H
