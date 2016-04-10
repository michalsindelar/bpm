//
// Created by Michal on 26/03/16.
//

#include "BpmVideoProcessor.h"

BpmVideoProcessor::BpmVideoProcessor(vector<Mat> video, float fl, float fh, int level, int fps, int framesCount) {
    this->video = video;
    this->fl = fl;
    this->fh = fh;
    // TODO: Level will be dynamic ??
    this->level = level;
    this->fps = fps;
    this->framesCount = framesCount;
    this->maskWidth = FREQ_MASK_WIDTH;
    this->getForeheadSkinArea();

    // Allocate
    this->pyramid = vector <vector <Mat> >(level);
    this->out = vector <Mat>(video.size());
}

void BpmVideoProcessor::compute() {

    // GDown pyramid for masking video
    buildGDownPyramid(video, pyramid, level);

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

    // Compute bpm from intensities
    this->intensities = countIntensities(forehead, 0, 1, 0);
    this->bpm = (int) round(findStrongestRowFreq(intensities, framesCount, fps));

    // Create beating mask for visualization
    amplifyFrequencyInPyramid(pyramid, temporalSpatial, out, bpm);
    reconstructMaskFromPyramid(pyramid, out);
}


void BpmVideoProcessor::amplifyFrequencyInPyramid(vector<vector<Mat> > &pyramid, vector<Mat> &temporalSpatial, vector<Mat> &dst, float bpm) {


    vector<boost::thread *> z;
    vector <PyramidLevelWorker> workerParts;
    // Initialize workers
    for (int i = 1; i < level; i++) {
        workerParts.push_back(PyramidLevelWorker());
    }

    for (int i = 1; i < level; i++) {
        z.push_back(new boost::thread(&PyramidLevelWorker::computeAmplificationPyramidLevel, boost::ref(workerParts[i-1]), pyramid.at(i), bpm, fps));
        // amplifyFrequencyInLevel(pyramid.at(i), temporalSpatial, pyramid.at(i), bpm, fps);
    }

    // Wait for threads
    for (int i = 1; i < level; i++) {
        z[i-1]->join();
        delete z[i-1];
    }

    // Swap
    for (int i = 1; i < level; i++) {
        workerParts[i-1].getDst().swap(pyramid.at(i-1));
    }
}

void BpmVideoProcessor::reconstructMaskFromPyramid (vector<vector<Mat> > &pyramid, vector <Mat>& dst) {
    // TODO: Each level must be in thread!!
    for (int i = 1; i < level; i++) {
        pyrUpVideo(pyramid.at(i), pyramid.at(0)[0].size(), i);

        for (int j = 0; j < pyramid.at(0).size(); j++) {
            if (dst[j].data) {
                dst[j] += pyramid.at(i-1)[j];
            } else {
                pyramid.at(i-1)[j].copyTo(dst[j]);
            }
        }
    }

    normalizeVid(dst, 0, 150, NORM_MINMAX );
}

void BpmVideoProcessor::buildGDownPyramid(vector<Mat> &src, vector<vector <Mat> > &pyramid, int level) {
    int framesInPart = 10;
    int parts = ceil(src.size() / framesInPart);

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
                pyramid[currLevel].push_back(tmp[i]);
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
