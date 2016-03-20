//
// Created by Michal on 08/02/16.
//

#include "Bpm.h"

// Constructor
Bpm::Bpm() {
    // Open Video Camera
    this->cam = VideoCapture((string) PROJECT_DIR+"/data/reference.mp4");

    if(!cam.isOpened()) cout << "Unable to open Video Camera";
    int frameRate = this->cam.get(CV_CAP_PROP_FPS);

    this->initialWorkerFlag = false;
    this->bpmWorker = AmplificationWorker();
}

int Bpm::run() {
    double i = 0;

    // Application window
    namedWindow( "App window", CV_WINDOW_AUTOSIZE);

    for (int frame = 0; true; frame++) {

        // Measuring elapsed time
        // TODO: clock_gettime() -- this one!, (gettimeofday())
        clock_t begin = clock();

        // Grab video frame
        Mat in;
        cam >> in; // type: CV_8UC3 (16)

        if (frame < CAMERA_INIT) continue;

        // Resize captured frame
        in = resizeImage(in, RESIZED_FRAME_WIDTH);

        // TODO: Move outside loop
        Size frameSize(in.cols, in.rows);

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
            face.width = ((face.x + face.width) > in.cols) ? face.width - (face.x + face.width - in.cols) : face.width;
            face.height = ((face.y + face.height) > in.rows) ? face.height - (face.y + face.height - in.rows) : face.height;

            Rect roi(face.x, face.y, face.width, face.height);
            controlFacePlacement(roi, frameSize);

            Mat croppedToFace = in(roi).clone();
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
            boost::thread workerThread(&AmplificationWorker::compute, &bpmWorker, videoBuffer);
            mergeFaces();
        }

        // Show bpmVisualization video after initialization compute
        // TODO: Check if this is performance ok
        if (this->bpmWorker.getInitialFlag()) {
            // AMPLIFICATION FOURIER MODE
            if (this->mode == FOURIER_MASK_MODE) {
                Mat visual = Mat::zeros(in.rows, in.cols, in.type());

                Mat tmp = resizeImage(this->bpmVisualization.at(frame % BUFFER_FRAMES), tmpFace.width);

//                Rect roi(face.x, face.y, tmp.cols, tmp.rows);
//                controlFacePlacement(roi, frameSize);
//                roi.x = roi.y = 0;
//
//                // if face would be outside frame crop, else keep same
//                Mat controlledTmp = tmp(roi);

                tmp.copyTo(visual(Rect(face.x, face.y, tmp.cols, tmp.rows)));
                out = out + this->beatVisibilityFactor*visual;
            }
            // AMPLIFICATION FAKE BEATING MODE
            else if (this->mode == FAKE_BEATING_MODE) {
                fakeBeating(out, i, 30, this->tmpFace);
            }
            putText(out, to_string(this->bpmWorker.getBpm()), Point(220, out.rows - 30), FONT_HERSHEY_SIMPLEX, 1.0,Scalar(200,200,200),2);

        } else {
            putText(out, "Loading...", Point(220, out.rows - 30), FONT_HERSHEY_SIMPLEX, 1.0,Scalar(200,200,200),2);
        }

        // Merge original + adjusted
        hconcat(out, in, window);

        // Put the image onto a screen
        imshow( "App window", window);

        // Update index
        i += .2;

        // Handling frame rate & time for closing window
        if (waitKey(1) >= 0) break;

        // Stop measuring loop time
        clock_t end = clock();

        // TODO: Check if working
//        double elapsedMus = double(end - begin) / CLOCKS_PER_SEC * 1000000;
//        int extraWaitMus = (elapsedMus > LOOP_WAIT_TIME_MUS) ? 0 : int(LOOP_WAIT_TIME_MUS - elapsedMus + 0.5);
//        usleep(extraWaitMus);
    }

    return 0;
}

void Bpm::updateFace(Rect face) {
    // After initial detection update only position (not size)
    if (!this->face.x) {
        this->face = this->tmpFace = face;
    }
}


void Bpm::mergeFaces() {
    this->face = this->tmpFace;
}

bool Bpm::isFaceDetected() {
    return !!this->face.x;
}

bool Bpm::isBufferFull() {
    return videoBuffer.size() == BUFFER_FRAMES;
}
