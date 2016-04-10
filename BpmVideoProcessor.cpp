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
}


void BpmVideoProcessor::amplifyFrequencyInPyramid(vector<vector<Mat> > &pyramid, vector<Mat> &temporalSpatial, vector<Mat> &dst, float bpm) {
    // TODO: Each level must be in thread!!
    for (int i = 1; i < level; i++) {


        vector<Mat> tmp(temporalSpatial.size());
        amplifyFrequencyInLevel(pyramid.at(i), temporalSpatial, tmp, bpm);
        pyrUpVideo(tmp, pyramid.at(0)[0].size(), i);

        for (int j = 0; j < pyramid.at(0).size(); j++) {
            if (dst[j].data) {
                dst[j] += tmp[j];
            } else {
                tmp[j].copyTo(dst[j]);
            }
        }
    }

    normalizeVid(dst, 0, 150, NORM_MINMAX );
}

void BpmVideoProcessor::amplifyFrequencyInLevel(vector<Mat> src, vector<Mat> &temporalSpatial, vector<Mat> &dst,
                                                float bpm) {
    // Create temporal spatial video
    createTemporalSpatial(src, temporalSpatial);

    // Bandpass temporal video
    bandpass(temporalSpatial, bpm);

    // Inverse temporal spatial to video
    inverseTemporalSpatial(temporalSpatial, dst);

    temporalSpatial.clear();
}

void BpmVideoProcessor::buildGDownPyramid(vector<Mat> &src, vector<vector <Mat> > &pyramid, int level) {
    int framesInPart = 50;
    int parts = ceil(src.size() / framesInPart);

    // This is for performance purposes
//    vector<boost::thread *> z;
//    vector <vector <Mat>>;
//    for (int i = 0; i < parts; i++) {
//
//        z.push_back(new boost::thread());
//    }
//
//    for (int i = 0; i < parts; i++) {
//        z.push_back(new boost::thread());
//    }

    for (int currLevel = 0; currLevel < level; currLevel++) {
        buildGDownPyramidLevel(src, pyramid.at(currLevel), currLevel);
    }
}

void BpmVideoProcessor::buildGDownPyramidLevel(vector<Mat> &src, vector<Mat> &dst, int currLevel) {
    for (int i = 0; i < src.size(); i++) {
        // 0 Level only copy
        if (currLevel == 0) {
            pyramid.at(currLevel).push_back(src[i]);
            continue;
        }

        // Minimal size of frame in pyramid
        if ((int) round(src[i].cols / 2.0f) <= MIN_WIDTH_IN_PYRAMID) {
            // Update level - needed for upsizing
            this->level = currLevel;
            break;
        }

        if (src[i].type() == CV_32FC3) {
            src[i].convertTo(src[i], CV_8UC3);
        }

        cvtColor2(src[i], src[i], CV2_BGR2YIQ); // returns CV_8UC3
        src[i].convertTo(src[i], CV_32FC3);

        pyrDown(src[i], src[i]);

        src[i].convertTo(src[i], CV_8UC3);
        cvtColor2(src[i], src[i], CV2_YIQ2BGR); // returns CV_8UC3
        src[i].convertTo(src[i], CV_32FC3);
        dst.push_back(src[i]);
    }
}


// TODO: Check - may not work properly
void BpmVideoProcessor::bandpass(vector<Mat>& temporalSpatial, float freq) {

    // Create mask based on strongest frequency
    Mat mask = generateFreqMask(freq);

    for (int i = 0; i < temporalSpatial.size(); i++) {
        
        for (int row = 0; row < temporalSpatial[i].rows; row++) {

            // FFT
            Mat fourierTransform;
            dft(temporalSpatial[i].row(row), fourierTransform, cv::DFT_SCALE | cv::DFT_COMPLEX_OUTPUT);

            // MASKING
            Mat planes[] = {Mat::zeros(fourierTransform.size(), CV_32F), Mat::zeros(fourierTransform.size(), CV_32F)};

            // Real & imag part
            split(fourierTransform, planes);

            // Masking parts
            planes[0] = planes[0].mul(mask);
            planes[1] = planes[1].mul(mask);


            // Merge back
            merge(planes, 2, fourierTransform);

            // IFFT
            dft(fourierTransform, fourierTransform, cv::DFT_INVERSE|cv::DFT_REAL_OUTPUT);


            // COPY BACK
            fourierTransform.copyTo(temporalSpatial[i].row(row));

            // RELEASE
            fourierTransform.release();
        }
        
    }
}

void BpmVideoProcessor::createTemporalSpatial(vector<Mat> src, vector<Mat>& temporalSpatial) {
    int dstVectorCount = src[0].size().width;
    int dstVectorWidth = (int) src.size();
    int dstVectorHeight = src[0].size().height;

    for (int i = 0; i < dstVectorCount; i++) {
        // One frame
        Mat frame(dstVectorHeight, dstVectorWidth, CV_32FC1);
        for (int j = 0; j < dstVectorWidth; j++) {
            Mat tmp = src[j].clone();
            cvtColor(tmp, tmp, CV_BGR2GRAY);
            tmp.convertTo(tmp, CV_32F);

            for(int k = 0; k < dstVectorHeight; k++) {
                frame.at<float>(k,j) = tmp.at<float>(k,i);
            }
        }
        temporalSpatial.push_back(frame.clone());
        frame.release();
    }
}

void BpmVideoProcessor::inverseTemporalSpatial(vector<Mat>& temporalSpatial, vector<Mat>& dst) {
    int dstCount = temporalSpatial[0].cols;
    int dstWidth = (int) temporalSpatial.size();
    int dstHeight = temporalSpatial[0].rows;

    for (int i = 0; i < dstCount; i++) {
        Mat outputFrame = Mat(dstHeight, dstWidth, CV_32FC1);

        for (int j = 0; j < dstWidth; j++) {
            for(int k = 0; k < dstHeight; k++) {
                    outputFrame.at<float>(k, j) = temporalSpatial[j].at<float>(k, i);
            }
        }

        cvtColor(outputFrame, outputFrame, CV_GRAY2BGR);
        outputFrame.convertTo(outputFrame, CV_8U);

        // Keep only red channel
        vector <Mat> channels;
        split(outputFrame, channels);
        channels[GREEN_CHANNEL] = channels[GREEN_CHANNEL]*0.1f;
        channels[BLUE_CHANNEL] = channels[BLUE_CHANNEL]*0.3f;
        merge(channels, outputFrame);

        // in range [0,255]
        normalize(outputFrame, outputFrame, 0, 150, NORM_MINMAX );

        dst.push_back(outputFrame.clone());
        outputFrame.release();
    }
}

void BpmVideoProcessor::amplifyVideoChannels(vector<Mat> &video, float r, float g, float b) {
    for (int i = 0; i < video.size(); i++) {
        // MASKING
        vector<Mat> channels;
        // Real & imag part
        split(video[i], channels);
        amplifyChannels(channels, r, g, b);
        merge(channels, video[i]);
    }
}

Mat BpmVideoProcessor::generateFreqMask(float freq) {
    float halfRange = this->maskWidth / 2;
    float fl, fh;

    // Compute fl & fh
    if (freq - halfRange < CUTOFF_FL) {
        fl = CUTOFF_FL;
        fh = CUTOFF_FH + this->maskWidth;
    }
    else if (freq + halfRange > CUTOFF_FH) {
        fh = CUTOFF_FH;
        fl = CUTOFF_FH - this->maskWidth;
    }
    else {
        fl = freq - halfRange;
        fh = freq + halfRange;
    }

    return maskingCoeffs(framesCount,  fl, fh, fps);
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
