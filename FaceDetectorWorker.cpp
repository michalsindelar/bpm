//
// Created by Michal on 04/03/16.
//

#include "FaceDetectorWorker.h"

FaceDetectorWorker::FaceDetectorWorker() {
    this->faceHeightScale = 1.3f;
    this->faceYOffset = 0.2f;

    this->working = false;

    this->faceCascade = CascadeClassifier();
    this->faceCascade.load((string) PROJECT_DIR+"/haarcascade_frontalface_alt.xml");
}

void FaceDetectorWorker::detectFace(Mat frame) {
    if (this->working) return;

    // Start work!
    this->working = true;
    this->frame = frame;

    // Create bw mat
    cvtColor(frame, frame, CV_BGR2GRAY );

    // Detect faces
    faceCascade.detectMultiScale( frame, faces, 1.3, 2, 0|CV_HAAR_SCALE_IMAGE);

    // Check whether detection successful
    if (!this->faces.size()) {
        this->working = false;
        return;
    }

    this->adjustFacesSize();
    this->working = false;
}

// Increase height of detected face
void FaceDetectorWorker::adjustFacesSize() {
    for (int i = 0; i < 1; i++) {
        this->faces[i].y -= this->faces[i].height*faceYOffset;
        this->faces[i].height *= faceHeightScale;

        // Control range
        this->faces[i].y = (this->faces[i].y < 0) ? 0 : this->faces[i].y;
        this->faces[i].height = ((this->faces[i].y + this->faces[i].height) > frame.rows) ?
            this->faces[i].height - this->faces[i].y :
            this->faces[i].height;
    }
}
