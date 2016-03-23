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
#include "imageOperation.h"

// Configuration
#include "./config.h"


using namespace cv;
using namespace std;

// CORE
void amplifySpatial(const vector<Mat> video, vector<Mat>& out, int & bpm, double alpha, int lowLimit, int highLimit, int framesCount, int level);
void buildGDownStack(const vector<Mat> video, vector<Mat>& stack, int framesCount, int level);
void blurDn(Mat & frame, int level, Mat kernel);
void bandpass(vector<Mat>& video, vector<Mat>&out, int & bpm, int lowLimit, int highLimit, int framesCount);

// DATA
vector<int> countIntensities(vector<Mat>& video);
void saveIntensities(vector<Mat>& video, string filename);

// TEMPORAL SPATIAL
void createTemporalSpatial(vector<Mat> &video, vector<vector<Mat> > &dst, int channel);
void inverseTemporalSpatial(vector<vector<Mat> > &stack, vector<Mat> &dst);

// KERNELS / MASKS
Mat binom5Kernel();
Mat maskingCoeffs(int width, float fl, float fh);

// FOURIER
Mat computeDFT(Mat image);
Mat updateResult(Mat complex);
void updateMag(Mat complex );

// FREQUENCY
float findStrongestTimeStackFreq(vector <vector<Mat> > timeStack);

float findStrongestRowFreq(Mat row);
float findStrongestRowFreq(vector<int> row);

// IMAGE OPERATION
void resizeCropVideo(vector<Mat>& video, int width);
void amplifyChannels(vector<Mat>& channels, int r, int g, int b);

#endif //BPM_AMPLIFY_H
