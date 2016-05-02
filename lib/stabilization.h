#ifndef BPM_STABILIZATION_H
#define BPM_STABILIZATION_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>

using namespace std;
using namespace cv;


int stabilizeVideo(vector<Mat>& src);

#endif //BPM_STABILIZATION_H
