#ifndef IMAGE_OPEARATION_H
#define IMAGE_OPEARATION_H

#include <iostream>
#include <ctime>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

Mat resizeImage (Mat image, const double width);
void adjustOutput (Mat image);
void fakeBeating (Mat image, double index, int maxValue);

#endif