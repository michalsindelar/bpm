//
// Created by Michal on 26/03/16.
//

#include "BpmVideoProcessor.h"

BpmVideoProcessor::BpmVideoProcessor(vector<Mat> video, float fl, float fh, int level, double fps, int framesCount, Rect faceRoi) {

    this->origVideo = video;
    cropToVideo(video, this->faceVideo, faceRoi);

    this->fl = fl;
    this->fh = fh;
    this->level = level;
    this->fps = fps;
    this->framesCount = framesCount;
    this->maskWidth = FREQ_MASK_WIDTH;
    this->faceRoi = faceRoi;

    // Detect forehead
    this->getForeheadSkinArea();
}

/**
 * Creates amplified mask based on Eulerian magnification
 */
void BpmVideoProcessor::computeAmplifiedMask() {

    // Set pyr down resizing
    // Set closest pyr down level value to ideal frame width for amplification
    // TODO: Control if video is big enough -> in bpm.cpp
    this->doubleDownscalingLevel = setDoubleDownscalingLevel(this->faceVideo[0].cols, IDEAL_WIDTH_FOR_AMPLIFICATION);

    // Init settings of amplification
    setMaxPyramidLevel();

    vector<boost::thread *> z;
    vector <ThreadWorker> workerParts;

    int framesInPart = min(FRAMES_FOR_VISUALIZATION, (int) faceVideo.size());
    int parts = (int) ceil(faceVideo.size() / framesInPart);

    // Initialize workers
    for (int i = 0; i < parts; i++) {
        workerParts.push_back(ThreadWorker());
    }

    // Start to compute in threads
    for (int i = 0; i < parts; i++) {
        int start = i * framesInPart;
        int end = start + framesInPart;

        z.push_back(new boost::thread(&ThreadWorker::amplifyVideo, boost::ref(workerParts[i]),
                                      vector<Mat>(faceVideo.begin() + start, faceVideo.begin() + end),
                                      doubleDownscalingLevel, level, bpm, fps));
    }

    // Wait for threads
    for (int i = 0; i < parts; i++) {
        z[i]->join();
        delete z[i];
    }

    // Copy thread parts to pyramid
    for (int i = 0; i < parts; i++) {
        vector <Mat> tmp = workerParts[i].getResult();
        for (int j = 0; j < tmp.size(); j++) {
            out.push_back(tmp[j]);
        }
    }
}

/**
 * Handles whole porcess of computing heartbeaet
 */
void BpmVideoProcessor::computeBpm(int computeType) {

    // Not working experiments
    // 1. Stabilize forehead
    // stabilizeVideo(this->forehead);
    // 2. Supress global changes
    // vector <double> globalIntensities = countOutsideIntensities(origVideo, faceRoi, 0, 1, 0, computeType);;
    // suppressGlobalChanges(this->foreheadIntensities, globalIntensities);

    switch (computeType) {
        case AVG_COMPUTE:
            this->foreheadIntensities = countIntensities(forehead, 0, 1, 0);
            break;
        case MEDIAN_COMPUTE:
            this->foreheadIntensities = countMedianValues(forehead, GREEN_CHANNEL);
            break;
        // Default mean values
        default:
            this->foreheadIntensities = countMedianValues(forehead, GREEN_CHANNEL);
            break;
    }

    this->bpm = (int) round(findStrongestRowFreq(foreheadIntensities, framesCount, fps));

}

/**
 * Automatically set how many level should have pyramid
 */
void BpmVideoProcessor::setMaxPyramidLevel() {

    // Respect further downscaling of video
    float width = faceVideo[0].cols;
    for (int i = 0; i < doubleDownscalingLevel; i++) {
        width /= 2;
    }

    // We want as many pyramid levels as possible with width bigger than constant
    for (int i = 0; ; i++) {
        width /= 2;
        // Minimal size of frame in pyramid
        if (width <= MIN_WIDTH_IN_PYRAMID) {
            // Update level - needed for upsizing
            this->level = i;
            break;
        }
    }
}

/**
 * Detects forehead mask
 */
void BpmVideoProcessor::getForeheadSkinArea() {

    // At first we try to detect forehead using eyes detection 10x
    int detected = false;

    for (int i = 0; i < faceVideo.size(); i++) {
        if (Detector::detectForeheadFromFaceViaEyesDetection(faceVideo[i], this->foreheadRoi)) {
            detected = true;
            break;
        }
    }

    // Unsuccessful eyes & forehead detection -> default forehead area
    if (!detected) {
        foreheadRoi = Detector::defaultForehead(faceVideo[0]);
    }

    cropToVideo(faceVideo, forehead, foreheadRoi);
}

/**
 * Computes bpm in each level of gaussian pyramid
 */
void BpmVideoProcessor::computeBpmFromPyramid() {
    float bpmSum = 0;
    int bpmLevel = 0;
    for (; bpmLevel < pyramidForehead.size(); bpmLevel++) {
        this->foreheadIntensities = countIntensities(pyramidForehead.at(bpmLevel), 0, 1, 0);
        bpmSum += (int) round(findStrongestRowFreq(foreheadIntensities, framesCount, fps));
    }
    this->bpm = (int) round(bpmSum / bpmLevel);
}