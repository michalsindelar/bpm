//
// Created by Michal on 30/01/16.
//

#include "amplify.h"

void amplifySpatial(Mat video[], Mat out[], double alpha, int lowLimit, int highLimit, int videoRate, int framesCount, int level) {
    int height = video[0].size().height;
    int width = video[0].size().width;

    int channels = 3;

    // TODO: Vector<Mat> Mat http://answers.opencv.org/question/6054/what-is-the-difference-between-vecmat-and-mat/
    Mat *stack = new Mat[framesCount];

    // Create gdown stack
    buildGDownStack(video, stack, framesCount, level);

    // Filtering
    bandpass(stack, out, lowLimit, highLimit, videoRate, framesCount);

    delete[] stack;
}

// Based on https://github.com/diego898/matlab-utils/blob/master/toolbox/EVM_Matlab/build_GDown_stack.m
// TODO: Check wheter rgb separate channels needed
void buildGDownStack(Mat video[], Mat stack[], int framesCount, int level) {
    int height = video[0].size().height;
    int width = video[0].size().width;
    int channels = 3;

    for (int i = 0; i < framesCount; i++) {
        // TODO: Should be in ntsc
        stack[i] = blurDn(video[i], level);
        //imshow("stack", stack[i]);
    }
}

/**
 * Downsample 2x and blur image in pyramid level
 */
Mat blurDn(Mat frame, int level) {
    if (level == 1) return frame;
    if (level > 1) frame = blurDn(frame, level-1);
    pyrDown(frame, frame, Size((frame.cols+1)/2, (frame.rows+1)/2), BORDER_REFLECT);
    return frame;
}

void bandpass(Mat video[], Mat filtered[], int lowLimit, int highLimit, int videoRate, int framesCount) {
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
            // Fourier
            // channels[j].convertTo(channels[j], CV_32FC1, 1/255.0);

            Mat planes[] = {Mat_<float>(channels[j]), Mat::zeros(channels[j].size(), CV_32F)};
            Mat complexI;
            merge(planes, 2, complexI);
            dft(complexI, complexI);  // Applying DFT

            // Here will be masking (!)
            video[j] = filtered[j];

            // Reconstructing original imae from the DFT coefficients
            Mat invDFT, invDFTcvt;
            idft(complexI, invDFT, DFT_SCALE | DFT_REAL_OUTPUT ); // Applying IDFT
            invDFT.convertTo(invDFTcvt, CV_8U);
        }

        // Merge rgb back
        merge(channels, filtered[i]);

        // Amplification
        filtered[i].mul(filtered[i], 50);
    }
}