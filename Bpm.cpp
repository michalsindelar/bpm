//
// Created by Michal on 08/02/16.
//

#include "Bpm.h"

// Constructor
Bpm::Bpm(int sourceMode, int maskMode, float beatVisibilityFactor) {
    this->sourceMode = sourceMode;
    this->maskMode = maskMode;
    this->beatVisibilityFactor = 0.8f;
    this->saveOutput = false;

    if (this->sourceMode == VIDEO_SOURCE_MODE) {
        // Open Video Camera
        this->input = VideoCapture((string) PROJECT_DIR + "/data/reference.mp4");
        if(!input.isOpened()) cout << "Unable to open Video File";
        this->fps = (int) round(this->input.get(CV_CAP_PROP_FPS));
    }

    else if (this->sourceMode == CAMERA_SOURCE_MODE) {
        // Open Video Camera
        this->input = VideoCapture(0);
        if(!input.isOpened()) cout << "Unable to open Video Camera";
        this->fps = FPS;
    }

    this->initialWorkerFlag = false;

    // Initialize middleware
    this->bpmWorker = AmplificationWorker();
    bpmWorker.setFps(fps);
}

int Bpm::run() {
    double i = 0;

    // Application window
    namedWindow( "App window", CV_WINDOW_AUTOSIZE);

    int ex = static_cast<int>(input.get(CV_CAP_PROP_FOURCC));

    if (saveOutput) {
        output.open((string) PROJECT_DIR+"/output/out.avi",CV_FOURCC('m', 'p', '4', 'v'),this->fps, Size(600,400),true);
    }

    for (int frame = 0; true; frame++) {

        // Measuring elapsed time
        // TODO: clock_gettime() -- this one!, (gettimeofday())
        clock_t begin = clock();

        // Grab video frame
        Mat in;
        input >> in; // type: CV_8UC3 (16)

        // Handle ending video
        if (!in.data) {
            if (this->sourceMode == VIDEO_SOURCE_MODE) {
                input.set(CV_CAP_PROP_POS_MSEC, 0);
                input >> in;
            } else if (this->sourceMode == CAMERA_SOURCE_MODE) {
                // TODO:
            }
        }

        if (frame < CAMERA_INIT) continue;

        // Resize captured frame
        in = resizeImage(in, RESIZED_FRAME_WIDTH);

        // TODO: Move outside loop
        Size frameSize(in.cols, in.rows);

        // Output
        Mat out = Mat(in.rows, in.cols, in.type());

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
            // Clear bpmWorker bpmVisualsubization array
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
            if (this->maskMode == FOURIER_MASK_MODE) {
                Mat visual = Mat::zeros(in.rows, in.cols, in.type());

                // As we crop mask in own thread while amplification
                // These steps are appli only if detected face positon has significantly changed
                Mat tmp = resizeImage(this->bpmVisualization.at(frame % BUFFER_FRAMES), tmpFace.width - 2*ERASED_BORDER_WIDTH);

                // Important range check
                Rect roi(face.x, face.y, tmp.cols, tmp.rows);
                controlFacePlacement(roi, frameSize);
                roi.x = roi.y = 0;

                // Crop in case mask would be outside frame
                tmp = tmp(roi);

                tmp.copyTo(visual(Rect(face.x + ERASED_BORDER_WIDTH, face.y + ERASED_BORDER_WIDTH, tmp.cols, tmp.rows)));
                out = in + this->beatVisibilityFactor * visual;
            }
            // AMPLIFICATION FAKE BEATING MODE
            else if (this->maskMode == FAKE_BEATING_MODE) {
                fakeBeating(out, i, 30, this->tmpFace);
            }
            putText(out, to_string(this->bpmWorker.getBpm()), Point(220, out.rows - 30), FONT_HERSHEY_SIMPLEX, 1.0,Scalar(200,200,200),2);

        } else {
            out = in.clone();
            putText(out, "Loading...", Point(220, out.rows - 30), FONT_HERSHEY_SIMPLEX, 1.0,Scalar(200,200,200),2);
        }

        if (saveOutput) {
            output.write(out);
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
        if (this->sourceMode == CAMERA_SOURCE_MODE) {
            double elapsedMus = double(end - begin) / CLOCKS_PER_SEC * 1000000;
            int extraWaitMus = (elapsedMus > (CLOCKS_PER_SEC / this->fps)) ? 0 : int(LOOP_WAIT_TIME_MUS - elapsedMus + 0.5);
            usleep(extraWaitMus);
        }
    }

    return 0;
}

void Bpm::updateFace(Rect face) {
    // After initial detection update only position (not size)
    if (!this->face.x) {
        this->face = this->tmpFace = face;
    } else {
        this->updateTmpFace(face, FACE_UPDATE_VARIATION);
    }
}

void Bpm::updateTmpFace(Rect face, float variation) {
    if (abs(this->tmpFace.x - face.x) > this->tmpFace.x * variation) {
        this->tmpFace.x = face.x;
    }
    if (abs(this->tmpFace.y - face.y) > this->tmpFace.y * variation) {
        this->tmpFace.y = face.y;
    }
    if (abs(this->tmpFace.width - face.width) > this->tmpFace.width * variation) {
        this->tmpFace.width = face.width;
    }
    if (abs(this->tmpFace.height - face.height) > this->tmpFace.height * variation) {
        this->tmpFace.height = face.height;
    }
}

void Bpm::mergeFaces() {
//    this->face = this->tmpFace;
}

bool Bpm::isFaceDetected() {
    return !!this->face.x;
}

bool Bpm::isBufferFull() {
    return videoBuffer.size() == BUFFER_FRAMES;
}
