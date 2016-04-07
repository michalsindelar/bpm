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

// Classes
#include "./AmplificationWorker.h"
#include "Detector.h"

// Functions
#include "./imageOperation.h"

// Configuration
#include "./config.h"

#include "skinDetect.h"

#define FULL_FACE 1
#define RESIZED_FACE 2


using namespace cv;
using namespace std;

class Bpm {
    private:
        // CAMERA_REAL_SOURCE_MODE / VIDEO_SOURCE_MODE
        int sourceMode;

        // FOURIER_MASK_MODE / FAKE_BEATING_MODE
        int maskMode;

        // Bool save output
        bool saveOutput;

        // Visibility factor of pulse ampliufication mask
        float beatVisibilityFactor;

        // Input videoreader information
        int fps;
        int bufferFrames;

        // Detected faces
        Rect fullFace;
        Rect resizedFace;
        Rect tmpFace;

        // Size of resized input
        Size frameSize;

        // Video objects
        VideoCapture input;
        VideoWriter output;

        // Vector for storing captured frames
        vector<Mat> videoBuffer;

        // Processed mask of blood flow
        vector<Mat> bpmVisualization;

        // OS window
        Mat window;
        string OSWindowName;

        // Worker for computing
        AmplificationWorker bpmWorker;

        // Worker for computing
        Detector faceFullDetector;
        Detector faceResizedDetector;
        Detector foreheadDetector;

        Detector smallFaceDetector;

        // Measuring data
        int measuringIteration;
        int workerIteration;

    public:
        Bpm(int mode, int maskMode, float beatVisibilityFactor);

        // Runt modes
        int run();
        int runCameraMode();
        int runRealVideoMode();
        int runStaticVideoMode();

        void updateFace(Rect src, Rect & dst);
        void updateTmpFace(Rect src);
        void mergeFaces();
        bool isFaceDetected(Rect face);
        bool isBufferFull();

        void pushInputToBuffer(Mat frame, int index);
        void pushInputToBuffer(Mat in);
        void controlMiddleWare();
        void compute(int index);
        void compute();
        void visualize(Mat & in, Mat & out, int index);

        void handleDetector(Mat in, int type);

        void setMode(int mode) {
            Bpm::maskMode = mode;
        }
};


#endif //BPM_BPM_H
