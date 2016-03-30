#ifndef IMAGE_OPEARATION_H
#define IMAGE_OPEARATION_H

#include <iostream>
#include <ctime>
#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include "config.h"
#include <math.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <fstream>

using namespace cv;
using namespace std;

// TODO: clean!!
// Resizing
Mat resizeImage (Mat image, const double width);
Mat resizeImage (Mat image, const double width, int interpolation);
Mat resizeImage (Mat image, const double width);
Size getResizedSize(Size origSize, const double width);
void resizeCropVideo(vector<Mat>& video, int width);
Mat cropImageBorder (Mat image, int borderWidth);

// Range control
void controlFacePlacement (Rect & roi, const Size frame);

bool compareColorAndBwMatrix(Mat color, Mat bw);
void adjustOutput (Mat image);
void fakeBeating (Mat image, double index, int maxValue, Rect face);

// Colors handling
void cvtColor2(Mat src, Mat & dst, int code);
void amplifyChannels(vector<Mat>& channels, int r, int g, int b);

// Frequency tools helpers
float freqToBpmMapper(int fps, int framesCount, int index);
float findStrongestRowFreq(vector<int> row, int framesCount, int fps);
float findStrongestRowFreq(Mat row, int framesCount, int fps);
Mat maskingCoeffs(int width, float fl, float fh, int fps);

// TODO: Extract to blur file?
// Blurring
Mat binom5Kernel();
void blurDn(Mat & frame, int level, Mat kernel);

// Intensities compute
vector<int> countIntensities(vector<Mat> &video);
void saveIntensities(vector<Mat>& video, string filename);
void generateTemporalSpatialImages(vector <vector<Mat> > temporalSpatialStack);

#endif