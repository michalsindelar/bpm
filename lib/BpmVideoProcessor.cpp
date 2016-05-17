//
// Created by Michal on 26/03/16.
//

#include "BpmVideoProcessor.h"

BpmVideoProcessor::BpmVideoProcessor(vector<Mat> video, float fl, float fh, int level, double fps, int framesCount, Rect faceRoi) {

    this->origVideo = video;
    cropToVideo(video, this->faceVideo, faceRoi);

    this->fl = fl;
    this->fh = fh;
    this->level = level;
    this->fps = fps;
    this->framesCount = framesCount;
    this->maskWidth = FREQ_MASK_WIDTH;
    this->faceRoi = faceRoi;

    // Detect forehead
    this->getForeheadSkinArea();


}


void BpmVideoProcessor::computeAmplifiedMask() {

    // Set pyr down resizing
    // Set closest pyr down level value to ideal frame width for amplification
    // TODO: Control if video is big enough -> in bpm.cpp
    this->doubleDownscalingLevel = setDoubleDownscalingLevel(this->faceVideo[0].cols, IDEAL_WIDTH_FOR_AMPLIFICATION);

    // Init settings of amplification
    setMaxPyramidLevel();

    vector<boost::thread *> z;
    vector <ThreadWorker> workerParts;

    int framesInPart = min(FRAMES_FOR_VISUALIZATION, (int) faceVideo.size());
    int parts = (int) ceil(faceVideo.size() / framesInPart);

    // Initialize workers
    for (int i = 0; i < parts; i++) {
        workerParts.push_back(ThreadWorker());
    }

    // Start to compute in threads
    for (int i = 0; i < parts; i++) {
        int start = i * framesInPart;
        int end = start + framesInPart;

        z.push_back(new boost::thread(&ThreadWorker::amplifyVideo, boost::ref(workerParts[i]),
                                      vector<Mat>(faceVideo.begin() + start, faceVideo.begin() + end),
                                      doubleDownscalingLevel, level, bpm, fps));
    }

    // Wait for threads
    for (int i = 0; i < parts; i++) {
        z[i]->join();
        delete z[i];
    }

    // Copy thread parts to pyramid
    for (int i = 0; i < parts; i++) {
        vector <Mat> tmp = workerParts[i].getResult();
        for (int j = 0; j < tmp.size(); j++) {
            out.push_back(tmp[j]);
        }
    }
}

