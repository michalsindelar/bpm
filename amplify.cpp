//
// Created by Michal on 30/01/16.
//

#include "amplify.h"
#include "imageOperation.h"


#define BLUE_CHANNEL 0
#define GREEN_CHANNEL 1
#define RED_CHANNEL 2

void amplifySpatial(vector<Mat>& video, vector<Mat>& out, double alpha, int lowLimit, int highLimit, int videoRate, int framesCount, int level) {

    // Allocate stack
    vector<Mat> stack;

    // Create gdown stack
    buildGDownStack(video, stack, framesCount, level);

    // Filtering
    bandpass(stack, out, lowLimit, highLimit, videoRate, framesCount);

    // Clear data
    stack.clear();
}

// Based on https://github.com/diego898/matlab-utils/blob/master/toolbox/EVM_Matlab/build_GDown_stack.m
// TODO: Check wheter rgb separate channels needed
void buildGDownStack(vector<Mat>& video, vector<Mat>& stack, int framesCount, int level) {
    Mat kernel = binom5Kernel();
    for (int i = 0; i < framesCount; i++) {
        // Result image push to stack
        Mat tmp = blurDn(video[i], level, kernel);
        stack.push_back(tmp.clone());
        tmp.release();
    }
    kernel.release();
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

    // FLoat at first
    cvtColor(frame, frame, CV_32F);

    // Convert to hsv (similar as ntsc)
    cvtColor(frame, frame, CV_BGR2HSV);

    // blur via binomial filter
    filter2D(frame, frame, -1, kernel);

    // Convert back
    cvtColor(frame, frame, CV_HSV2BGR);
    cvtColor(frame, frame, CV_32F);

    return frame;
}

void shift(Mat magI) {

    // crop if it has an odd number of rows or columns
    magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));

    int cx = magI.cols/2;
    int cy = magI.rows/2;

    Mat q0(magI, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
    Mat q1(magI, Rect(cx, 0, cx, cy));  // Top-Right
    Mat q2(magI, Rect(0, cy, cx, cy));  // Bottom-Left
    Mat q3(magI, Rect(cx, cy, cx, cy)); // Bottom-Right

    Mat tmp;                            // swap quadrants (Top-Left with Bottom-Right)
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    q1.copyTo(tmp);                     // swap quadrant (Top-Right with Bottom-Left)
    q2.copyTo(q1);
    tmp.copyTo(q2);
}

void bandpass(vector<Mat>& video, vector<Mat>& filtered, int lowLimit, int highLimit, int videoRate, int framesCount) {
    // TODO: Describe
    int height =  video[0].size().height;
    int width =  video[0].size().width;

    // TODO: Connect with main class
    int fps = 30;
    int fl = 60/50; // Low freq cut-off
    int fh = 160/60; // High freg cut-off

    // Prepare freq.

    // Create mask
    // http://vgg.fiit.stuba.sk/2012-05/frequency-domain-filtration/
    Mat kernel = maskKernel(width, height, video.size(), fps, fl, fh);
    shift(kernel);
    Mat kernelPlanes[] = {
        Mat::zeros(video[0].size(), CV_32F),
        Mat::zeros(video[0].size(), CV_32F)
    };
    Mat kernelSpec;
    kernelPlanes[0] = kernel; // real
    kernelPlanes[1] = kernel; // imaginar
    merge(kernelPlanes, 2, kernelSpec);

    float amplCoeffs[] = {50*0.2f, 50*0.2f, 50};

    for (int i = 0; i < video.size(); i++) {
        // Split into channels
        vector<Mat> channels;
        split(video[i],channels);

        // Convert to desired type
        // Multi channel img
        for (int j = 0; j < 3; j++) {

            // Should be each channel separate
            // Planes only for dft purposes
            Mat planes[] = {Mat_<float>(channels[j]), Mat::zeros(channels[j].size(), CV_32F)};

            Mat complexI;
            merge(planes, 2, complexI);
            dft(complexI, complexI);  // Applying DFT

            // Masking
            mulSpectrums(complexI, kernelSpec, complexI, DFT_ROWS);

            // Amplification
            //complexI = complexI*amplCoeffs[j];


            // Reconstructing original imae from the DFT coefficients
            Mat tmp;
            idft(complexI, tmp, DFT_SCALE | DFT_REAL_OUTPUT ); // Applying IDFT
            tmp.convertTo(channels[j], CV_8U);
        }

        // Merge rgb back
        Mat tmp;
        merge(channels, tmp);
        tmp.convertTo(tmp, CV_8U);
        filtered.push_back(tmp);

        tmp.release();
        channels.clear();

        // Amplification
        //filtered[i].mul(filtered[i], 50);
    }
    kernel.release();
}

