//
// Created by Michal on 08/02/16.
//

#include "BpmWindow.h"

// Constructor



void Bpm::init(int sourceMode, int maskMode) {

    this->sourceMode = sourceMode;
    this->maskMode = maskMode;
    this->beatVisibilityFactor = 0.5f;

    this->OSWindowName = "Bpm";
    this->workerIteration = 0;


    // CAMERA SOURCE MODE
    if (this->sourceMode == CAMERA_SOURCE_MODE) {
        // Open Video Camera
        this->input = VideoCapture(0);
        if (!input.isOpened()) cout << "Unable to open Video Camera";

        // FPS is dynamically set during grabbing frames
        this->fps = 0;
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
    }

    // STORE ORIGINAL DIMENSION
    this->origFrameSize = Size(this->input.get(CV_CAP_PROP_FRAME_WIDTH), this->input.get(CV_CAP_PROP_FRAME_HEIGHT));

    // GET DOUBLE DOWNSCALING LEVEL
    // according to orientation
    if (origFrameSize.width > origFrameSize.height) {
        this->doubleDownscalingLevel = setDoubleDownscalingLevel(origFrameSize.width, RESIZED_FRAME_WIDTH);
    } else {
        this->doubleDownscalingLevel = setDoubleDownscalingLevel(origFrameSize.height, RESIZED_FRAME_WIDTH);
    }

    // COMPUTE RESIZED FRAME SIZE according to orientatin
    this->frameSize = getResizedSize(origFrameSize, origFrameSize.width / (pow(2,doubleDownscalingLevel)));

    // INITALIZE MIDDLEWARE
    this->bpmWorker = Middleware();
    this->bpmWorker.setFps(fps);
    this->bpmWorker.setBufferFrames(bufferFrames);


    // MAYBE OPEN OUTPUT FILE
    if (saveOutput) {
        Size outputSize = (this->sourceMode == VIDEO_STATIC_SOURCE_MODE) ? origFrameSize : frameSize;
        output.open(this->outputFilePath, CV_FOURCC('M', 'J', 'P', 'G'), this->fps, outputSize, true);
        this->bpmWorker.setOutputFilePath(this->outputFolderPath);
    }

    this->measuringIteration = 20;
    this->workerIteration = 0;

    if (false) {
        ofstream dataFile;
        dataFile.open((string) DATA_DIR + "/measure_72.txt", ios::out);
        printIterationHead(dataFile);
        dataFile.close();
    }

    // Initial state
    this->state = DETECTING;

    // State notes
    initLoadingNotes();

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

    return 0;
}

int Bpm::runRealVideoMode() {

    // Timestamps
    auto prevGrabTime = std::chrono::steady_clock::now();

    // In frame
    Mat in;

    for (int index = 0; true; index++) {

        // Grab frame from input stream
        input >> in; // type: CV_8UC3 (constant 16)

        // Skip initial frames because possible artefacts in video
        if (index < CAMERA_INIT) continue;

        // Dynamically updating fps in camera mode
        if (this->sourceMode == CAMERA_SOURCE_MODE) {
            updateFps(prevGrabTime, index);
        }

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

        pushInputToBuffer(in, index);

        // Resizing without interpolation -> downscaling 2n x
        resize(in, in, this->frameSize, 0, 0);

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


        // Start to save file after visualization is detected
        if (saveOutput && this->state == VISUALIZATION_DETECTED) {
            output.write(out);
        }

        renderMainWindow(in, out, index);

        // Put the image onto a screen
        imshow(this->OSWindowName, window);

        // Handling frame rate & time for closing window
        if (waitKey(1) == 27) break;
    }

    if (saveOutput) {
        output.release();
    }

    destroyAllWindows();
    return 0;
}

int Bpm::runStaticVideoMode() {

    // Fill buffer from whole video
    Mat in;
    int stddev = 00;

    // We need to have both faces detected
    while(!isFaceDetected(this->fullFace) || !isFaceDetected(this->resizedFace)) {
        // imGray is the grayscale of the input image
        input >> in;

        // Add noise
        cv::Mat noise = Mat(in.size(),CV_8UC3);
        cv::randn(noise, 0, stddev);
        in = in + noise;
        normalize(in, in, 0, 255, CV_MINMAX, CV_8UC3);

        // Check full face detector
        handleDetector(in, FULL_FACE);

        // Resize captured frame
        in = resizeImage(in, this->frameSize.width);

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

        // Add noise
        cv::Mat noise = Mat(in.size(),CV_8UC3);
        cv::randn(noise, 0, stddev);
        in = in + noise;
        normalize(in, in, 0, 255, CV_MINMAX, CV_8UC3);

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

        // Add noise
        cv::Mat noise = Mat(in.size(),CV_8UC3);
        cv::randn(noise, 0, stddev);
        in = in + noise;
        normalize(in, in, 0, 255, CV_MINMAX, CV_8UC3);

        // Reset video
        if (!in.data) {
            input.set(CV_CAP_PROP_POS_MSEC, 0);
            // TODO: Uncomment
            return 0;
            input >> in;

            this->saveOutput = false;
            this->output.release();
        }

        // In static mode we want to save full resolution image
        if (saveOutput) {
            Mat out = Mat(in.rows, in.cols, in.type());
            visualizeAmplified(in, out, frame, true);
            output.write(out);
        }

        // Resize captured frame!
        in = resizeImage(in, this->frameSize.width);

        // Output
        Mat out = Mat(in.rows, in.cols, in.type());

        visualizeAmplified(in, out, frame);

        // Merge original + adjusted
        hconcat(out, in, window);

        // Put the image onto a screen
        imshow(this->OSWindowName, window);

        // Handling frame rate & time for closing window
        if (waitKey(1) >= 0) break;
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
        // If still fetching update state to computing
        this->state = (this->state == FETCHING) ? COMPUTING : this->state;

        // In camera mode fps is computed dynamically
        this->bpmWorker.setFps(fps);
        compute();
    }
}

