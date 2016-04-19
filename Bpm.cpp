//
// Created by Michal on 08/02/16.
//

#include "Bpm.h"

// Constructor



int Bpm::init(int sourceMode, int maskMode) {

    this->sourceMode = sourceMode;
    this->maskMode = maskMode;
    this->beatVisibilityFactor = 0.4f;

    this->OSWindowName = "Bpm";
    this->workerIteration = 0;


    // CAMERA SOURCE MODE
    if (this->sourceMode == CAMERA_SOURCE_MODE) {
        // Open Video Camera
        this->input = VideoCapture(0);
        if (!input.isOpened()) cout << "Unable to open Video Camera";
        this->fps = FPS;
        this->bufferFrames = BUFFER_FRAMES;
    }

    // VIDEO SOURCE MODE
    else if (this->sourceMode == VIDEO_REAL_SOURCE_MODE || this->sourceMode == VIDEO_STATIC_SOURCE_MODE) {
        // Open Video Camera
        this->input = VideoCapture(this->videoFilePath);
        if (!input.isOpened()) cout << "Unable to open Video File";
        this->fps = (int) round(this->input.get(CV_CAP_PROP_FPS));

        if (this->sourceMode == VIDEO_REAL_SOURCE_MODE) {
            this->bufferFrames = BUFFER_FRAMES;
        }
        else if (this->sourceMode == VIDEO_STATIC_SOURCE_MODE) {
            // TODO: All frames - will be devided in class to threads
//            this->bufferFrames = BUFFER_FRAMES;
        }

    }

    // COMPUTE RESIZED FRAME SIZE
    this->frameSize = getResizedSize(
            Size(this->input.get(CV_CAP_PROP_FRAME_WIDTH), this->input.get(CV_CAP_PROP_FRAME_HEIGHT)),
            RESIZED_FRAME_WIDTH);

    // INITALIZE MIDDLEWARE
    // TODO: Rename to middleware
    this->bpmWorker = AmplificationWorker();
    this->bpmWorker.setFps(fps);
    this->bpmWorker.setBufferFrames(bufferFrames);

    // TODO: Will be known from modes selector window gui
    // TODO: Function
    this->saveOutput = false;
    if (saveOutput) {
        output.open((string) PROJECT_DIR + "/output/out.avi", CV_FOURCC('m', 'p', '4', 'v'), this->fps, Size(600, 400),
                    true);
    }

    this->measuringIteration = 20;
    this->workerIteration = 0;

    if (false) {
        ofstream dataFile;
        dataFile.open((string) DATA_DIR + "/measure_72.txt", ios::out);
        printIterationHead(dataFile);
        dataFile.close();
    }

    // State
    this->state = DETECTING;

    // State notes
    fillLoadingNotes();

    // State bar with white bg
    this->stateBar = Mat((int) round(frameSize.height * 0.1), 2*frameSize.width, CV_8UC3);
}

int Bpm::run() {
    // Application window
    namedWindow( this->OSWindowName, CV_WINDOW_AUTOSIZE);

    switch (this->sourceMode) {
        case VIDEO_REAL_SOURCE_MODE:
            return runRealVideoMode();
        case VIDEO_STATIC_SOURCE_MODE:
            return runStaticVideoMode();
        case CAMERA_SOURCE_MODE:
            return runRealVideoMode();
        default:
            return runRealVideoMode();
    }

}

int Bpm::runRealVideoMode() {

    for (int index = 0; true; index++) {

        // Exit measuring
        if (false && (workerIteration > this->measuringIteration)) {
            break;
        };

        // Grab video frame
        Mat in;
        input >> in; // type: CV_8UC3 (16)

        if (index < CAMERA_INIT) continue;

        // Reset video when video ends
        if (!in.data) {
            input.set(CV_CAP_PROP_POS_MSEC, 0);
            input >> in;
        }

        // Keep maximum BUFFER_FRAMES size
        if (this->isBufferFull()) {
            videoBuffer.erase(videoBuffer.begin());
        }

        // Check full face detector
        handleDetector(in, FULL_FACE);

        // Start cropping frames to face after init & face detected
        pushInputToBuffer(in, index);

        // Resize captured frame
        in = resizeImage(in, RESIZED_FRAME_WIDTH);

        // Check resized face detector
        handleDetector(in, RESIZED_FACE);
        handleDetector(in, FOREHEAD);

        // Output
        Mat out = Mat(in.rows, in.cols, in.type());

        // Control state -> update / first compute / nothing
        controlMiddleWare(index);

        // Show bpmVisualization video after initialization compute
        // TODO: Check if this is performance ok
        visualize(in, out, index);

        if (false) {
            output.write(out);
        }

        renderMainWindow(in, out);

        // Put the image onto a screen
        imshow(this->OSWindowName, window);

        // Handling frame rate & time for closing window
        if (waitKey(1) >= 0) break;
    }

    return 0;
}

