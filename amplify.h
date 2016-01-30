//
// Created by Michal on 30/01/16.
//

#ifndef BPM_AMPLIFY_H
#define BPM_AMPLIFY_H

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

void amplifySpatial(Mat video[], double alpha, int lowLimit, int highLimit, int frameRate);
Mat* buildGDownStack(Mat video[], int framesCount, int level);
Mat blurDn(Mat frame, int level);
void bandpass(Mat video[], int lowLimit, int highLimit, int videoRate, int framesCount);

#endif //BPM_AMPLIFY_H
