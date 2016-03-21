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

using namespace cv;
using namespace std;

Mat resizeImage (Mat image, const double width);
bool compareColorAndBwMatrix(Mat color, Mat bw);
void adjustOutput (Mat image);
void fakeBeating (Mat image, double index, int maxValue, Rect face);
void controlFacePlacement (Rect & roi, const Size frame);
Mat cropImageBorder (Mat image, int borderWidth);
void cvtColor2(Mat src, Mat & dst, int code);
#endif