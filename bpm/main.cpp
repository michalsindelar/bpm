#include <iostream>
#include <boost/thread.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>

#include "../imageOperation.h"
#include "../amplify.h"

#define INITIAL_FRAMES 20
#define BUFFER_FRAMES 50
#define FRAME_RATE 15
#define MICROSECONDS 1000000

using namespace cv;
using namespace std;

class InitialWorker {
public:
    void Compute(){
        cout << "Computing in class";
        // do stuff
        sleep(5);
        flag = true;
        cout << "Computed in class";
    }
    bool flag = false;
};


class AmplificationWorker {
public:
    void Compute(){
        cout << "Computing bpm";
        int ret = 100;

        // do stuff
        amplifySpatial(videoBuffer, filtered, 50, 50/60, 180/60, FRAME_RATE, BUFFER_FRAMES, 6);

        bpm = ret;

        flag = true;
        initialFlag = true;
        cout << "Computed bpm in class";
    }
    int bpm;
    bool flag;
    bool initialFlag;
    
    Mat *filtered;
    Mat *videoBuffer;
};


int main (int argc, const char * argv[]) {
    VideoCapture cam(0);
    
    if(!cam.isOpened())
        return -1;
    
    Mat videoBuffer[BUFFER_FRAMES];


    // Class for initial compute
    InitialWorker worker;

    // Class for bpm computing
    AmplificationWorker bpmWorker;


    bool initialWorkerFlag = false;
    int currBpm;
    double i = 0;

    // Computed from amplify
    Mat filtered[BUFFER_FRAMES];

    // OS window
    Mat window;

    // Execute while ended by user
    for (int frame = 0; true; frame++) {
        
        // Grab video frame
        Mat in;
        cam >> in; // type: CV_8U

        // Initial compute in own thread
        if (frame == INITIAL_FRAMES) {
            boost::function<void()> th_func = boost::bind(&InitialWorker::Compute, &worker);
            boost::thread th(th_func);
        }

        // Check if initial compute is done
        if (worker.flag && !initialWorkerFlag) {
            initialWorkerFlag = true;
            cout << "FINISHED";
        }

        // Resize captured frame
        in = resizeImage(in, 700);

        // Push to buffer according captured resized frame
        videoBuffer[frame % BUFFER_FRAMES] = in;

        // Start computing when buffer filled
        if (frame % BUFFER_FRAMES == 0 && frame != 0) {
            bpmWorker.videoBuffer = videoBuffer;
            bpmWorker.filtered = filtered;

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
            imshow("FILTERED", resizeImage(filtered[frame % BUFFER_FRAMES], 1200));
        }

        // Output
        Mat out = in.clone();

        // Adjustemnt of output
        fakeBeating(out, i, FRAME_RATE/10);

        // Merge original + adjusted
        hconcat(in, out, window);

        //put the image onto a screen
        imshow("video:", window);

        //press anything within the poped-up window to close this program
        if (waitKey(1) >= 0) break;


        // handle frame rate
        usleep((double) MICROSECONDS / FRAME_RATE);

        i += .2;
    }
}
