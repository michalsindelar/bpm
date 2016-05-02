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
#include "constants.h"
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
int setDoubleDownscalingLevel(int origWidth, int resultWidth);

// Blurring
void pyrUpVideo(vector<Mat>& video, Size size, int level);
void pyrUpVideo(vector<Mat> &video, vector<Mat> &dst, Size size, int level);
Mat binom5Kernel();
void blurDn(Mat & frame, int level, Mat kernel);

// Cropping
void resizeCropVideo(vector<Mat>& video, int width);
void cropToVideo(vector<Mat> src, vector<Mat>& dst, int width);
void cropToVideo(vector<Mat> src, vector<Mat>& dst, Rect roi);
Mat cropImageBorder (Mat image, int borderWidth);
void fillRoiInVideo(vector<Mat> src, vector<Mat> & dst, Rect roi, Scalar color);

// Unify mats
void unifyMatSize(Mat & frame, Size unifiedSize);

// Range control
void handleRoiPlacement(Rect &roi, const Size frame);
void handleRoiPlacement(Rect &roi, const Size frame, int erasedBorder);

// Deprecated
bool compareColorAndBwMatrix(Mat color, Mat bw);
void adjustOutput (Mat image);
void fakeBeating (Mat image, double index, int maxValue, Rect face);

// Colors handling
void cvtColor2(Mat src, Mat & dst, int code);
void amplifyChannels(Mat& frame, float r, float g, float b);
void normalizeVid(vector<Mat>& video, int min, int max, int type);

// Frequency tools helpers
float freqToBpmMapper(double fps, int framesCount, int index);
float findStrongestRowFreq(vector<double> row, int framesCount, double fps);
float findStrongestRowFreq(Mat row, int framesCount, double fps);

// TODO: rename according to numpy
Mat generateFreqMask(float freq, int framesCount, double fps);
Mat maskingCoeffs(int width, float fl, float fh, double fps);

// Intensities compute
vector<double> countIntensities(vector<Mat> video);
vector<double> countIntensities(vector<Mat> video, float r, float g, float b);
vector<double> countMeanValues(vector<Mat> video, int channel = GREEN_CHANNEL);
void saveIntensities(vector<double> intensities, string filename);
void generateTemporalSpatialImages(vector <vector<Mat> > temporalSpatialStack);
void suppressGlobalChanges(vector<double>& localIntensities, vector<double> globalIntensities);
vector<double> countOutsideIntensities(vector<Mat> video, Rect face, float r, float g, float b, int mode);


// Printing data
void printIterationRow(vector<Mat> video, int framesCount, double fps, int realBpm, ofstream &file);
void printIterationHead(ofstream &file);


Point2d getCenter(Size size);
double getDistance(Point2d a, Point2d b);

// Visualize
void printRectOnFrame(Mat &frame, Rect rect, Scalar color);

#endif