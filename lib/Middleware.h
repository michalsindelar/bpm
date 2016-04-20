//
// Created by Michal on 08/02/16.
//

#ifndef BPM_AMPLIFICATIONWORKER_H
#define BPM_AMPLIFICATIONWORKER_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <highgui.h>

#include "./BpmVideoProcessor.h"

// Configuration
#include "constants.h"
#include <boost/thread.hpp>

using namespace cv;
using namespace std;

class AmplificationWorker {
    int bpm;
    int fps;

    bool working;
    bool bpmDetected;

    bool initialFlag;
    int bufferFrames;

    Size resizedFace;

    vector<Mat> videoBuffer;
    vector<Mat> visualization;

    Rect faceRoi;

    public:
        AmplificationWorker();
        void compute(vector<Mat> videoBuffer);
        void updateBpm(int bpm);

        bool isWorking() {
            return this->working;
        }
        bool getInitialFlag() {
            return initialFlag;
        }

        vector<Mat> & getVisualization() {
            return this->visualization;
        }

        int getBpm() const {
            return bpm;
        }

        void setFps(int fps) {
            AmplificationWorker::fps = fps;
        }

        void setBufferFrames(int bufferFrames) {
            AmplificationWorker::bufferFrames = bufferFrames;
        }

        void setVideoBuffer(vector<Mat> videoBuffer) {
            this->videoBuffer.swap(videoBuffer);
        }

        void clearVisualization() {
            this->visualization.clear();
        }

        void setResizedFaceSize(const Size &resizedFace) {
            AmplificationWorker::resizedFace = resizedFace;
        }

        bool isBpmDetected() const {
            return bpmDetected;
        }

        void setFaceRoi(const Rect &faceRoi) {
            AmplificationWorker::faceRoi = faceRoi;
        }

        const Rect &getFaceRoi() const {
            return faceRoi;
        }
};


#endif //BPM_AMPLIFICATIONWORKER_H