int Bpm::runStaticVideoMode() {

    // Fill buffer from whole video
    Mat in;

    // We need to have both faces detected
    while(!isFaceDetected(this->fullFace) || !isFaceDetected(this->fullFace)) {
        input >> in;

        // Check full face detector
        handleDetector(in, FULL_FACE);

        // Resize captured frame
        in = resizeImage(in, RESIZED_FRAME_WIDTH);

        // Check resized face detector
        handleDetector(in, RESIZED_FACE);
    }

    // Reset video
    input.set(CV_CAP_PROP_POS_MSEC, 0);

    // Buffer video frames
    // TODO: ffmpeg || libancv library for faster buffering?
    int bufferFrames = 0;
    while(true) {
        input >> in;

        // Check whether frame still exists
        if (!in.cols || !in.data) {
            break;
        }

        bufferFrames++;
        pushInputToBuffer(in.clone());
        waitKey(1);
    }

    // Update buffer frames
    this->bufferFrames = bufferFrames;
    bpmWorker.setBufferFrames(bufferFrames);

    // Compute here thread here has no reason
    compute(false);
    this->bpmVisualization = this->bpmWorker.getVisualization();
    this->bpmWorker.clearVisualization();

    // Set video to start
    input.set(CV_CAP_PROP_POS_MSEC, 0);

    for (int frame = 0; true; frame++) {

        // Grab video frame
        input >> in; // type: CV_8UC3 (16)

        // Reset video
        if (!in.data) {
            input.set(CV_CAP_PROP_POS_MSEC, 0);
            input >> in;
        }

        // Resize captured frame!
        in = resizeImage(in, RESIZED_FRAME_WIDTH);

        // Output
        Mat out = Mat(in.rows, in.cols, in.type());

        visualizeAmplified(in, out, frame);

        if (saveOutput) {
            output.write(out);
        }

        // Merge original + adjusted
        hconcat(out, in, window);

        // Put the image onto a screen
        imshow(this->OSWindowName, window);

        // Handling frame rate & time for closing window
        if (waitKey(1) >= 0) break;
    }

    return 0;
}

int Bpm::runCameraMode() {

    for (int frame = 0; true; frame++) {

        // Measuring elapsed time
        // TODO: clock_gettime() -- this one!, (gettimeofday())
        clock_t begin = clock();

        // Grab video frame
        Mat in;
        input >> in; // type: CV_8UC3 (16)

        // Handle ending video
        if (!in.data) {
            return 1;
        }

        if (frame < CAMERA_INIT) continue;

        // Resize captured frame
        in = resizeImage(in, RESIZED_FRAME_WIDTH);

        // Output
        Mat out = Mat(in.rows, in.cols, in.type());

        // Detect face in own thread
        if (!faceFullDetector.isWorking()) {
            boost::thread workerThread(&Detector::detectFace, &faceFullDetector, in);
        }

        if (faceFullDetector.getFaces().size()) {
            this->updateFace(faceFullDetector.getBiggestFace(), this->fullFace);
        }

        // Keep maximum BUFFER_FRAMES size
        if (this->isBufferFull()) {
            // Erase first frame
            videoBuffer.erase(videoBuffer.begin());
        }

        // Start cropping frames to face only after init
        // TODO: Can't run without face!
        // Or can be choosen center of image
        pushInputToBuffer(in, frame);

        // Update bpm once bpmWorker ready
        controlMiddleWare(frame);

        // Start computing when buffer filled
        compute(true);

        // Show bpmVisualization video after initialization compute
        // TODO: Check if this is performance ok
        visualize(in, out, frame);

        if (saveOutput) {
            output.write(out);
        }

        // Merge original + adjusted
        hconcat(out, in, window);

        // Put the image onto a screen
        imshow(this->OSWindowName, window);

        // Handling frame rate & time for closing window
        if (waitKey(1) >= 0) break;

        // Stop measuring loop time
        clock_t end = clock();

        // Experimental fps settings
        // TODO: Check if working
        double elapsedMus = double(end - begin) / CLOCKS_PER_SEC * 1000000;
        int extraWaitMus = (elapsedMus > (CLOCKS_PER_SEC / this->fps)) ? 0 : int(LOOP_WAIT_TIME_MUS - elapsedMus + 0.5);
        usleep(extraWaitMus);
    }

    return 0;
}

