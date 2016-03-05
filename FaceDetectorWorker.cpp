//
// Created by Michal on 04/03/16.
//

#include "FaceDetectorWorker.h"

FaceDetectorWorker::FaceDetectorWorker() {
    this->working = false;

    this->faceCascade = CascadeClassifier();
    // TODO: Relative path!!
    this->faceCascade.load("/Users/michal/Dev/bpm/haarcascade_frontalface_alt.xml");
}

void FaceDetectorWorker::detectFace(Mat frame) {
    if (this->working) return;

    // Start work!
    this->working = true;
    this->frame = frame;

    // Create bw mat
    cvtColor(frame, frame, CV_BGR2GRAY );

    // Detect faces
    faceCascade.detectMultiScale( frame, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(80, 80) );

    this->working = false;
}
