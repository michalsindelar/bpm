#ifndef IMAGE_OPEARATION_H
#define IMAGE_OPEARATION_H

#include <iostream>
#include <ctime>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

Mat resizeImage (Mat image, const double width);
bool compareColorAndBwMatrix(Mat color, Mat bw);
void adjustOutput (Mat image);
void fakeBeating (Mat image, double index, int maxValue);
void controlFacePlacement (Rect & roi, const Size frame);
Mat cropImageBorder (Mat image, int borderWidth);

#endif