void BpmVideoProcessor::computeBpm(int computeType, string outputFilePath) {

    switch (computeType) {
        case AVG_COMPUTE:
            this->foreheadIntensities = countIntensities(forehead, 0, 1, 0);
            break;
        case MEAN_COMPUTE:
            this->foreheadIntensities = countMeanValues(forehead, GREEN_CHANNEL);
            break;
        // Default mean values
        default:
            this->foreheadIntensities = countIntensities(forehead, 0, 1, 0);
            break;
    }


    this->bpm = (int) round(findStrongestRowFreq(foreheadIntensities, BUFFER_FRAMES, fps));

// Stabilize forehead
//    stabilizeVideo(this->forehead);

// vector <double> globalIntensities = countOutsideIntensities(origVideo, faceRoi, 0, 1, 0, computeType);;
//suppressGlobalChanges(this->foreheadIntensities, globalIntensities);

    int array[] = {200, 250, 300, 350, 375, 400};
    vector<int> values;
    int real = 60;

    float acc = 0.05;
    int fl = (int) round(real - real*acc);
    int fh = (int) round(real + real*acc);


    // Test size
    if (DATA_LOGGING) {
        ofstream myfile;
        myfile.open(outputFilePath+"/sizeTestTable.txt", ios::out);

        int length = BUFFER_FRAMES;
        for(int down = 0; ; down++ ) {

            // Test size >= 0
            Mat tmp = forehead[0];
            bool shouldStop;
            for(int j = 0; j < down; j++) {
                if (tmp.cols / 2 <= 10 || tmp.rows / 2 <= 10) {shouldStop = true;};
                resize(tmp, tmp, Size(tmp.cols / 2, tmp.rows / 2), 0, 0);
            }
            if (shouldStop) break;

            for (int i = 0; i < length; i++) {
                Mat tmp = forehead[i];
                for(int j = 0; j < down; j++) {
                    resize(tmp, tmp, Size(tmp.cols / 2, tmp.rows / 2), 0, 0);
                }

                Scalar color = mean(tmp);
                this->foreheadIntensities.at(i) = color[GREEN_CHANNEL];
            }

            int val = (int) round(findStrongestRowFreq(foreheadIntensities, length, fps));
            myfile << "Size (";
            myfile << tmp.cols;
            myfile << "x";
            myfile << tmp.rows;
            myfile << "), ";
            myfile << val;
            myfile << ", ";

            if (val >= fl && val <= fh) {
                myfile << "0";
            } else {
                float diff = min( abs(fl-val), abs(fh-val) );
                if ( abs(fl-val) < abs(fh-val)) {
                    myfile << (abs(diff / fl) * 100);
                } else {
                    myfile << (abs(diff / fh) * 100);
                }
            }
            myfile << "\n";
        }
    }

    // Test fps
    if (DATA_LOGGING) {

    }

    // Roi testing
    if (DATA_LOGGING) {
        int length = BUFFER_FRAMES;

        ofstream myfile;
        myfile.open(outputFilePath+"/roiTestTable.txt", ios::out);

        // Save image
        imwrite( outputFilePath+"/full.jpg", this->origVideo[100] );
        imwrite( outputFilePath+"/face.jpg", this->faceVideo[100] );
        imwrite( outputFilePath+"/forehead.jpg", this->forehead[100] );


        // Original video
        this->foreheadIntensities = countIntensities(vector<Mat>(origVideo.begin(), origVideo.begin() + length), 0, 1, 0);
        int value = (int) round(findStrongestRowFreq(foreheadIntensities, length, fps));

        myfile << "Full frame value, ";
        myfile << value;
        myfile << "\n";

        // Face roi
        this->foreheadIntensities = countIntensities(vector<Mat>(faceVideo.begin(), faceVideo.begin() + length), 0, 1, 0);
        int fullFrameBpm = (int) round(findStrongestRowFreq(foreheadIntensities, length, fps));
        myfile << "Face value, ";
        myfile << value;
        myfile << "\n";

        // Forehead roi
        this->foreheadIntensities = countIntensities(vector<Mat>(forehead.begin(), forehead.begin() + length), 0, 1, 0);
        value = (int) round(findStrongestRowFreq(foreheadIntensities, length, fps));
        myfile << "Forehead value, ";
        myfile << value;
        myfile << "\n";
    }

    // Lengths and type of counting testing
    if (DATA_LOGGING) {
        // Save image
        imwrite( outputFilePath+"/preview.jpg", this->faceVideo[100] );

        ofstream myfile;
        myfile.open(outputFilePath+"/detectTypeLengthTable.txt", ios::out);

        myfile << "type";
        myfile << ", ";
        for(int length : array) {
            myfile << length;
            myfile << ", ";
        }
        myfile << "\n";


        myfile << "Green median";
        myfile << ", ";
        for(int length : array) {
            this->foreheadIntensities = countMeanValues(vector<Mat>(forehead.begin(), forehead.begin() + length));
            values.push_back((int) round(findStrongestRowFreq(foreheadIntensities, length, fps)));
            myfile << (int) round(findStrongestRowFreq(foreheadIntensities, length, fps));
            myfile << ", ";
        }
        myfile << "\n";

        myfile << "Green median % failure";
        myfile << ", ";
        for (int val : values) {
            if (val >= fl && val <= fh) {
                myfile << "0";
            } else {
                float diff = min( abs(fl-val), abs(fh-val) );
                if ( abs(fl-val) < abs(fh-val)) {
                    myfile << (abs(diff / fl) * 100);
                } else {
                    myfile << (abs(diff / fh) * 100);
                }
            }
            myfile << ", ";
        }
        myfile << "\n";
        values.clear();

        myfile << "Average intensities all";
        myfile << ", ";
        for(int length : array) {
            this->foreheadIntensities = countIntensities(vector<Mat>(forehead.begin(), forehead.begin() + length), 1, 1, 1);
            values.push_back((int) round(findStrongestRowFreq(foreheadIntensities, length, fps)));
            myfile << (int) round(findStrongestRowFreq(foreheadIntensities, length, fps));
            myfile << ", ";
        }
        myfile << "\n";

        myfile << "Average intensities all % failure";
        myfile << ", ";
        for (int val : values) {
            if (val >= fl && val <= fh) {
                myfile << "0";
            } else {
                float diff = min( abs(fl-val), abs(fh-val) );
                if ( abs(fl-val) < abs(fh-val)) {
                    myfile << (abs(diff / fl) * 100);
                } else {
                    myfile << (abs(diff / fh) * 100);
                }
            }
            myfile << ", ";
        }
        myfile << "\n";
        values.clear();

        myfile << "Average intensities green";
        myfile << ", ";
        for(int length : array) {
            this->foreheadIntensities = countIntensities(vector<Mat>(forehead.begin(), forehead.begin() + length), 0, 1, 0);
            values.push_back((int) round(findStrongestRowFreq(foreheadIntensities, length, fps)));
            myfile << (int) round(findStrongestRowFreq(foreheadIntensities, length, fps));
            myfile << ", ";
        }
        myfile << "\n";

        myfile << "Average intensities green % failure";
        myfile << ", ";
        for (int val : values) {
            if (val >= fl && val <= fh) {
                myfile << "0";
            } else {
                float diff = min( abs(fl-val), abs(fh-val) );
                if ( abs(fl-val) < abs(fh-val)) {
                    myfile << (abs(diff / fl) * 100);
                } else {
                    myfile << (abs(diff / fh) * 100);
                }
            }
            myfile << ", ";
        }
        myfile << "\n";
        values.clear();


        myfile.close();
        saveIntensities(this->foreheadIntensities, outputFilePath+"/median-green-intensities.txt");
    }

    // Real app compute should be used for amplification
    this->foreheadIntensities = this->foreheadIntensities = countIntensities(forehead, 0, 1, 0);;
    this->bpm = (int) round(findStrongestRowFreq(foreheadIntensities, 500, fps));

}



