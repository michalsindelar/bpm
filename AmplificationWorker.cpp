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

    // Amplify
    amplifySpatial(this->videoBuffer, this->visualization, this->bpm, 50, 50/60, 70/60, 30, int(videoBuffer.size()), 4);
    this->videoBuffer.clear();

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