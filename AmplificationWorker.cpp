//
// Created by Michal on 08/02/16.
//

#include "AmplificationWorker.h"

AmplificationWorker::AmplificationWorker() {
    // Default fps
    this->fps = FPS;

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
    int currBpm = 0;

    // CRATE AMPLIFICATION CLASS
    //


    // Resizing must be computed according to face SIZE !!
    BpmVideoProcessor bpmVideoProcessor = BpmVideoProcessor(videoBuffer, FL, FH, 5, fps, BUFFER_FRAMES);
    bpmVideoProcessor.compute();
    this->visualization = bpmVideoProcessor.getOut();
    currBpm = bpmVideoProcessor.getBpm();

    // Prevent big changes
    this->bpm = this->bpm ? (this->bpm + currBpm) / 2 : currBpm;

    resizeCropVideo(this->visualization, this->videoBuffer[0].cols);
//    resizeCropVideo();


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