void Bpm::pushInputToBuffer(Mat in, int index) {
    if (index > CAMERA_INIT && this->isFaceDetected(this->fullFace)) {
        pushInputToBuffer(in);
    }
}

void Bpm::pushInputToBuffer(Mat in) {
    if (this->isFaceDetected(this->fullFace)) {
        videoBuffer.push_back(in);
    }
}

void Bpm::controlMiddleWare(int index) {
    bool shouldCompute = (index > CAMERA_INIT + this->bufferFrames && this->isBufferFull() && !bpmWorker.isWorking());
    bool shouldUpdateMiddleWare = (!this->bpmWorker.isWorking() && this->bpmWorker.getInitialFlag() && this->isBufferFull());

    // If still computing update state to bpm detected
    if (this->bpmWorker.isBpmDetected()) {
        this->state = (this->state == COMPUTING) ? BPM_DETECTED : this->state;
    }

    // If still only bpm detected update state to visualization detected
    if (shouldUpdateMiddleWare) {
        this->state = (this->state == BPM_DETECTED) ? VISUALIZATION_DETECTED : this->state;
    }

    // Get visualization from worker
    if (shouldUpdateMiddleWare) {
        // Clear current bpmVisualization array
        this->bpmVisualization.clear();
        // Copy to loop bpmVisualization vid
        this->bpmWorker.getVisualization().swap(this->bpmVisualization);
        // Clear bpmWorker bpmVisualsubization array
        this->bpmWorker.clearVisualization();
        this->workerIteration++;
    }

    // Start computing when buffer filled
    if (shouldCompute) {
        // If still fetching update to computing
        this->state = (this->state == FETCHING) ? COMPUTING : this->state;
        compute();
    }
}

void Bpm::compute(bool thread) {
    if (thread) {
        boost::thread workerThread(&AmplificationWorker::compute, &bpmWorker, videoBuffer);
        mergeFaces();
    } else {
        bpmWorker.compute(videoBuffer);
    }
}

void Bpm::visualize(Mat & in, Mat & out, int index) {

    if (state == FETCHING) {
        out = in.clone();
        putText(out, "Needed more " + to_string(bufferFrames - index) + " frames", Point(20, out.rows - 30), FONT_HERSHEY_SIMPLEX,
                1.0, Scalar(200, 200, 200), 2);
    }
    else if (state == BPM_DETECTED) {
        out = in.clone();
        putText(out, to_string(this->bpmWorker.getBpm()), Point(out.cols / 2 - 30, out.rows - 30), FONT_HERSHEY_SIMPLEX, 1.0,
                Scalar(200, 200, 200), 2);
    }
    else if (state == VISUALIZATION_DETECTED) {
        visualizeAmplified(in, out, index);
    }
    else {
        out = in.clone();
    }

    // Face detection & forehead
    visualizeDetected(in);

}


void Bpm::visualizeDetected(Mat &in) {
    if (isFaceDetected(this->tmpFace)) {
        printRectOnFrame(in, tmpFace, Scalar(255,255,255));
    }
    if (isForeheadDetected()) {
        Rect foreheadGlobal = forehead;
        // Place to global space
        foreheadGlobal.x += tmpFace.x;
        foreheadGlobal.y += tmpFace.y;

        printRectOnFrame(in, foreheadGlobal, Scalar(255,255,255));
    }
}


void Bpm::visualizeAmplified(Mat &in, Mat &out, int index) {
    Mat visual = Mat::zeros(in.rows, in.cols, in.type());

    // As we crop mask in own thread while amplification
    // These steps are appli only if detected face positon has significantly changed
    Mat tmp = resizeImage(this->bpmVisualization.at(index % this->bpmVisualization.size()),
                          tmpFace.width - 2 * ERASED_BORDER_WIDTH);

    // Important range check
    Rect roi(tmpFace.x, tmpFace.y, tmp.cols, tmp.rows);
    handleRoiPlacement(roi, frameSize, ERASED_BORDER_WIDTH);
    roi.x = roi.y = 0;

    // Crop in case mask would be outside frame
    tmp = tmp(roi);

    tmp.copyTo(visual(Rect(tmpFace.x + ERASED_BORDER_WIDTH, tmpFace.y + ERASED_BORDER_WIDTH, tmp.cols, tmp.rows)));
    out = in + this->beatVisibilityFactor * visual;

    putText(out, to_string(this->bpmWorker.getBpm()), Point(220, out.rows - 30), FONT_HERSHEY_SIMPLEX, 1.0,
            Scalar(200, 200, 200), 2);
}

