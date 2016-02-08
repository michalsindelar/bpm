//
// Created by Michal on 30/01/16.
//

#ifndef BPM_AMPLIFY_H
#define BPM_AMPLIFY_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/core.hpp>

using namespace cv;
using namespace std;

void amplifySpatial(vector<Mat>& video, vector<Mat>& out, double alpha, int lowLimit, int highLimit, int videoRate, int framesCount, int level);
void buildGDownStack(vector<Mat>& video, vector<Mat>& stack, int framesCount, int level);
Mat blurDn(Mat frame, int level, Mat kernel);
void bandpass(vector<Mat>& video, vector<Mat>& filtered, int lowLimit, int highLimit, int videoRate, int framesCount);
Mat binom5Kernel();

#endif //BPM_AMPLIFY_H
