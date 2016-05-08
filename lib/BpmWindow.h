//
// Created by Michal on 08/02/16.
//

#ifndef BPM_BPM_H
#define BPM_BPM_H

#include <iostream>
#include <boost/thread.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <highgui.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <chrono>

// Classes
#include "./Middleware.h"
#include "./Detector.h"

// Functions
#include "./imageOperation.h"

// Configuration
#include "./../constants.h"

// Detectors
#define FULL_FACE 10
#define RESIZED_FACE 20
#define FOREHEAD 30

// States
#define DETECTING 0
#define FETCHING 1
#define COMPUTING 2
#define BPM_DETECTED 3
#define VISUALIZATION_DETECTED 4


using namespace cv;
using namespace std;

class Bpm {
    private:
        // CAMERA_REAL_SOURCE_MODE / VIDEO_SOURCE_MODE
        int sourceMode;

        // DEFAULT VIDEO NAME - must be in repository
        string videoFilePath;

        // FOURIER_MASK_MODE / FAKE_BEATING_MODE
        int maskMode;

        // Bool save output
        bool saveOutput;
        string outputFilePath;

        // Visibility factor of pulse ampliufication mask
        float beatVisibilityFactor;

        // Input videoreader information
        double fps;
        int bufferFrames;

        // Detected faces
        Rect fullFace, resizedFace, tmpFace, forehead;

        // Size of resized input
        Size frameSize;
        Size origFrameSize;
        int doubleDownscalingLevel;

        // Video objects
        VideoCapture input;
        VideoWriter output;

        // Vector for storing captured frames
        vector<Mat> videoBuffer;

        // Processed mask of blood flow
        vector<Mat> bpmVisualization;

        // OS window
        Mat window;
        Mat stateBar;
        string OSWindowName;

        // Worker for computing
        Middleware bpmWorker;

        // Worker for computing
        Detector faceFullDetector;
        Detector faceResizedDetector;
        Detector foreheadDetector;

        // App state
        int state;
        vector <string> stateNotes;

        // Measuring data
        int measuringIteration;
        int workerIteration;

    public:
        void init(int mode, int maskMode);
        void initLoadingNotes();

        // Run modes
        int run();
        int runCameraMode();
        int runRealVideoMode();
        int runStaticVideoMode();

        // Buffer
        void pushInputToBuffer(Mat frame, int index);
        void pushInputToBuffer(Mat in);
        bool isBufferFull();

        // Middleware control
        void controlMiddleWare(int index);
        void compute(bool thread = true);

        // Visualization
        void visualize(Mat & in, Mat & out, int index);
        void visualizeDetected(Mat & in);
        void visualizeAmplified(Mat & in, Mat & out, int index, bool origSize);
        void visualizeAmplified(Mat & in, Mat & out, int index);

        // Detectors control
        void handleDetector(Mat in, int type);
        bool isFaceDetected(Rect face);
        bool isForeheadDetected();
        void updateFace(Rect src, Rect & dst);
        void updateTmpFace(Rect src);
        void mergeFaces();
        void updateFps(std::chrono::time_point<std::chrono::high_resolution_clock> & prev, int index);

        // OS Window
        void renderMainWindow(Mat &a, Mat &b, int main);
        void mergeMainWindow(Mat &a, Mat &b);
        void renderStateBar(int index);

        void setMode(int mode) {
            Bpm::maskMode = mode;
        }

        void setVideoFileName(const string &videoFileName) {
            Bpm::videoFilePath = videoFileName;
        }

        void setOutputFilePath(const string &outputFilePath) {
            Bpm::outputFilePath = outputFilePath;
            Bpm::saveOutput = true;
        }
};


#endif //BPM_BPM_H
