#include <iostream>
#include <boost/thread.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>

#include "../imageOperation.h"
#include "../amplify.h"

#define INITIAL_FRAMES 20
#define BUFFER_FRAMES 20
#define FRAME_RATE 15
#define MICROSECONDS 1000000

using namespace cv;
using namespace std;

class AmplificationWorker {
public:
    void Compute(){
        cout << "Computing bpm";
        int ret = 100;

        // do stuff
        amplifySpatial(this->videoBuffer, this->filtered, 50, 50/60, 180/60, FRAME_RATE, BUFFER_FRAMES, 6);
        bpm = ret;
        flag = true;
        initialFlag = true;
        cout << "Computed bpm in class";
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
    vector<Mat> videoBuffer;
    vector<Mat> filtered;

    // OS window
    Mat window;

    // Execute while ended by user
    for (int frame = 0; true; frame++) {
        // Grab video frame
        Mat in;
        cam >> in; // type: CV_8U
        // Resize captured frame
        in = resizeImage(in, 700);

        // Output
        Mat out = in.clone();

        // Push to buffer according captured resized frame
        videoBuffer.push_back(out);

        // Start computing when buffer filled
        // TODO: REMOVE DEV ONLY
        if ((frame + 1) == BUFFER_FRAMES && frame != 0) {

            for (int j = 0; j < BUFFER_FRAMES; j++) {
                bpmWorker.videoBuffer.push_back(videoBuffer[j].clone());
            }
            videoBuffer.clear();
            boost::function<void()> th_bpm = boost::bind(&AmplificationWorker::Compute, &bpmWorker);
            boost::thread th(th_bpm);
        }

        // Update bpm once bpmWorker ready
        if (bpmWorker.flag) {
            bpmWorker.flag = false;
            currBpm = bpmWorker.bpm;
        }

        // At least first filtered vid computed
        if (bpmWorker.initialFlag) {
            imshow("FILTERED", bpmWorker.videoBuffer[0]);
        }



        // Adjustemnt of output
        //        fakeBeating(out, i, FRAME_RATE/10);

        // Merge original + adjusted
        hconcat(in, out, window);

        //put the image onto a screen

        imshow("video:", window);

        //press anything within the poped-up window to close this program
        if (waitKey(1) >= 0) break;


        // handle frame rate
//        usleep((double) MICROSECONDS / FRAME_RATE);

        i += .2;
    }
}
