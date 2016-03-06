//
// Created by Michal on 08/02/16.
//

#include "Bpm.h"

// Constructor
Bpm::Bpm() {
    // Open Video Camera
    this->cam = VideoCapture(0);

    if(!cam.isOpened()) cout << "Unable to open Video Camera";

    this->initialWorkerFlag = false;
    this->bpmWorker = AmplificationWorker();
}

int Bpm::run() {
    double i = 0;

    // Application window
    namedWindow( "App window", CV_WINDOW_AUTOSIZE);

    for (int frame = 0; true; frame++) {
        // Grab video frame
        Mat in;
        cam >> in; // type: CV_8U

        if (frame < CAMERA_INIT) continue;

        // Resize captured frame
        in = resizeImage(in, RESIZED_FRAME_WIDTH);

        // Output
        Mat out = in.clone();

        // Detect face in own thread
        if (!faceDetector.isWorking()) {
            boost::thread workerThread(&FaceDetectorWorker::detectFace, &faceDetector, in);
        }

        if (faceDetector.getFaces().size()) {
            // TODO: function get biggest face
            this->updateFace(faceDetector.getFaces()[0]);
        }

        // Keep maximum BUFFER_FRAMES size
        if (this->isBufferFull()) {
            // Erase first frame
            videoBuffer.erase(videoBuffer.begin());
        }

        // Start cropping frames to face only after init
        // TODO: Can't run without face!
        // Or can be choosen center of image
        if (frame > CAMERA_INIT && this->isFaceDetected()) {
            Mat croppedToFace = in(Rect(face.x, face.y, face.width, face.height)).clone();
            videoBuffer.push_back(croppedToFace);
        }

        // Update bpm once bpmWorker ready
        if (!this->bpmWorker.isWorking() && this->bpmWorker.getInitialFlag() && this->isBufferFull() ) {
            // Clear current bpmVisualization array
            this->bpmVisualization.clear();
            // Copy to loop bpmVisualization vid
            this->bpmWorker.getVisualization().swap(this->bpmVisualization);
            // Clear bpmWorker bpmVisualization array
            this->bpmWorker.clearVisualization();
        }

        // Start computing when buffer filled
        if (frame > CAMERA_INIT + BUFFER_FRAMES && this->isBufferFull() && !bpmWorker.isWorking()) {
            boost::function<void()> th_bpm = boost::bind(&AmplificationWorker::compute, &bpmWorker, videoBuffer);
            boost::thread th(th_bpm);
        }

        // Show bpmVisualization video after initialization compute
        // TODO: Check if this is performance ok
        if (this->bpmWorker.getInitialFlag()) {
            Mat visual = Mat::zeros(in.rows, in.cols, in.type());

            Mat tmp = resizeImage(this->bpmVisualization.at(frame % BUFFER_FRAMES), face.width);
            tmp.copyTo(visual(cv::Rect(face.x,face.y, tmp.cols, tmp.rows)));
            out = out + this->beatVisibilityFactor*visual;
        }

        // Merge original + adjusted
        hconcat(out, in, window);

        // Put the image onto a screen
        imshow( "App window", window);

        // Free
        in.release();
        out.release();

        //press anything within the poped-up window to close this program
        if (waitKey(1) >= 0) break;

        i += .2;
    }

    return 0;
}

void Bpm::updateFace(Rect face) {
    // After initial detection update only position (not size)
    if (!this->face.x) {
        this->face = face;
    } else {
        this->face.x = face.x;
        this->face.y = face.y;
    }
}

bool Bpm::isFaceDetected() {
    return !!this->face.x;
}

bool Bpm::isBufferFull() {
    return videoBuffer.size() == BUFFER_FRAMES;
}
