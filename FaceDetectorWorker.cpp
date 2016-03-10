//
// Created by Michal on 04/03/16.
//

#include "FaceDetectorWorker.h"

FaceDetectorWorker::FaceDetectorWorker() {
    this->working = false;

    this->faceCascade = CascadeClassifier();
    // TODO: Relative path!!
    string path = PROJECT_DIR;
    this->faceCascade.load(path+"/haarcascade_frontalface_alt.xml");
}

void FaceDetectorWorker::detectFace(Mat frame) {
    if (this->working) return;

    // Start work!
    this->working = true;
    this->frame = frame;

    // Create bw mat
    cvtColor(frame, frame, CV_BGR2GRAY );

    // Detect faces
    try {
        faceCascade.detectMultiScale( frame, faces, 1.3, 2, 0|CV_HAAR_SCALE_IMAGE);
    } catch (Exception & e) {
        this->working = false;
        return;
    }

    // Increase height of detected face
    // TODO: If someday support for mor faces
    for (int i = 0; i < 1; i++) {
        this->faces[i].y -= this->faces[i].height*faceYOffset;
        this->faces[i].height *= faceHeightScale;
    }

    this->working = false;
}
