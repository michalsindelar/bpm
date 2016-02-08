//
// Created by Michal on 30/01/16.
//

#include "amplify.h"
#include "imageOperation.h"

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

    return frame;
}

void bandpass(vector<Mat>& video, vector<Mat>& filtered, int lowLimit, int highLimit, int videoRate, int framesCount) {
    // TODO: Describe
    int height =  video[0].size().height;
    int width =  video[0].size().width;

    // TODO:
    float fps = 30.0f;

    // Prepare freq.
    int freq[height];
    int tmp[height];
    Mat mask;
    float videoSize = (float) video.size();

    for (int i = 0; i < height; i++) {
        freq[i] = (i*height)/videoRate;
        tmp[i] = (freq[i] > lowLimit && freq[i] < highLimit) ? 1 : 0;
    }

    //repeat((const _InputArray &) tmp, 1, width * framesCount, mask);

    // Create mask
    MatExpr kernel = Mat::zeros(height, width, CV_32F);

    // Create row 0.25 - 0.5 ----- 30.0
    Mat col(1, height, CV_32F);
    for (int i = 1; i < width; i++) {
        float value = (i-1)/videoSize*fps;
        col.at<float>(0,i-1) = value;
    }

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

        tmp.release();
        while (channels.size()) {
            channels.pop_back();
        }

        // Amplification
        //filtered[i].mul(filtered[i], 50);
    }
    mask.release();
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