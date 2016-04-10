//
// Created by Michal on 26/03/16.
//

#ifndef BPM_BPMVIDEOPROCESSOR_H
#define BPM_BPMVIDEOPROCESSOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/core.hpp>

#include <iostream>
#include <fstream>
#include <boost/thread.hpp>
#include "imageOperation.h"


// Configuration
#include "./config.h"

class BpmVideoProcessor {
    // Input face video - should be const
    vector<Mat> video;

    // Try to extract more sking to this
    vector<Mat> forehead;

    vector<Mat> out; // Blood circulation visualization
    vector<Mat> blurred; // Blurred video in set level
    vector<Mat> blurredForMask; // Blurred video in set level
    vector<Mat> temporalSpatial; // Temporal spatial

    vector <vector <Mat> > pyramid;
    vector <vector <Mat> > pyramidForehead;

    vector<double> intensities; // Intensities of blurred frames

    // Computed bpm
    int bpm;

    // Lower & Higher cutoff
    float fl, fh;

    // FPS of input
    int fps;

    // Number of frames in face buffer
    int framesCount;

    // Level of double decreasing size of each frame
    // For mask computation
    int level;

    // How wide freq (bpm) range should we keep
    int maskWidth;

    public:
        BpmVideoProcessor(vector<Mat> video, float fl, float fh, int level, int fps, int framesCount);
        void compute();

        void buildGDownPyramid(vector<Mat> &src, vector<vector <Mat> > &pyramid, int level);

        // Used for generating beating mask
        void amplifyFrequencyInPyramid(vector<vector <Mat> > &pyramid, vector<Mat> &temporalSpatial, vector<Mat> &dst, float bpm);
        void amplifyFrequencyInLevel(vector<Mat> src, vector<Mat> &temporalSpatial, vector<Mat> &dst, float bpm);

        void createTemporalSpatial(vector<Mat> src, vector<Mat>& temporalSpatial);
        void bandpass(vector<Mat>& temporalSpatial, float freq);
        void inverseTemporalSpatial(vector<Mat>& temporalSpatial, vector<Mat>& dst);

        // TODO: rename according to numpy
        Mat generateFreqMask(float freq);
        static void amplifyVideoChannels(vector<Mat>& video, float r, float g, float b);
        void getForeheadSkinArea();

        const vector<Mat> &getOut() const {
            return out;
        }

        const vector<Mat> &getTemporalSpatialMask() const {
            return temporalSpatial;
        }

        int getBpm() const {
            return bpm;
        }


        static void buildGDownPyramidLevel(vector<Mat> &src, vector<Mat> &dst, int currLevel) {
            for (int i = 0; i < src.size(); i++) {
                // 0 Level only copy
                if (currLevel == 0) {
                    src[i].convertTo(src[i], CV_32FC3);
                    dst.push_back(src[i]);
                    continue;
                }

                if (src[i].type() == CV_32FC3) {
                    src[i].convertTo(src[i], CV_8UC3);
                }

                cvtColor2(src[i], src[i], CV2_BGR2YIQ); // returns CV_8UC3
                src[i].convertTo(src[i], CV_32FC3);

                pyrDown(src[i], src[i]);

                src[i].convertTo(src[i], CV_8UC3);
                cvtColor2(src[i], src[i], CV2_YIQ2BGR); // returns CV_8UC3
                src[i].convertTo(src[i], CV_32FC3);
                dst.push_back(src[i]);
            }
        }

};

class PyramidLevelWorker {
    vector<Mat> dst;
    int part;

    public:

        PyramidLevelWorker(int part) {
            this->part = part;
        }

        void compute(vector<Mat> &src, int currLevel) {
            BpmVideoProcessor::buildGDownPyramidLevel(src, dst, currLevel);
            int i = 0;
        }

         vector<Mat> &getDst() {
            return dst;
        }
};

#endif //BPM_BPMVIDEOPROCESSOR_H
