//
// Created by Michal on 08/02/16.
//

#include "AmplificationWorker.h"

AmplificationWorker::AmplificationWorker() {
    this->initialFlag = false;
    this->working = false;
};

void AmplificationWorker::compute(vector<Mat> videoBuffer){
    if (this->working) return;

    // Start work!
    this->working = true;
    cout << "Computing bpm";

    // At first fill class buffer with copies!
    this->setVideoBuffer(videoBuffer);

    int ret = 100;

    // TODO: Remove -DEV ONLY
    int fps = 30;

    // Amplify
    amplifySpatial(this->videoBuffer, this->visualization, 50, 50/60, 70/60, 30, int(videoBuffer.size()), 3);
    this->videoBuffer.clear();

    this->bpm = ret;
    this->working = false;
    this->initialFlag = true;
    cout << "Computed bpm in class";
};


void AmplificationWorker::setVideoBuffer(vector<Mat> videoBuffer) {

    this->videoBuffer.swap(videoBuffer);
};

void AmplificationWorker::clearVisualization() {
    this->visualization.clear();
}