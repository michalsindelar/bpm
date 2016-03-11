//
// Created by Michal on 08/02/16.
//

#include "AmplificationWorker.h"

AmplificationWorker::AmplificationWorker() {
    this->initialFlag = false;
    this->working = false;
};

void AmplificationWorker::compute(vector<Mat> videoBuffer){
    if (this->working) return;

    // Start work!
    this->working = true;
    cout << "Computing bpm";

    // At first fill class buffer with copies!
    this->setVideoBuffer(videoBuffer);

    int ret = 100;

    // TODO: Remove -DEV ONLY
    int fps = 30;

    // Amplify
    amplifySpatial(this->videoBuffer, this->visualization, 50, 50/60, 70/60, 30, int(videoBuffer.size()), 3);
    this->videoBuffer.clear();

    this->bpm = ret;
    this->working = false;
    this->initialFlag = true;
    cout << "Computed bpm in class";
};

int computeBpm() {
//    % Count of intensities
//    intensityCount = length(intensitySum);
//
//    x = 1:intensityCount;
//    y = intensitySum;
//
//    %a scle to [0,1] range
//            y = y./max(y);
//
//    % Own function !!!!
//
//                   % FT dat
//            FA = fft(intensitySum);
//    % ulozi absolutni hodnoty FT dat
//    B = abs(FA);
//    % oreze cast FT spektra, ktere odpovida srdecni frekvenci nizsi nez 30
//                                                                        % 27 = FPS signalu
//    B(1:floor(length(B)/(frameRate*2)))=0;
//    % najde nejsilnejsi frekvenci, V je sila te frekvence, I je ta frekvence samotna
//    [V,I]=max(B);
//    % prepocita "obrazovou" frekvenci I na srdecni
//                                           % 27 = FPS signalu
//            bpmResult = round(60*frameRate*I/length(B));

    return 60;
}

void AmplificationWorker::setVideoBuffer(vector<Mat> videoBuffer) {

    this->videoBuffer.swap(videoBuffer);
};

void AmplificationWorker::clearVisualization() {
    this->visualization.clear();
}