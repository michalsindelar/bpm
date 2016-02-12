#ifndef SKIN_DETECT_H
#define SKIN_DETECT_H

#include <iostream>
#include <ctime>

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

void detectSkin (Mat image);
vector<Rect> detectFace (Mat image);

#endif