void BpmVideoProcessor::setMaxPyramidLevel() {

    // Respect further downscaling of video
    float width = faceVideo[0].cols;
    for (int i = 0; i < doubleDownscalingLevel; i++) {
        width /= 2;
    }

    // We want as many pyramid levels as possible with width bigger than constant
    for (int i = 0; ; i++) {
        width /= 2;
        // Minimal size of frame in pyramid
        if (width <= MIN_WIDTH_IN_PYRAMID) {
            // Update level - needed for upsizing
            this->level = i;
            break;
        }
    }
}

void BpmVideoProcessor::getForeheadSkinArea() {

    // At first we try to detect forehead using eyes detection 10x
    int detected = false;
    // TODO: Check how long does it take
    for (int i = 0; i < faceVideo.size(); i++) {
        if (Detector::detectForeheadFromFaceViaEyesDetection(faceVideo[i], this->foreheadRoi)) {
            detected = true;
            break;
        }
    }

    // Unsuccessful eyes & forehead detection -> default forehead area
    if (!detected) {
        foreheadRoi = Detector::defaultForehead(faceVideo[0]);
    }

    cropToVideo(faceVideo, forehead, foreheadRoi);

    if (true){
        for (int i = 0; i < 10; i++) {
            Mat tmp = faceVideo[i];
            rectangle(tmp, Point(foreheadRoi.x, foreheadRoi.y), Point(foreheadRoi.x + foreheadRoi.width, foreheadRoi.y + foreheadRoi.height), Scalar(255,255,255));
            imwrite( (string) PROJECT_DIR+"/images/forehead/head-sking"+to_string(i)+".jpg", tmp );
            imwrite( (string) PROJECT_DIR+"/images/forehead/forehead"+to_string(i)+".jpg", forehead[i]);
            imwrite( (string) PROJECT_DIR+"/images/forehead/head"+to_string(i)+".jpg", faceVideo[i] );
        }
    }

}

// Currently not used -> same results from only one level == orig video
void BpmVideoProcessor::computeBpmFromPyramid() {
    float bpmSum = 0;
    int bpmLevel = 0;
    for (; bpmLevel < pyramidForehead.size(); bpmLevel++) {
        this->foreheadIntensities = countIntensities(pyramidForehead.at(bpmLevel), 0, 1, 0);
        bpmSum += (int) round(findStrongestRowFreq(foreheadIntensities, framesCount, fps));
    }
    this->bpm = (int) round(bpmSum / bpmLevel);
}