void Bpm::updateFace(Rect src, Rect& dst) {
    // After initial detection update only position (not size)
    if (!isFaceDetected(dst)) {
        dst = src;

        // Face both forehead detected -> fetching frames
        this->state = (this->state == DETECTING) ? FETCHING: this->state;
    }
}

void Bpm::updateTmpFace(Rect src) {
    float variation = DETECTOR_UPDATE_VARIATION;
    if (abs(this->tmpFace.x - src.x) > this->tmpFace.x * variation) {
        this->tmpFace.x = src.x;
    }
    if (abs(this->tmpFace.y - src.y) > this->tmpFace.y * variation) {
        this->tmpFace.y = src.y;
    }
    if (abs(this->tmpFace.width - src.width) > this->tmpFace.width * variation) {
        this->tmpFace.width = src.width;
    }
    if (abs(this->tmpFace.height - src.height) > this->tmpFace.height * variation) {
        this->tmpFace.height = src.height;
    }
}

void Bpm::mergeFaces() {
//    this->face = this->tmpFace;
}

bool Bpm::isFaceDetected(Rect face) {
    return !!face.x;
}

bool Bpm::isForeheadDetected() {
    return !!this->forehead.x;
}

bool Bpm::isBufferFull() {
    return videoBuffer.size() == this->bufferFrames;
}

void Bpm::handleDetector(Mat in, int type) {

    // Detect face in own thread
    // Update once face ready
    if (type == FULL_FACE) {
        // TODO: Change only if differs
        if (!faceFullDetector.isWorking()) {
            boost::thread workerThread(&Detector::detectFace, &faceFullDetector, in);
        }
        if (faceFullDetector.isDetected()) {
            this->updateFace(faceFullDetector.getBiggestFace(), this->fullFace);

            if (!isFaceDetected(this->bpmWorker.getFaceRoi())) {
                this->bpmWorker.setFaceRoi(this->fullFace);
            }
        }
    } else if (type == RESIZED_FACE) {
        if (!faceResizedDetector.isWorking()) {
            boost::thread workerThread(&Detector::detectFace, &faceResizedDetector, in);
        }
        if (faceResizedDetector.isDetected()) {
            this->updateFace(faceResizedDetector.getBiggestFace(), this->resizedFace);
            this->updateTmpFace(faceResizedDetector.getBiggestFace());

            // Update size of detected face in resized frame ->
            // do as much work in thread as possible
            this->bpmWorker.setResizedFaceSize(faceResizedDetector.getBiggestFace().size());
        }
    } else if (type == FOREHEAD) {
        if (!foreheadDetector.isWorking() && isFaceDetected(tmpFace)) {
            // TODO: This could be better
            Rect roi = tmpFace;
            handleRoiPlacement(roi, in.size());
            boost::thread workerThread(&Detector::detectForehead, &foreheadDetector, in(roi));
        }
        if (foreheadDetector.isDetected()) {
            if (!isForeheadDetected()) {
                this->forehead = foreheadDetector.getForehead();
            } else {
                Rect newForehead = foreheadDetector.getForehead();
                // Update only when significant change
                bool shouldUpdate = (abs(forehead.x - newForehead.x) > (2*DETECTOR_UPDATE_VARIATION * forehead.x)) || (abs(forehead.y - newForehead.y) > (2*DETECTOR_UPDATE_VARIATION * forehead.y));
                // And still must be centered
                shouldUpdate = shouldUpdate && Detector::shouldAcceptForehead(Rect(0, 0, resizedFace.width, resizedFace.height), newForehead);
                this->forehead = shouldUpdate ? newForehead : this->forehead;
            }

        }
    }
}

void Bpm::fillLoadingNotes() {
    this->stateNotes.push_back("Detecting face and forehead.");
    this->stateNotes.push_back("Please don't move.");
    this->stateNotes.push_back("Trying to compute your bpm.");
    this->stateNotes.push_back("Amplifying your bpm.");
    this->stateNotes.push_back("Showing amplified blood flow.");
}


void Bpm::renderMainWindow(Mat &a, Mat &b) {
    // Render state note to state bar
    renderStateBar();
    // Merge frames & status bar together
    mergeMainWindow(a, b);
}

void Bpm::mergeMainWindow(Mat &a, Mat &b) {
    // Merge original + adjusted
    hconcat(a, b, window);

    // Merge frames + status bar
    vconcat(window, stateBar, window);
}

void Bpm::renderStateBar() {
    // Default bg background
    this->stateBar = Scalar(246,246,246);
    putText(
            this->stateBar,
            this->stateNotes[this->state]+"...",
            Point(20, 20),
            FONT_HERSHEY_SIMPLEX,
            0.5f, // font scale
            Scalar(0,0,0), // color
            1 // thickness
    );
}
