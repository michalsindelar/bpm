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
    this->bpm = (int) round(findStrongestRowFreq(countIntensities(blurred), framesCount, fps));
    createTemporalSpatial(); // Create temporal spatial video
    bandpass();
    inverseTemporalSpatial();
    
    // Here will be probably second iteration
}

void BpmVideoProcessor::buildGDownStack() {
    Mat kernel = binom5Kernel();
    for (int i = 0; i < framesCount; i++) {
        Mat frame = video[i].clone();

        // TODO: REWRITE ctColor2 to float
        cvtColor2(frame, frame, CV2_BGR2YIQ); // returns CV_8UC3

        // TODO: This solves rounding to int at first and than back to float
        frame.convertTo(frame, CV_32FC3);

        blurDn(frame, level, kernel);

        frame.convertTo(frame, CV_8UC3);

        cvtColor2(frame, frame, CV2_YIQ2BGR); // returns CV_8UC3

        frame.convertTo(frame, CV_32FC3);

        blurred.push_back(frame);
    }

    kernel.release();
}

void BpmVideoProcessor::bandpass() {
    // TODO: Describe
    int height =  blurred[0].size().height;
    int width =  blurred[0].size().width;


    

    // First of all we need to find strongest frequency for all
    // TODO: This is only initial compute of strongest freq
    float strongestTimeStackFreq = findStrongestRowFreq(countIntensities(blurred), framesCount, fps);

    // Create mask based on strongest frequency
    Mat mask = maskingCoeffs(framesCount,  strongestTimeStackFreq-15, strongestTimeStackFreq+15, fps);

    for (int i = 0; i < temporalSpatialMask.size(); i++) {
        
        for (int row = 0; row < temporalSpatialMask[i].rows; row++) {

            // FFT
            Mat fourierTransform;
            dft(temporalSpatialMask[i].row(row), fourierTransform, cv::DFT_SCALE | cv::DFT_COMPLEX_OUTPUT);

            // MASKING
            Mat planes[] = {Mat::zeros(fourierTransform.size(), CV_32F), Mat::zeros(fourierTransform.size(), CV_32F)};

            // Real & imag part
            split(fourierTransform, planes);

            // Masking parts
//                planes[0] = planes[0].mul(mask);
//                planes[1] = planes[1].mul(mask);


            // Stretch
//                normalize(planes[0], planes[0], 0, 255);

            // Merge back
            merge(planes, 2, fourierTransform);

            // IFFT
            dft(fourierTransform, fourierTransform, cv::DFT_INVERSE|cv::DFT_REAL_OUTPUT);

//                normalize(fourierTransform, fourierTransform, 0, 1, NORM_MINMAX);

            // COPY BACK
            fourierTransform.copyTo(temporalSpatialMask[i].row(row));

            // RELEASE
            fourierTransform.release();
        }
        
    }
    bpm = round(strongestTimeStackFreq);
}

void BpmVideoProcessor::createTemporalSpatial() {
    int dstVectorCount = blurred[0].size().width;
    int dstVectorWidth = (int) blurred.size();
    int dstVectorHeight = blurred[0].size().height;

    for (int i = 0; i < dstVectorCount; i++) {
        // One frame
        Mat frame(dstVectorHeight, dstVectorWidth, CV_32FC1);
        for (int j = 0; j < dstVectorWidth; j++) {
            Mat tmp;
            cvtColor(blurred[j], tmp, CV_BGR2GRAY);
            tmp.convertTo(tmp, CV_32F);

            for(int k = 0; k < dstVectorHeight; k++) {
                frame.at<float>(k,j) = tmp.at<float>(k,i);
            }
        }
        temporalSpatialMask.push_back(frame.clone());
        frame.release();
    }
}

void BpmVideoProcessor::inverseTemporalSpatial() {
    int dstCount = temporalSpatialMask[0].cols;
    int dstWidth = (int) temporalSpatialMask.size();
    int dstHeight = temporalSpatialMask[0].rows;

    for (int i = 0; i < dstCount; i++) {
        Mat outputFrame = Mat(dstHeight, dstWidth, CV_32FC1);

        for (int j = 0; j < dstWidth; j++) {
            for(int k = 0; k < dstHeight; k++) {
                    outputFrame.at<float>(k, j) = temporalSpatialMask[j].at<float>(k,i);
            }
        }

        // Amplify frame's channels
//        amplifyChannels(channels, 2, 1, 1);

        // Convert to basic CV_8UC3
//        cvtColor(outputFrame, outputFrame, );
        cvtColor(outputFrame, outputFrame, CV_GRAY2BGR);
        outputFrame.convertTo(outputFrame, CV_8U);

        // in range [0,255]
        normalize(outputFrame, outputFrame, 0, 255, NORM_MINMAX );

        this->out.push_back(outputFrame.clone());
        outputFrame.release();
    }
}

