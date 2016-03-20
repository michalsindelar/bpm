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
    int bpm = 0;
    amplifySpatial(this->videoBuffer, this->visualization, bpm, 50, 50/60, 120/60, int(videoBuffer.size()), 2);
    resizeCropVideo(this->visualization, this->videoBuffer[0].cols);

    // Prevent big changes
    this->bpm = this->bpm ? (this->bpm + bpm)/2 : bpm;

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