// Assume video is single channel
void createTimeChangeStack(vector<Mat>& video, vector <vector<Mat> >& dst, int channel) {

    // DST vector
    // video[0].size().width - vectors count
    // width: video.size()
    // height video[0].size().height
    int dstVectorCount = video[0].size().width;
    int dstVectorWidth = (int) video.size();
    int dstVectorHeight = video[0].size().height;

    for (int i = 0; i < dstVectorCount; i++) {
        // One frame
        Mat frame(dstVectorHeight, dstVectorWidth, CV_32F);
        for (int j = 0; j < dstVectorWidth; j++) {
            for(int k = 0; k < dstVectorHeight; k++) {

                // Split into channel and take the desired one
                // TODO: Optimalization split video outside loop into channels
                vector<Mat> channels;
                split(video[j],channels);

                // because y,x indexation -> k, i
                frame.at<float>(k,j) = channels[channel].at<float>(k, i);
            }
        }
        dst[channel].push_back(frame);
        frame.release();
    }
}

void inverseCreateTimeChangeStack(vector<Mat>& stack, vector<Mat>& dst) {

    int dstCount = stack[0].size().width;
    int dstWidth = (int) stack.size();
    int dstHeight = stack[0].size().height;

    for (int i = 0; i < dstCount; i++) {
        // One frame
        Mat frame(dstHeight, dstWidth, CV_32F);

        for (int j = 0; j < dstWidth; j++) {
            for(int k = 0; k < dstHeight; k++) {
                // because y,x indexation -> k, i
                //frame.at<float>() = dst[j].at<float>(k, i);
            }
        }
        dst.push_back(frame);
        frame.release();
    }

}

Mat maskKernel(int width, int height, int videoSize, int fps, int fl, int fh) {
    Mat kernel;

    // Create row 0.25 - 0.5 ----- 30.0
    Mat col(height, 1, CV_32F);
    for (int i = 1; i < height; i++) {
        float value = (i-1)/( (float) videoSize)* (float) fps;
        // We want to mask freq out of [fl, fh]
        if (value < fl || value > fh) {
            value = 0;
        }
        col.at<float>(i-1, 0) = value;
    }
    repeat(col, 1, width, kernel);

    return kernel;
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
    kernel.at<float>(0,0) = 1.0f / 256.0f;
    kernel.at<float>(1,0) = 4.0f / 256.0f;
    kernel.at<float>(2,0) = 6.0f / 256.0f;
    kernel.at<float>(3,0) = 4.0f / 256.0f;
    kernel.at<float>(4,0) = 1.0f / 256.0f;

    // 2nd row
    kernel.at<float>(0,1) = 4.0f / 256.0f;
    kernel.at<float>(1,1) = 16.0f / 256.0f;
    kernel.at<float>(2,1) = 24.0f / 256.0f;
    kernel.at<float>(3,1) = 16.0f / 256.0f;
    kernel.at<float>(4,1) = 4.0f / 256.0f;

    // 3rd row
    kernel.at<float>(0,2) = 6.0f / 256.0f;
    kernel.at<float>(1,2) = 24.0f / 256.0f;
    kernel.at<float>(2,2) = 36.0f / 256.0f;
    kernel.at<float>(3,2) = 24.0f / 256.0f;
    kernel.at<float>(4,2) = 6.0f / 256.0f;


    // 4th row
    kernel.at<float>(0,3) = 4.0f / 256.0f;
    kernel.at<float>(1,3) = 16.0f / 256.0f;
    kernel.at<float>(2,3) = 24.0f / 256.0f;
    kernel.at<float>(3,3) = 16.0f / 256.0f;
    kernel.at<float>(4,3) = 4.0f / 256.0f;

    // 5th row
    kernel.at<float>(0,4) = 1.0f / 256.0f;
    kernel.at<float>(1,4) = 4.0f / 256.0f;
    kernel.at<float>(2,4) = 6.0f / 256.0f;
    kernel.at<float>(3,4) = 4.0f / 256.0f;
    kernel.at<float>(4,4) = 1.0f / 256.0f;

    return kernel;
}