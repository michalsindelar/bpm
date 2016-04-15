//
// Created by Michal on 26/03/16.
//

#include "BpmVideoProcessor.h"

BpmVideoProcessor::BpmVideoProcessor(vector<Mat> video, float fl, float fh, int level, int fps, int framesCount) {
    this->video = video;
    this->fl = fl;
    this->fh = fh;
    this->level = level;
    this->fps = fps;
    this->framesCount = framesCount;
    this->maskWidth = FREQ_MASK_WIDTH;

    this->getForeheadSkinArea();

    // Pre-allocate
    this->pyramid = vector <vector <Mat> >();
    this->pyramidForehead = vector <vector <Mat> >();

    this->out = vector <Mat>(video.size());
}


void BpmVideoProcessor::computeBpm() {
    // GDown pyramid for forehead
    buildGDownPyramid(forehead, pyramidForehead, level);

    // Compute bpm multi level
    computeBpmFromPyramid();
}

void BpmVideoProcessor::computeAmplifiedMask() {

    // Use only first FRAMES_FOR_VISUALIZATION frames - enough for fine amplification
    vector <Mat> cutVideo = vector <Mat>(video.begin(), video.begin() + FRAMES_FOR_VISUALIZATION);

    for (int i = 0; i < cutVideo .size(); i++) {
        pyrDown(cutVideo [i], cutVideo [i]);
        pyrDown(cutVideo [i], cutVideo [i]);
    }

    // GDown pyramid for masking cutVideo 
    buildGDownPyramid(cutVideo , pyramid, level);

    /*
    if (true) {
//        saveIntensities(countIntensities(forehead), (string) DATA_DIR+"/full-0.txt");
//        saveIntensities(countIntensities(forehead, 0, 1, 0), (string) DATA_DIR+"/green-0.txt");
        // Measure
//        ofstream dataFile;
//        dataFile.open((string) DATA_DIR + "/measure_72.txt", ios::app);
//        printIterationRow(blurred, framesCount, fps, 72, dataFile);
//        dataFile.close();
    }
     */

    // Create beating mask for visualization
    amplifyFrequencyInPyramid(pyramid, temporalSpatial, out, bpm);
    reconstructMaskFromPyramid(pyramid, out);


    for (int i = 0; i < cutVideo .size(); i++) {
        pyrUp(cutVideo [i], cutVideo [i]);
        pyrUp(cutVideo [i], cutVideo [i]);
    }
}


void BpmVideoProcessor::amplifyFrequencyInPyramid(vector<vector<Mat> > &pyramid, vector<Mat> &temporalSpatial, vector<Mat> &dst, float bpm) {


    vector<boost::thread *> z;
    vector <PyramidLevelWorker> workerParts;
    // Initialize workers
    for (int i = 0; i < level; i++) {
        workerParts.push_back(PyramidLevelWorker());
    }

    for (int i = 0; i < level; i++) {
        z.push_back(new boost::thread(&PyramidLevelWorker::computeAmplificationPyramidLevel, boost::ref(workerParts[i]), pyramid.at(i), bpm, fps));
    }

    // Wait for threads
    for (int i = 0; i < level; i++) {
        z[i]->join();
        delete z[i];
    }

    // Swap
    for (int i = 0; i < level; i++) {
        workerParts[i].getDst().swap(pyramid.at(i));
    }
}

void BpmVideoProcessor::reconstructMaskFromPyramid (vector<vector<Mat> > &pyramid, vector <Mat>& dst) {
    vector<boost::thread *> z;
    vector <PyramidLevelWorker> workerParts;
    // Initialize workers
    for (int i = 0; i < level; i++) {
        workerParts.push_back(PyramidLevelWorker());
    }

    for (int i = 0; i < level; i++) {
        z.push_back(new boost::thread(&PyramidLevelWorker::reconstructPyramidLevel, boost::ref(workerParts[i]), pyramid, i));
    }

    // Wait for threads
    for (int i = 0; i < level; i++) {
        z[i]->join();
        delete z[i];
    }

    // Dst is empty
    workerParts[0].getDst().swap(dst);

    // Now increment dst
    for (int i = 1; i < level; i++) {
        vector <Mat> tmp = workerParts[i].getDst();
        for (int j = 0; j < tmp.size(); j++) {
            dst[j] += tmp[j];
        }
    }

    // Normalize dst
    normalizeVid(dst, 0, 200, NORM_MINMAX );
}

