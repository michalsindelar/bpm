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
        this->input = VideoCapture((string) VIDEO_SAMPLES_DIR + "/mom_fine_81.mov");
        if (!input.isOpened()) cout << "Unable to open Video File";
        this->fps = (int) round(this->input.get(CV_CAP_PROP_FPS));

        if (this->sourceMode == VIDEO_REAL_SOURCE_MODE) {
            this->bufferFrames = BUFFER_FRAMES;
        }
        else if (this->sourceMode == VIDEO_STATIC_SOURCE_MODE) {
            // TODO: Check int vs double
//            this->bufferFrames = BUFFER_FRAMES;mouse
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

    this->showProcessRatio = (float) RESIZED_FRAME_WIDTH / (float) RESIZED_FRAME_WIDTH_FOR_PROCESSING ;
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
            return runCameraMode();
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

        // Reset video
        if (!in.data) {
            input.set(CV_CAP_PROP_POS_MSEC, 0);
            input >> in;
        }

        // Resize captured frame
        in = resizeImage(in, RESIZED_FRAME_WIDTH);

        if (frame < CAMERA_INIT) continue;

        // Keep maximum BUFFER_FRAMES size
        if (this->isBufferFull()) {
            // Erase first frame
            videoBuffer.erase(videoBuffer.begin());
        }

        // Start cropping frames to face only after init
        pushInputToBuffer(in, frame);

        // Detect face in own thread
        if (!faceDetector.isWorking()) {
            boost::thread workerThread(&FaceDetectorWorker::detectFace, &faceDetector, in);
        }

        // Output
        Mat out = Mat(in.rows, in.cols, in.type());

        if (faceDetector.getFaces().size()) {
            // TODO: function get biggest face
            this->updateFace(faceDetector.getBiggestFace());
        }

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
    }

    return 0;
}

int Bpm::runStaticVideoMode() {

    // Fill buffer from whole video
    Mat in;

    // Detect face at first
    while(!isFaceDetected()) {
        input >> in;
        in = resizeImage(in, RESIZED_FRAME_WIDTH);
        faceDetector.detectFace(in);
        if (faceDetector.getFaces().size()) {
            this->updateFace(faceDetector.getBiggestFace());
        }
    }
    // Reset video
    input.set(CV_CAP_PROP_POS_MSEC, 0);

    int bufferFrames = 0;
    while(in.cols != 0 && in.data) {
        // Detect face in own thread

        in = resizeImage(in, RESIZED_FRAME_WIDTH);
        pushInputToBuffer(in);

        if (!faceDetector.isWorking()) {
            boost::thread workerThread(&FaceDetectorWorker::detectFace, &faceDetector, in);
        }
        if (faceDetector.getFaces().size()) {
            // TODO: function get biggest face
            this->updateFace(faceDetector.getFaces()[0]);
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
        if (!faceDetector.isWorking()) {
            boost::thread workerThread(&FaceDetectorWorker::detectFace, &faceDetector, in);
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
        if (!faceDetector.isWorking()) {
            boost::thread workerThread(&FaceDetectorWorker::detectFace, &faceDetector, in);
        }

        if (faceDetector.getFaces().size()) {
            // TODO: function get biggest face
            this->updateFace(faceDetector.getBiggestFace());
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
    if (index > CAMERA_INIT && this->isFaceDetected()) {
        pushInputToBuffer(resizeImage(in, RESIZED_FRAME_WIDTH_FOR_PROCESSING));
    }
}

void Bpm::pushInputToBuffer(Mat in) {
    if (this->isFaceDetected()) {
        face.width = ((face.x + face.width) > in.cols) ? face.width - (face.x + face.width - in.cols) : face.width;
        face.height = ((face.y + face.height) > in.rows) ? face.height - (face.y + face.height - in.rows) : face.height;

        Rect roi(face.x, face.y, face.width, face.height);
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


void Bpm::visualize(Mat in, Mat & out, int index) {
    if (this->bpmWorker.getInitialFlag()) {
        // AMPLIFICATION FOURIER MODE
        Mat visual = Mat::zeros(in.rows, in.cols, in.type());

        // As we crop mask in own thread while amplification
        // These steps are appli only if detected face positon has significantly changed
        Mat tmp = resizeImage(this->bpmVisualization.at(index % this->bufferFrames), tmpFace.width - 2*ERASED_BORDER_WIDTH);

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
    return videoBuffer.size() == this->bufferFrames;
}
