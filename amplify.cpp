//
// Created by Michal on 30/01/16.
//

#include "amplify.h"

void amplifySpatial(vector<Mat>& video, vector<Mat>& out, double alpha, int lowLimit, int highLimit, int videoRate, int framesCount, int level) {

    vector<Mat> stack;
    // Create gdown stack
    buildGDownStack(video, stack, framesCount, level);
    // Filtering
    bandpass(stack, out, lowLimit, highLimit, videoRate, framesCount);
}

// Based on https://github.com/diego898/matlab-utils/blob/master/toolbox/EVM_Matlab/build_GDown_stack.m
// TODO: Check wheter rgb separate channels needed
void buildGDownStack(vector<Mat>& video, vector<Mat>& stack, int framesCount, int level) {
    Mat kernel = binom5Kernel();
    for (int i = 0; i < framesCount; i++) {
        // TODO: Should be in ntsc
        // Split into channels
        vector<Mat> channels;
        Mat tmp;

        split(video[i], channels);
        channels[0] = blurDn(channels[0], level, kernel);
        channels[1] = blurDn(channels[1], level, kernel);
        channels[2] = blurDn(channels[2], level, kernel);

        merge(channels, tmp);
        stack.push_back(tmp);
    }
}

/**
 * Downsample 2x and blur image in pyramid level
 * TODO: Blurring should be via  binomial filter see link
 * http://optica.csic.es/projects/tools/steer/1.matlabPyrTools/namedFilter.html

 * Each chanel separate
 */
Mat blurDn(Mat frame, int level, Mat kernel) {
    if (level == 1) return frame;
    if (level > 1) frame = blurDn(frame, level-1, kernel);

    // resize 1/2
    resize(frame, frame, Size(frame.size().width / 2, frame.size().height / 2), 0, 0, INTER_LINEAR);

    // blur via binomial filter
    filter2D(frame, frame, -1, kernel);

    return frame;
}

void bandpass(vector<Mat>& video, vector<Mat>& filtered, int lowLimit, int highLimit, int videoRate, int framesCount) {
    // TODO: Describe
    int height =  video[0].size().height;
    int width =  video[0].size().width;

    // Prepare freq.
    int freq[height];
    int tmp[height];
    Mat mask;

    for (int i = 0; i < height; i++) {
        freq[i] = (i*height)/videoRate;
        tmp[i] = (freq[i] > lowLimit && freq[i] < highLimit) ? 1 : 0;
    }

    //repeat((const _InputArray &) tmp, 1, width * framesCount, mask);

    for (int i = 0; i < framesCount; i++) {
        // Split into channels
        vector<Mat> channels;
        split(video[i],channels);

        // Convert to desired type

        int height = video[i].rows;
        int width = video[i].cols;

        // Multi channel img
        for (int j = 0; j < 1; j++) {

            // Should be each channel separate
            // Planes only for dft purposes
            Mat planes[] = {Mat_<float>(channels[j]), Mat::zeros(channels[j].size(), CV_32F)};

            Mat complexI;
            merge(planes, 2, complexI);
            dft(complexI, complexI);  // Applying DFT

            // Here will be masking (!)

            // Reconstructing original imae from the DFT coefficients
            Mat invDFT, invDFTcvt;
            idft(complexI, invDFT, DFT_SCALE | DFT_REAL_OUTPUT ); // Applying IDFT
            invDFT.convertTo(invDFTcvt, CV_8U);
        }

        // Merge rgb back
        Mat tmp;
        merge(channels, tmp);
        filtered.push_back(tmp);

        // Amplification
        //filtered[i].mul(filtered[i], 50);
    }
}
/**
* BINOMIAL 5 - kernel
* 1 4 6 4 1
* 4 16 24 16 4
* 6 24 36 24 6
* 4 16 24 16 4
* 1 4 6 4 1
*/
Mat binom5Kernel() {
    Mat kernel(5, 5, CV_32F);

    // 1st row
    kernel.at<float>(0,0) = 1.0f;
    kernel.at<float>(1,0) = 4.0f;
    kernel.at<float>(3,0) = 6.0f;
    kernel.at<float>(4,0) = 4.0f;
    kernel.at<float>(5,0) = 1.0f;

    // 2nd row
    kernel.at<float>(0,1) = 4.0f;
    kernel.at<float>(1,1) = 16.0f;
    kernel.at<float>(3,1) = 24.0f;
    kernel.at<float>(4,1) = 16.0f;
    kernel.at<float>(5,1) = 4.0f;

    // 3rd row
    kernel.at<float>(0,2) = 6.0f;
    kernel.at<float>(1,2) = 24.0f;
    kernel.at<float>(3,2) = 36.0f;
    kernel.at<float>(4,2) = 24.0f;
    kernel.at<float>(5,2) = 6.0f;


    // 4th row
    kernel.at<float>(0,1) = 4.0f;
    kernel.at<float>(1,1) = 16.0f;
    kernel.at<float>(3,1) = 24.0f;
    kernel.at<float>(4,1) = 16.0f;
    kernel.at<float>(5,1) = 4.0f;

    // 5th row
    kernel.at<float>(0,4) = 1.0f;
    kernel.at<float>(1,4) = 4.0f;
    kernel.at<float>(3,4) = 6.0f;
    kernel.at<float>(4,4) = 4.0f;
    kernel.at<float>(5,4) = 1.0f;

    normalize(kernel, kernel);
    return kernel;
}