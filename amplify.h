//
// Created by Michal on 30/01/16.
//

#ifndef BPM_AMPLIFY_H
#define BPM_AMPLIFY_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/core.hpp>

#include <iostream>
#include <fstream>

// Configuration
#include "./config.h"

using namespace cv;
using namespace std;

void amplifySpatial(vector<Mat>& video, vector<Mat>& out, double alpha, int lowLimit, int highLimit, int videoRate, int framesCount, int level);
void buildGDownStack(vector<Mat>& video, vector<Mat>& stack, int framesCount, int level);
Mat blurDn(Mat frame, int level, Mat kernel);
void bandpass(vector<Mat>& video, vector<Mat>& filtered, int lowLimit, int highLimit, int videoRate, int framesCount);
Mat binom5Kernel();
void createTimeChangeStack(vector<Mat>& video, vector <vector<Mat> >& dst, int channel);
void inverseCreateTimeChangeStack(vector <vector<Mat> >& stack, vector<Mat>& dst);
Mat maskingCoeffs(int width, int fps, int fl, int fh);
void amplifyChannels(vector<Mat>& channels, int r, int g, int b);
vector<int> countIntensities(vector<Mat>& video);
void saveIntensities(vector<Mat>& video, string filename);
int computeBpm(vector<int> intensitySum);

#endif //BPM_AMPLIFY_H
