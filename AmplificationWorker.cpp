//
// Created by Michal on 08/02/16.
//

#include "AmplificationWorker.h"

AmplificationWorker::AmplificationWorker() {
    // Default fps
    this->fps = FRAME_RATE;

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

    // CRATE AMPLIFICATION CLASS
    //


    // Resizing must be computed according to face SIZE !!
    BpmVideoProcessor bpmVideoProcessor = BpmVideoProcessor(videoBuffer, FL, FH, 5, fps, BUFFER_FRAMES);
    bpmVideoProcessor.compute();
    this->visualization = bpmVideoProcessor.getOut();

//    amplifySpatial(this->videoBuffer, this->visualization, bpm, 50, 50/60, 120/60, int(videoBuffer.size()),5);
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