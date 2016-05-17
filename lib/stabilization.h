#ifndef BPM_STABILIZATION_H
#define BPM_STABILIZATION_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>

using namespace std;
using namespace cv;

// Slightly improved version from http://nghiaho.com/?p=2093
int stabilizeVideo(vector<Mat>& src);

#endif //BPM_STABILIZATION_H
