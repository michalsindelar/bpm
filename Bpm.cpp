//
// Created by Michal on 08/02/16.
//

#include "Bpm.h"

// Constructor
Bpm::Bpm() {

    // Open Video Camera
    // TODO Merge!
    try {
        this->cam = VideoCapture(0);
    } catch(int e) {
        cout << "Unable to open Video Camera";
    };

    if(!cam.isOpened())
        cout << "Unable to open Video Camera";

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
        in = resizeImage(in, 700);

        // Output
        Mat out = in.clone();

        // Keep maximum BUFFER_FRAMES size
        if (videoBuffer.size() == BUFFER_FRAMES) {
            videoBuffer.pop_front();
        }
        videoBuffer.push_back(in.clone());

        // Update bpm once bpmWorker ready
        // Clear current bpmVisualization array
        // Copy to loop bpmVisualization vid
        // Clear bpmWorker bpmVisualization array
        if (this->bpmWorker.isReady()) {
            this->bpmWorker.setReady(false);
            this->bpmVisualization.clear();

            for (Mat img : this->bpmWorker.getVisualization()) {
                this->bpmVisualization.push_back(img.clone());
            }

            this->bpmWorker.clearVisualization();
        }

        // Start computing when buffer filled
        // TODO: REMOVE DEV ONLY
        if ((frame + 1) % BUFFER_FRAMES == 0 && frame > CAMERA_INIT + BUFFER_FRAMES) {
            boost::function<void()> th_bpm = boost::bind(&AmplificationWorker::compute, &bpmWorker, videoBuffer);
            boost::thread th(th_bpm);
        }

        // Show bpmVisualization video after initialization compute
        if (this->bpmWorker.getInitialFlag()) {
            imshow("FILTERED", resizeImage(this->bpmVisualization.at(frame % BUFFER_FRAMES), 1000));
            if (waitKey(10) >= 0) break;
        }

        // Adjustemnt of output
        //        fakeBeating(out, i, FRAME_RATE/10);

        // Merge original + adjusted
        hconcat(in, out, window);

        //put the image onto a screen
        imshow("video:", window);

        // Free
        in.release();
        out.release();

        //press anything within the poped-up window to close this program
        if (waitKey(10) >= 0) break;

        i += .2;
    }


}