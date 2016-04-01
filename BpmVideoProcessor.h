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
#include "imageOperation.h"

// Configuration
#include "./config.h"

class BpmVideoProcessor {
    // Input face video - should be const
    vector<Mat> video;

    // Try to extract more sking to this
    vector<Mat> skinVideo;

    vector<Mat> out; // Blood circulation visualization
    vector<Mat> blurred; // Blurred video in set level
    vector<Mat> blurredForMask; // Blurred video in set level
    vector<Mat> temporalSpatial; // Temporal spatial

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
    int level;

    // Level of double decreasing size of each frame
    // For mask computation
    int levelForMask;

    // How wide freq (bpm) range should we keep
    int maskWidth;


    public:
        BpmVideoProcessor(vector<Mat> video, float fl, float fh, int level, int fps, int framesCount);
        void compute();

        void buildGDownStack();
        void bandpass();
        void createTemporalSpatial();
        void inverseTemporalSpatial();

        Mat generateFreqMask(float freq);
        static void amplifyVideoChannels(vector<Mat>& video, float r, float g, float b);
        void gainMoreSkinFromFace();

        const vector<Mat> &getOut() const {
            return out;
        }

        const vector<Mat> &getTemporalSpatialMask() const {
            return temporalSpatial;
        }

        int getBpm() const {
            return bpm;
        }
};


#endif //BPM_BPMVIDEOPROCESSOR_H