void BpmVideoProcessor::buildGDownPyramid(vector<Mat> &src, vector<vector <Mat> > &pyramid, int level) {
    int framesInPart = 20;

    int parts = (int) ceil(src.size() / framesInPart);

    for (int currLevel = 0; currLevel < level; currLevel++) {

        // We need to split to threads due to performance
        vector<boost::thread *> z;
        vector <PyramidLevelWorker> workerParts;

        // Minimal size of frame in pyramid
        if ((int) round(src[0].cols / 2.0f) <= MIN_WIDTH_IN_PYRAMID) {
            // Update level - needed for upsizing
            this->level = currLevel;
            break;
        }

        // Allocate new level in pyramid
        pyramid.push_back(vector<Mat>());

        // Initialize workers
        for (int i = 0; i < parts; i++) {
            workerParts.push_back(PyramidLevelWorker());
        }

        for (int i = 0; i < parts; i++) {
            int start = i * framesInPart;
            int end = start + framesInPart;
            z.push_back(new boost::thread(&PyramidLevelWorker::computeGDownPyramidLevel, boost::ref(workerParts[i]),
                                          vector<Mat>(src.begin() + start, src.begin() + end), currLevel));
        }

        // Clear src video
        src.clear();

        // Wait for threads
        for (int i = 0; i < parts; i++) {
            z[i]->join();
            delete z[i];
        }

        // Copy thread parts to pyramid
        for (int i = 0; i < parts; i++) {
            vector <Mat> tmp = workerParts[i].getDst();
            for (int j = 0; j < tmp.size(); j++) {
                src.push_back(tmp[j]);
                pyramid[currLevel].push_back(tmp[j]);
            }
        }
    }
}

void BpmVideoProcessor::amplifyVideoChannels(vector<Mat> &video, float r, float g, float b) {
    for (int i = 0; i < video.size(); i++) {
        amplifyChannels(video[i], r, g, b);
    }
}

void BpmVideoProcessor::getForeheadSkinArea() {

    // At first we try to detect forehead using eyes detection 10x
    Rect foreheadRoi;
    int detected = false;
    // We try 10 times to detect
    for (int i = 0; i < 10; i++) {
        if (detectForeheadFromFaceViaEyesDetection(video[i], foreheadRoi)) {
            detected = true;
            break;
        }
    }

    // Unsuccessful eyes & forehead detection -> default forehead area
    if (!detected) {
        foreheadRoi = defaultForehead(video[0]);
    }

    cropToVideo(video, forehead, foreheadRoi);

    if (false) {
        for (int i = 0; i < 10; i++) {
            Mat tmp = video[i];
            rectangle(tmp, Point(foreheadRoi.x, foreheadRoi.y), Point(foreheadRoi.x + foreheadRoi.width, foreheadRoi.y + foreheadRoi.height), Scalar(255,255,255));
            imwrite( (string) PROJECT_DIR+"/images/forehead/head-sking"+to_string(i)+".jpg", tmp );
            imwrite( (string) PROJECT_DIR+"/images/forehead/forehead"+to_string(i)+".jpg", forehead[i]);
            imwrite( (string) PROJECT_DIR+"/images/forehead/head"+to_string(i)+".jpg", video[i] );
        }
    }

}

void BpmVideoProcessor::computeBpmFromPyramid() {
    float bpmSum = 0;
    int bpmLevel = 0;
    for (; bpmLevel < pyramidForehead.size(); bpmLevel++) {
        this->intensities = countIntensities(pyramidForehead.at(bpmLevel), 0, 1, 0);
        bpmSum += (int) round(findStrongestRowFreq(intensities, framesCount, fps));
    }
    this->bpm = (int) round(bpmSum / bpmLevel);
}