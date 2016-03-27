//
// Created by Michal on 26/03/16.
//

#include "BpmVideoProcessor.h"


BpmVideoProcessor::BpmVideoProcessor(vector<Mat> video, float fl, float fh, int level, int fps, int framesCount) {
    this->video = video;
    this->fl = fl;
    this->fh = fh;
    // TODO: Level will be dynamic
    this->level = level;
    this->fps = fps;
    this->framesCount = framesCount;
}

void BpmVideoProcessor::compute() {
    buildGDownStack();
    bandpass();
    bpm = findStrongestRowFreq(countIntensities(out));
}

void BpmVideoProcessor::buildGDownStack() {
    Mat kernel = binom5Kernel();
    for (int i = 0; i < framesCount; i++) {
        Mat out = video[i].clone();

        // TODO: REWRITE ctColor2 to float
        cvtColor2(out, out, CV2_BGR2YIQ); // returns CV_8UC3

        // TODO: This solves rounding to int at first and than back to float
        out.convertTo(out, CV_32FC3);

        blurDn(out, level, kernel);

        out.convertTo(out, CV_8UC3);

        cvtColor2(out, out, CV2_YIQ2BGR); // returns CV_8UC3

        out.convertTo(out, CV_32FC3);

        temporalSpatialMask.push_back(out);

    }
    kernel.release();
}

void BpmVideoProcessor::bandpass() {
    // TODO: Describe
    int height =  video[0].size().height;
    int width =  video[0].size().width;

    vector <vector<Mat> > temporalSpatialStack(3);

    // Create temporal spatial video
    createTemporalSpatial(temporalSpatialStack);

    // First of all we need to find strongest frequency for all
    float strongestTimeStackFreq = findStrongestRowFreq(countIntensities(video));

    // Create mask based on strongest frequency
    Mat mask = maskingCoeffs(video.size(),  strongestTimeStackFreq-15, strongestTimeStackFreq+15);

    for (int i = 0; i < temporalSpatialStack[0].size(); i++) {
        for (int channel = 0; channel < 3; channel++) {
            for (int row = 0; row < temporalSpatialStack[channel][i].rows; row++) {

                // FFT
                Mat fourierTransform;
                dft(temporalSpatialStack[channel][i].row(row), fourierTransform, cv::DFT_SCALE | cv::DFT_COMPLEX_OUTPUT);

                // MASKING
                Mat planes[] = {Mat::zeros(fourierTransform.size(), CV_32F), Mat::zeros(fourierTransform.size(), CV_32F)};

                // Real & imag part
                split(fourierTransform, planes);

                // Masking parts
                planes[0] = planes[0].mul(mask);
                planes[1] = planes[1].mul(mask);


                // Stretch
//                normalize(planes[0], planes[0], 0, 255);

                // Merge back
                merge(planes, 2, fourierTransform);

                // IFFT
                dft(fourierTransform, fourierTransform, cv::DFT_INVERSE|cv::DFT_REAL_OUTPUT);

                normalize(fourierTransform, fourierTransform, 0, 1, NORM_MINMAX);

                // COPY BACK
                fourierTransform.copyTo(temporalSpatialStack[channel][i].row(row));

                // RELEASE
                fourierTransform.release();
            }
        }
    }
    bpm = round(strongestTimeStackFreq);
    inverseTemporalSpatial(temporalSpatialStack, out);
}

void BpmVideoProcessor::createTemporalSpatial(vector <vector<Mat> > & temporalSpatialStack) {
    createTemporalSpatial(temporalSpatialStack, RED_CHANNEL);
    createTemporalSpatial(temporalSpatialStack, GREEN_CHANNEL);
    createTemporalSpatial(temporalSpatialStack, BLUE_CHANNEL);
}

void BpmVideoProcessor::createTemporalSpatial(vector <vector<Mat> > & temporalSpatialStack, int channel) {

    // DST vector
    // video[0].size().width - vectors count
    // width: video.size()
    // height video[0].size().height
    int dstVectorCount = temporalSpatialMask[0].size().width;
    int dstVectorWidth = (int) temporalSpatialMask.size();
    int dstVectorHeight = temporalSpatialMask[0].size().height;

    for (int i = 0; i < dstVectorCount; i++) {
        // One frame
        Mat frame(dstVectorHeight, dstVectorWidth, CV_32FC1);
        for (int j = 0; j < dstVectorWidth; j++) {
            vector<Mat> channels;
            temporalSpatialMask[j].convertTo(temporalSpatialMask[j], CV_32FC3);
            split(temporalSpatialMask[j],channels);

            for(int k = 0; k < dstVectorHeight; k++) {
                frame.at<float>(k,j) = channels[channel].at<float>(k,i);
            }
        }
        temporalSpatialStack[channel].push_back(frame);
        frame.release();
    }
}
