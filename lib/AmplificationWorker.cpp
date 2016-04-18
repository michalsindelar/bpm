//
// Created by Michal on 08/02/16.
//

#include "AmplificationWorker.h"

AmplificationWorker::AmplificationWorker() {
    // Default fps
    this->fps = FPS;

    this->initialFlag = false;
    this->working = false;
    this->bpm = 0;
    this->bpmDetected = false;
};

void AmplificationWorker::compute(vector<Mat> videoBuffer){
    // Just to be extra sure worker is ready
    if (this->working) return;

    // Start work!
    this->working = true;
    this->bpmDetected = false;
    cout << "Computing bpm";

    // At first fill class buffer with copies!
    this->setVideoBuffer(videoBuffer);

    // Level for masking
    int level = 6;

    // Resizing should be set according to face SIZE !!
    BpmVideoProcessor bpmVideoProcessor = BpmVideoProcessor(videoBuffer, CUTOFF_FL, CUTOFF_FH, level, fps, bufferFrames, faceRoi);

    // Compute bpm at first
    bpmVideoProcessor.computeBpm();
    updateBpm(bpmVideoProcessor.getBpm());

    // TODO: Here will be video split into thread parts
    // Each part has own bpmVideoProcessor class
    // Compute bpm amplification mask

    vector<boost::thread *> z;
    int threadPartFrames = min(FRAMES_FOR_VISUALIZATION, (int) videoBuffer.size());
    int parts = (int) ceil(videoBuffer.size() / threadPartFrames);

    for (int i = 0; i < parts; i++) {
        // Magic here
    }
    bpmVideoProcessor.computeAmplifiedMask();
    this->visualization = bpmVideoProcessor.getOut();


    // Resize and crop video to similar size as face in gui
    resizeCropVideo(this->visualization, this->resizedFace.width);

    // Cleaning
    this->videoBuffer.clear();
    this->working = false;
    this->initialFlag = true;

};

void AmplificationWorker::updateBpm(int bpm) {
    // Prevent big changes
    this->bpm = this->bpm ? (this->bpm + bpm) / 2 : bpm;
    this->bpmDetected = true;
}
