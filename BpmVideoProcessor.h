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

    // Processed temporal spatial mask
    vector<Mat> out;
    vector<Mat> blurred;
    vector<Mat> temporalSpatialMask;

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

    public:
        BpmVideoProcessor(vector<Mat> video, float fl, float fh, int level, int fps, int framesCount);
        void compute();

        void buildGDownStack();
        void bandpass();
        void createTemporalSpatial(vector <vector<Mat> > & temporalSpatialStack, vector<Mat> source);
        void createTemporalSpatial(vector <vector<Mat> > & temporalSpatialStack, vector<Mat> source, int channel);
        void inverseTemporalSpatial(vector <vector<Mat> > & temporalSpatialStack);


        const vector<Mat> &getOut() const {
            return out;
        }

        const vector<Mat> &getTemporalSpatialMask() const {
            return temporalSpatialMask;
        }

        int getBpm() const {
            return bpm;
        }
};


#endif //BPM_BPMVIDEOPROCESSOR_H
