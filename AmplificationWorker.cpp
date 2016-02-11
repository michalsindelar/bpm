//
// Created by Michal on 08/02/16.
//

#include "AmplificationWorker.h"

AmplificationWorker::AmplificationWorker() {
    this->initialFlag = false;
    this->ready = false;
};

void AmplificationWorker::compute(deque<Mat> videoBuffer){
    // At first fill class buffer with copies!
    this->setVideoBuffer(videoBuffer);

    cout << "Computing bpm";
    int ret = 100;

    // TODO: Remove -DEV ONLY
    int fps = 30;

    // Amplify
    amplifySpatial(this->videoBuffer, this->visualization, 50, 50/60, 180/60, 30, int(videoBuffer.size()), 4);
    this->videoBuffer.clear();

    bpm = ret;
    ready = true;
    initialFlag = true;
    cout << "Computed bpm in class";
};

void AmplificationWorker::setVideoBuffer(deque<Mat> videoBuffer) {
    int i = 0;
    for (Mat img : videoBuffer) {
        this->videoBuffer.push_back(img.clone());
        i++;
    }
};

void AmplificationWorker::clearVisualization() {
    this->visualization.clear();
}