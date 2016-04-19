//
// Created by Michal on 26/03/16.
//

#include "BpmVideoProcessor.h"

BpmVideoProcessor::BpmVideoProcessor(vector<Mat> video, float fl, float fh, int level, int fps, int framesCount, Rect faceRoi) {

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

    // Pre-allocate
    setMaxPyramidLevel();
}


void BpmVideoProcessor::computeAmplifiedMask() {

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
                                      vector<Mat>(faceVideo.begin() + start, faceVideo.begin() + end), level, bpm, fps));

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

void BpmVideoProcessor::computeBpm(int computeType) {
    // TODO: Consider converting to ntsc
    switch (computeType) {
        case AVG_COMPUTE:
            this->foreheadIntensities = countIntensities(forehead, 0, 1, 0);
            break;
        case MEAN_COMPUTE:
            this->foreheadIntensities = countMeanValues(forehead, GREEN_CHANNEL);
            break;
        // Default mean values
        default:
            this->foreheadIntensities = countMeanValues(forehead, GREEN_CHANNEL);
            break;
    }

    vector <double> globalIntensities = countOutsideIntensities(origVideo, faceRoi, 0, 1, 0, computeType);;
    //suppressGlobalChanges(this->foreheadIntensities, globalIntensities);

    this->bpm = (int) round(findStrongestRowFreq(foreheadIntensities, framesCount, fps));


    if (DATA_LOGGING) {
        saveIntensities(this->foreheadIntensities, (string) DATA_DIR+"/localIntensities.txt");
        saveIntensities(globalIntensities, (string) DATA_DIR+"/global.txt");
     }

}

void BpmVideoProcessor::setMaxPyramidLevel() {
    // We want as many pyramid levels as possible
    int width = faceVideo[0].cols;
    for (int i = 0; ; i++) {
        width = (int) round(width / 2.0f);
        // Minimal size of frame in pyramid
        if ((int) round(width) <= MIN_WIDTH_IN_PYRAMID) {
            // Update level - needed for upsizing
            this->level = i;
            break;
        }

    }
}

void BpmVideoProcessor::getForeheadSkinArea() {

    // At first we try to detect forehead using eyes detection 10x
    int detected = false;
    // TODO: Check how long does it take
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

    if (DATA_LOGGING){
        for (int i = 0; i < 10; i++) {
            Mat tmp = faceVideo[i];
            rectangle(tmp, Point(foreheadRoi.x, foreheadRoi.y), Point(foreheadRoi.x + foreheadRoi.width, foreheadRoi.y + foreheadRoi.height), Scalar(255,255,255));
            imwrite( (string) PROJECT_DIR+"/images/forehead/head-sking"+to_string(i)+".jpg", tmp );
            imwrite( (string) PROJECT_DIR+"/images/forehead/forehead"+to_string(i)+".jpg", forehead[i]);
            imwrite( (string) PROJECT_DIR+"/images/forehead/head"+to_string(i)+".jpg", faceVideo[i] );
        }
    }

}

// Currently not used -> same results from only one level == orig video
void BpmVideoProcessor::computeBpmFromPyramid() {
    float bpmSum = 0;
    int bpmLevel = 0;
    for (; bpmLevel < pyramidForehead.size(); bpmLevel++) {
        this->foreheadIntensities = countIntensities(pyramidForehead.at(bpmLevel), 0, 1, 0);
        bpmSum += (int) round(findStrongestRowFreq(foreheadIntensities, framesCount, fps));
    }
    this->bpm = (int) round(bpmSum / bpmLevel);
}