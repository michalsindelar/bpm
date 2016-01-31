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


        // Multi channel img
        for (int j = 0; j < 1; j++) {
            // Fourier
            channels[j].convertTo(channels[j], CV_32FC1, 1/255.0);

            imshow("converted", channels[j]);
            dft(channels[j], channels[j], CV_DXT_INVERSE);

            // Apply mask and remove freq. cut off
            // channels[j] = channels[j].mul(mask);

            normalize(channels[j], channels[j], 0, 1, CV_MINMAX);
            // Inverse fourier
            idft(channels[j], channels[j]);

            channels[j].convertTo(channels[j], CV_8U, 255.0);

            imshow("Filtered", channels[j]);
        }

        // Merge rgb back
        merge(channels, filtered[i]);

        // Amplification
        filtered[i].mul(filtered[i], 50);
    }
}