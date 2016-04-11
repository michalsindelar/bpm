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
    this->OSWindowName = "OS window";
    this->workerIteration = 0;

    if (this->sourceMode == CAMERA_SOURCE_MODE) {
        // Open Video Camera
        this->input = VideoCapture(0);
        if (!input.isOpened()) cout << "Unable to open Video Camera";
        this->fps = FPS;
        this->bufferFrames = BUFFER_FRAMES;
    }

    else if (this->sourceMode == VIDEO_REAL_SOURCE_MODE || this->sourceMode == VIDEO_STATIC_SOURCE_MODE) {
        // Open Video Camera
        this->input = VideoCapture((string) VIDEO_SAMPLES_DIR + "/old/mom_fine_81.mov");
        if (!input.isOpened()) cout << "Unable to open Video File";
        this->fps = (int) round(this->input.get(CV_CAP_PROP_FPS));

        if (this->sourceMode == VIDEO_REAL_SOURCE_MODE) {
            this->bufferFrames = BUFFER_FRAMES;
        }
        else if (this->sourceMode == VIDEO_STATIC_SOURCE_MODE) {
            // TODO: Check int vs double
//            this->bufferFrames = BUFFER_FRAMES;
        }

    }

    this->frameSize = getResizedSize(
            Size(this->input.get(CV_CAP_PROP_FRAME_WIDTH), this->input.get(CV_CAP_PROP_FRAME_HEIGHT)),
            RESIZED_FRAME_WIDTH);

    // Initialize middleware
    this->bpmWorker = AmplificationWorker();
    bpmWorker.setFps(fps);
    bpmWorker.setBufferFrames(bufferFrames);

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
            return runCameraMode();
    }
}

