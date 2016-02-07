#include <iostream>
#include <boost/thread.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <highgui.h>

#include "../imageOperation.h"
#include "../amplify.h"

#define BUFFER_FRAMES 40
#define FRAME_RATE 15
#define CAMERA_INIT 10

using namespace cv;
using namespace std;

class AmplificationWorker {
public:
    void Compute(deque<Mat> videoBuffer){
        // At first fill class buffer with copies!
        this->setVideoBuffer(videoBuffer);
//        this->countDroppedFrames();

        cout << "Computing bpm";
        int ret = 100;

        // do stuff
        amplifySpatial(this->videoBuffer, this->filtered, 50, 50/60, 180/60, FRAME_RATE, BUFFER_FRAMES, 6);
        this->videoBuffer.clear();

        bpm = ret;
        flag = true;
        initialFlag = true;
        cout << "Computed bpm in class";
    };

    void setVideoBuffer(deque<Mat> videoBuffer) {
        int i = 0;
        for (Mat img : videoBuffer) {
            this->videoBuffer.push_back(img.clone());
            i++;
        }
    };


    AmplificationWorker() {
        this->initialFlag = false;
        this->flag = false;
    };
    int bpm;
    bool flag;
    bool initialFlag;

    vector<Mat> videoBuffer;
    vector<Mat> filtered;
};


int main (int argc, const char * argv[]) {
    VideoCapture cam(0);

    if(!cam.isOpened())
        return -1;

    // Class for bpm computing
    AmplificationWorker bpmWorker;


    bool initialWorkerFlag = false;
    int currBpm;
    double i = 0;

    // VideoBuffer
    deque<Mat> videoBuffer;
    vector<Mat> filtered;

    // OS window
    Mat window;

    // NOTE: The loop must be as simple as possible
    // Performance issues with dropping frames
    for (int frame = 0; true; frame++) {
        // Grab video frame
        Mat in;
        cam >> in; // type: CV_8U

        if (frame < CAMERA_INIT) continue;

        // Resize captured frame
        in = resizeImage(in, 700);

        // Output
        Mat out = in.clone();

        // Keep maximum BUFFER_FRAMES size
        if (videoBuffer.size() == BUFFER_FRAMES) {
            videoBuffer.pop_front();
        }
        videoBuffer.push_back(in.clone());

        // Update bpm once bpmWorker ready
        // Clear current filtered array
        // Copy to loop filtered vid
        // Clear bpmWorker filtered array
        if (bpmWorker.flag) {
            filtered.clear();

            for (Mat img : bpmWorker.filtered) {
                filtered.push_back(img.clone());
            }

            bpmWorker.filtered.clear();
            bpmWorker.flag = false;
        }

        // Start computing when buffer filled
        // TODO: REMOVE DEV ONLY
        if ((frame + 1) % BUFFER_FRAMES == 0 && frame > CAMERA_INIT + BUFFER_FRAMES) {
            boost::function<void()> th_bpm = boost::bind(&AmplificationWorker::Compute, &bpmWorker, videoBuffer);
            boost::thread th(th_bpm);
        }

        // Show filtered video after initialization compute
        if (bpmWorker.initialFlag) {
            imshow("FILTERED", resizeImage(filtered.at(frame % BUFFER_FRAMES), 800));
            if (waitKey(10) >= 0) break;
        }

        // Adjustemnt of output
         fakeBeating(out, i, FRAME_RATE/10);

        // Merge original + adjusted
        hconcat(in, out, window);

        //put the image onto a screen
        imshow("video:", window);

        // Free
        in.release();
        out.release();

        //press anything within the poped-up window to close this program
        if (waitKey(10) >= 0) break;

        i += .2;
    }
}
