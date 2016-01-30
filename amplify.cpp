//
// Created by Michal on 30/01/16.
//

#include "amplify.h"

void amplifySpatial(Mat video[], double alpha, int lowLimit, int highLimit, int videoRate, int framesCount, int level) {
    int height = video[0].size().height;
    int width = video[0].size().width;

    int channels = 3;

    // Create gdown stack
    Mat stack[framesCount];
    buildGDownStack(video, stack, framesCount, level);

    // Filtering
    bandpass(stack, lowLimit, highLimit, videoRate, framesCount);
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

void bandpass(Mat video[], int lowLimit, int highLimit, int videoRate, int framesCount) {
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

    repeat((const _InputArray &) tmp, 1, width * framesCount, mask);


    Mat out[framesCount];
    for (int i = 0; i < framesCount; i++) {
        dft(video[i], out[i]);
    }


}