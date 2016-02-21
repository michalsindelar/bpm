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

    for (int frame = 0; true; frame++) {
        // Grab video frame
        Mat in;
        cam >> in; // type: CV_8U

        if (frame < CAMERA_INIT) continue;

        // Resize captured frame
        in = resizeImage(in, 1000);

        // Output
        Mat out = in.clone();

        // Detect face
        if (this->faces.size() == 0) {
            this->faces = detectFace(out);
        }

        // TODO: pop only cropped frame with face

        // Keep maximum BUFFER_FRAMES size
        if (videoBuffer.size() == BUFFER_FRAMES) {
            // Erase first frame
            videoBuffer.erase(videoBuffer.begin());
        }
        videoBuffer.push_back(in.clone());

        // Update bpm once bpmWorker ready
        if (!this->bpmWorker.isWorking() && this->bpmWorker.getInitialFlag()) {
            this->bpmVisualization.clear(); // Clear current bpmVisualization array
            this->bpmWorker.getVisualization().swap(this->bpmVisualization); // Copy to loop bpmVisualization vid
            this->bpmWorker.clearVisualization(); // Clear bpmWorker bpmVisualization array
        }

        // Start computing when buffer filled
        // TODO: REMOVE DEV ONLY
        if (frame > CAMERA_INIT + BUFFER_FRAMES && videoBuffer.size() == BUFFER_FRAMES && !bpmWorker.isWorking()) {
            boost::function<void()> th_bpm = boost::bind(&AmplificationWorker::compute, &bpmWorker, videoBuffer);
            boost::thread th(th_bpm);
        }

        // Show bpmVisualization video after initialization compute
        if (this->bpmWorker.getInitialFlag()) {
            imshow("FILTERED", resizeImage(this->bpmVisualization.at(frame % BUFFER_FRAMES), 800));
        }

        // Merge original + adjusted
        hconcat(in, out, window);

        // Put the image onto a screen
        namedWindow( "Display Image", CV_WINDOW_AUTOSIZE );
        imshow( "Display Image", window);

        // Free
        in.release();
        out.release();

        //press anything within the poped-up window to close this program
        if (waitKey(10) >= 0) break;

        i += .2;
    }

    return 0;
}