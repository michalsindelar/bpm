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
    this->levelForMask = 5;
    this->fps = fps;
    this->framesCount = framesCount;
    this->maskWidth = FREQ_MASK_WIDTH;
    this->getForeheadSkinArea();
}

void BpmVideoProcessor::compute() {
    // GDown pyramid for compute
    buildGDownStack(skinVideo, blurred, 1);
    // GDown pyramid for masking video
    buildGDownStack(video, blurredForMask, levelForMask);

    saveIntensities(countIntensities(blurred), (string) DATA_DIR+"/full-0.txt");
    saveIntensities(countIntensities(blurred, 0, 1, 0), (string) DATA_DIR+"/green-0.txt");

    if (true) {
        // Measure
//        ofstream dataFile;
//        dataFile.open((string) DATA_DIR + "/measure_72.txt", ios::app);
//        printIterationRow(blurred, framesCount, fps, 72, dataFile);
//        dataFile.close();
    }

    // Compute bpm from intensities
    this->intensities = countIntensities(blurred, 0, 1, 0);
    this->bpm = (int) round(findStrongestRowFreq(intensities, framesCount, fps));

    // Amplify blurred buffer's red channel
    amplifyVideoChannels(blurredForMask, 0,  10, 0);
    amplifyVideoChannels(blurred, 0, 10, 0);

    // Create temporal spatial video
    createTemporalSpatial();

    // Bandpass temporal video
    bandpass();

    // Inverse temporal spatial to video
    inverseTemporalSpatial();
}

void BpmVideoProcessor::buildGDownStack(vector<Mat> src, vector<Mat>& blurredDst, int level) {
    for (int i = 0; i < framesCount; i++) {
        Mat frame = src[i].clone();

        // TODO: REWRITE ctColor2 to float
        cvtColor2(frame, frame, CV2_BGR2YIQ); // returns CV_8UC3

        frame.convertTo(frame, CV_32FC3);

        // Blurring in level for mask at first
        for (int j = 0; j < level; j++) {
            pyrDown(frame, frame);
        }

        frame.convertTo(frame, CV_8UC3);

        cvtColor2(frame, frame, CV2_YIQ2BGR); // returns CV_8UC3

        frame.convertTo(frame, CV_32FC3);

        blurredDst.push_back(frame);
    }
}

void BpmVideoProcessor::bandpass() {

    // First of all we need to find strongest frequency for all
    this->intensities = countIntensities(blurred);
    float strongestTimeStackFreq = findStrongestRowFreq(intensities, framesCount, fps);

    // Create mask based on strongest frequency
    Mat mask = generateFreqMask(this->bpm);

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

void BpmVideoProcessor::createTemporalSpatial() {
    int dstVectorCount = blurredForMask[0].size().width;
    int dstVectorWidth = (int) blurredForMask.size();
    int dstVectorHeight = blurredForMask[0].size().height;

    for (int i = 0; i < dstVectorCount; i++) {
        // One frame
        Mat frame(dstVectorHeight, dstVectorWidth, CV_32FC1);
        for (int j = 0; j < dstVectorWidth; j++) {
            Mat tmp = blurredForMask[j].clone();
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

void BpmVideoProcessor::inverseTemporalSpatial() {
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

        this->out.push_back(outputFrame.clone());
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

    cropToVideo(video, skinVideo, foreheadRoi);

    for (int i = 0; i < 10; i++) {
        imwrite( (string) PROJECT_DIR+"/images/forehead/forehead"+to_string(i)+".jpg", skinVideo[i] );
        imwrite( (string) PROJECT_DIR+"/images/forehead/head"+to_string(i)+".jpg", video[i] );
    }

}