void Bpm::compute(bool thread) {
    if (thread) {
        boost::thread workerThread(&Middleware::compute, &bpmWorker, videoBuffer);
        mergeFaces();
    } else {
        bpmWorker.compute(videoBuffer);
    }
}

void Bpm::visualize(Mat & in, Mat & out, int index) {

    if (state == FETCHING) {
        out = in.clone();
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

void Bpm::visualizeAmplified(Mat &in, Mat &out, int index, bool origSize) {
    // Orig full size vs Resized
    Rect face = origSize ? this->fullFace : this->resizedFace;
    Size frameSize = origSize ? this->origFrameSize : this->frameSize;

    Mat visual = Mat::zeros(in.rows, in.cols, in.type());
    // As we crop mask in own THREAD while amplification
    // These steps are appli only if detected face positon has significantly changed
    Mat tmp = resizeImage(this->bpmVisualization.at(index % this->bpmVisualization.size()),
                          face.width - 2 * ERASED_BORDER_WIDTH);

    // Important range check
    Rect roi(face.x, face.y, tmp.cols, tmp.rows);
    handleRoiPlacement(roi, frameSize, ERASED_BORDER_WIDTH);
    roi.x = roi.y = 0;

    // Crop in case mask would be outside frame
    tmp = tmp(roi);

    tmp.copyTo(visual(Rect(face.x + ERASED_BORDER_WIDTH, face.y + ERASED_BORDER_WIDTH, tmp.cols, tmp.rows)));
    out = in + this->beatVisibilityFactor * visual;

    putText(out, to_string(this->bpmWorker.getBpm()), Point(out.cols - out.cols * 0.3, out.rows - 30), FONT_HERSHEY_SIMPLEX, 2.0,
            Scalar(200, 200, 200), 2);
}


void Bpm::visualizeAmplified(Mat &in, Mat &out, int index) {
    // Default face is tmp face
    visualizeAmplified(in, out, index, false);
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

void Bpm::initLoadingNotes() {
    this->stateNotes.push_back("Detecting face and forehead.");
    this->stateNotes.push_back("Please don't move.");
    this->stateNotes.push_back("Trying to compute your bpm.");
    this->stateNotes.push_back("Amplifying your bpm.");
    this->stateNotes.push_back("Showing amplified blood flow.");
}


void Bpm::renderMainWindow(Mat &a, Mat &b, int index) {
    // Render state note to state bar
    renderStateBar(index);
    // Merge frames & status bar together
    mergeMainWindow(a, b);
}

void Bpm::mergeMainWindow(Mat &a, Mat &b) {
    // Merge original + adjusted
    hconcat(a, b, window);

    // Merge frames + status bar
    vconcat(window, stateBar, window);
}

void Bpm::renderStateBar(int index) {
    // Default bg background

    this->stateBar = Scalar(246,246,246);
    putText(
            this->stateBar,
            this->stateNotes[this->state]
            + "..." + (this->state == FETCHING ? ("Needed more " + to_string(max(bufferFrames - index, 0)) + " frames") : ""),
            Point(20, 20),
            FONT_HERSHEY_SIMPLEX,
            0.5f, // font scale
            Scalar(0,0,0), // color
            1 // thickness
    );
}

void Bpm::updateFps(std::chrono::time_point<std::chrono::high_resolution_clock> & prev, int index) {
    if (index == CAMERA_INIT) {
        prev = std::chrono::steady_clock::now();
    } else if (this->sourceMode == CAMERA_SOURCE_MODE && index > CAMERA_INIT) {
        auto curr = chrono::steady_clock::now();
        auto duration = chrono::duration_cast< std::chrono::milliseconds> (curr - prev);
        double currFps = 1000.0f / duration.count();
        this->fps = (this->fps == 0) ? currFps : (this->fps + currFps) / 2.0f;
        prev = curr;
    }
}
