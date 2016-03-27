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
    this->bpm = (int) round(findStrongestRowFreq(countIntensities(out), framesCount, fps));

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

    vector <vector<Mat> > temporalSpatialStack(3);

    // Create temporal spatial video
    createTemporalSpatial(temporalSpatialStack, blurred);

    // First of all we need to find strongest frequency for all
    // TODO: This is only initial compute of strongest freq
    float strongestTimeStackFreq = findStrongestRowFreq(countIntensities(blurred), framesCount, fps);

    // Create mask based on strongest frequency
    Mat mask = maskingCoeffs(framesCount,  strongestTimeStackFreq-15, strongestTimeStackFreq+15, fps);

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
                fourierTransform.copyTo(temporalSpatialStack[channel][i].row(row));

                // RELEASE
                fourierTransform.release();
            }
        }
    }
    bpm = round(strongestTimeStackFreq);
    inverseTemporalSpatial(temporalSpatialStack);

}

void BpmVideoProcessor::createTemporalSpatial(vector <vector<Mat> > & temporalSpatialStack, vector<Mat> source) {
    createTemporalSpatial(temporalSpatialStack, source, RED_CHANNEL);
    createTemporalSpatial(temporalSpatialStack, source, GREEN_CHANNEL);
    createTemporalSpatial(temporalSpatialStack, source, BLUE_CHANNEL);
}

void BpmVideoProcessor::createTemporalSpatial(vector <vector<Mat> > & temporalSpatialStack, vector<Mat> source, int channel) {

    // DST vector
    // video[0].size().width - vectors count
    // width: video.size()
    // height video[0].size().height
    int dstVectorCount = source[0].size().width;
    int dstVectorWidth = (int) source.size();
    int dstVectorHeight = source[0].size().height;

    for (int i = 0; i < dstVectorCount; i++) {
        // One frame
        Mat frame(dstVectorHeight, dstVectorWidth, CV_32FC1);
        for (int j = 0; j < dstVectorWidth; j++) {
            vector<Mat> channels;
            source[j].convertTo(source[j], CV_32FC3);
            split(source[j],channels);

            for(int k = 0; k < dstVectorHeight; k++) {
                frame.at<float>(k,j) = channels[channel].at<float>(k,i);
            }
        }
        temporalSpatialStack[channel].push_back(frame);
        frame.release();
    }
}

void BpmVideoProcessor::inverseTemporalSpatial(vector<vector<Mat> > &temporalSpatialStack) {

    int dstCount = temporalSpatialStack[0][0].size().width;
    int dstWidth = (int) temporalSpatialStack[0].size();
    int dstHeight = temporalSpatialStack[0][0].size().height;

    for (int i = 0; i < dstCount; i++) {

        // Initialize channels
        // TODO: Can't be one line??
        vector<Mat> channels;
        for (int channel = 0; channel < 3; channel++) {
            channels.push_back(Mat(dstHeight, dstWidth, CV_32F));
        }

        for (int j = 0; j < dstWidth; j++) {
            for(int k = 0; k < dstHeight; k++) {
                for (int channel = 0; channel < 3; channel++) {
                    channels[channel].at<float>(k, j) = temporalSpatialStack[channel][j].at<float>(k,i);
                }
            }
        }

        // Amplify frame's channels
//        amplifyChannels(channels, 2, 1, 1);

        // Merge channels into colorFrame
        Mat outputFrame;
        merge(channels, outputFrame);

        // Convert to basic CV_8UC3
        outputFrame.convertTo(outputFrame, CV_8UC3);

//        cvtColor(outputFrame,outputFrame,CV_BGR2GRAY);
//        cvtColor(outputFrame,outputFrame,CV_GRAY2BGR);

        // in range [0,255]
        normalize(outputFrame, outputFrame, 0, 255, NORM_MINMAX );

        out.push_back(outputFrame);
        channels.clear();
        outputFrame.release();
    }
}