int Bpm::runRealVideoMode() {

    for (int frame = 0; true; frame++) {

        // Exit measuring
        if (false && (workerIteration > this->measuringIteration)) {
            break;
        };

        // Grab video frame
        Mat in;
        input >> in; // type: CV_8UC3 (16)

        if (frame < CAMERA_INIT) continue;

        // Reset video when video ends
        if (!in.data) {
            input.set(CV_CAP_PROP_POS_MSEC, 0);
            input >> in;
        }

        // Keep maximum BUFFER_FRAMES size
        if (this->isBufferFull()) {
            videoBuffer.erase(videoBuffer.begin());
        }

        // Start cropping frames to face only after init
        pushInputToBuffer(in, frame);

        // Check full face detector
        handleDetector(in, FULL_FACE);

        // Resize captured frame
        in = resizeImage(in, RESIZED_FRAME_WIDTH);

        // Check resized face detector
        handleDetector(in, RESIZED_FACE);
        handleDetector(in, FOREHEAD);

        // Output
        Mat out = Mat(in.rows, in.cols, in.type());

        // Update bpm once bpmWorker ready
        controlMiddleWare();

        // Start computing when buffer filled
        compute(frame);

        // Show bpmVisualization video after initialization compute
        // TODO: Check if this is performance ok
        visualize(in, out, frame);

        if (false) {
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

int Bpm::runStaticVideoMode() {

    // Fill buffer from whole video
    Mat in;

    // Detect face at first
    while(!isFaceDetected(this->fullFace)) {
        input >> in;
        in = resizeImage(in, RESIZED_FRAME_WIDTH);
        faceFullDetector.detectFace(in);
        if (faceFullDetector.getFaces().size()) {
            this->updateFace(faceFullDetector.getBiggestFace(), this->fullFace);
        }
    }
    // Reset video
    input.set(CV_CAP_PROP_POS_MSEC, 0);

    int bufferFrames = 0;
    while(in.cols != 0 && in.data) {
        // Detect face in own thread

        in = resizeImage(in, RESIZED_FRAME_WIDTH);

        if (!faceFullDetector.isWorking()) {
            boost::thread workerThread(&Detector::detectFace, &faceFullDetector, in);
        }
        if (faceFullDetector.getFaces().size()) {
            // TODO: function get biggest face
            this->updateFace(faceFullDetector.getBiggestFace(), this->resizedFace);
        }

        pushInputToBuffer(in);

        input >> in;
        bufferFrames++;

        waitKey(1);
    }

    this->bufferFrames = bufferFrames;
    bpmWorker.setBufferFrames(bufferFrames);

    compute();
    this->bpmVisualization = this->bpmWorker.getVisualization();
    this->bpmWorker.clearVisualization();

    // Reset video
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

        // Detect face in own thread
        if (!faceFullDetector.isWorking()) {
            boost::thread workerThread(&Detector::detectFace, &faceFullDetector, in);
        }

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
        controlMiddleWare();

        // Start computing when buffer filled
        compute(frame);

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
        fullFace.width = ((fullFace.x + fullFace.width) > in.cols) ? fullFace.width - (fullFace.x + fullFace.width - in.cols) : fullFace.width;
        fullFace.height = ((fullFace.y + fullFace.height) > in.rows) ? fullFace.height - (fullFace.y + fullFace.height - in.rows) : fullFace.height;

        Rect roi(fullFace.x, fullFace.y, fullFace.width, fullFace.height);
        handleRoiPlacement(roi, in.size(), ERASED_BORDER_WIDTH);
        Mat croppedToFace = in(roi).clone();
        videoBuffer.push_back(croppedToFace);
    }
}

void Bpm::controlMiddleWare() {
    bool shouldUpdateMiddleWare = (!this->bpmWorker.isWorking() && this->bpmWorker.getInitialFlag() && this->isBufferFull());
    if (shouldUpdateMiddleWare) {
        // Clear current bpmVisualization array
        this->bpmVisualization.clear();
        // Copy to loop bpmVisualization vid
        this->bpmWorker.getVisualization().swap(this->bpmVisualization);
        // Clear bpmWorker bpmVisualsubization array
        this->bpmWorker.clearVisualization();
        this->workerIteration++;
    }
}

void Bpm::compute(int index) {
    bool shouldCompute = (index > CAMERA_INIT + this->bufferFrames && this->isBufferFull() && !bpmWorker.isWorking());
    if (shouldCompute) {
        boost::thread workerThread(&AmplificationWorker::compute, &bpmWorker, videoBuffer);
        mergeFaces();
    }
}

void Bpm::compute() {
    bpmWorker.compute(videoBuffer);
}

void Bpm::visualize(Mat & in, Mat & out, int index) {
    if (this->bpmWorker.getInitialFlag()) {
        // AMPLIFICATION FOURIER MODE
        Mat visual = Mat::zeros(in.rows, in.cols, in.type());

        // As we crop mask in own thread while amplification
        // These steps are appli only if detected face positon has significantly changed
        Mat tmp = resizeImage(this->bpmVisualization.at(index % this->bpmVisualization.size()), tmpFace.width - 2*ERASED_BORDER_WIDTH);

        // Important range check
        Rect roi(tmpFace.x, tmpFace.y, tmp.cols, tmp.rows);
        handleRoiPlacement(roi, frameSize, ERASED_BORDER_WIDTH);
        roi.x = roi.y = 0;

        // Crop in case mask would be outside frame
        tmp = tmp(roi);

        tmp.copyTo(visual(Rect(tmpFace.x + ERASED_BORDER_WIDTH, tmpFace.y + ERASED_BORDER_WIDTH, tmp.cols, tmp.rows)));
        out = in + this->beatVisibilityFactor * visual;

        putText(out, to_string(this->bpmWorker.getBpm()), Point(220, out.rows - 30), FONT_HERSHEY_SIMPLEX, 1.0,Scalar(200,200,200),2);

    } else {
        out = in.clone();
        putText(out, "Loading... "+to_string(index), Point(220, out.rows - 30), FONT_HERSHEY_SIMPLEX, 1.0,Scalar(200,200,200),2);
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

void Bpm::updateFace(Rect src, Rect& dst) {
    // After initial detection update only position (not size)
    if (!isFaceDetected(dst)) {
        dst = src;
    }
}

void Bpm::updateTmpFace(Rect src) {
    float variation = FACE_UPDATE_VARIATION;
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
            this->bpmWorker.setResizedFace(faceResizedDetector.getBiggestFace().size());
        }
    } else if (type == FOREHEAD) {
        if (!foreheadDetector.isWorking() || isFaceDetected(tmpFace)) {
            // TODO: This could be better
            Rect roi = tmpFace;
            handleRoiPlacement(roi, in.size());
            boost::thread workerThread(&Detector::detectForehead, &foreheadDetector, in(roi));
        }
        if (foreheadDetector.isDetected()) {
            this->forehead = foreheadDetector.getForehead();
        }
    }
}
