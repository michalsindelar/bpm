#include <iostream>
#include <ctime>
#include <unistd.h>
#include <boost/thread.hpp>


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <imageOperation.h>
#include <skinDetect.h>

#define INITIAL_FRAMES 20
#define BUFFER_FRAMES 20
#define FRAME_RATE 15
#define MICROSECONDS 1000000

using namespace cv;
using namespace std;

class InitialWorker {
public:
    void Compute(){
        cout << "Computing in class";
        int ret = 100;
        // do stuff
        sleep(5);
        bpm = ret;
        flag = true;
        cout << "Computed in class";
    }
    int bpm;
    bool flag = false;
};

int main (int argc, const char * argv[]) {
    VideoCapture cam(0);
    
    if(!cam.isOpened())
        return -1;
    
    Mat imagesBuffer[BUFFER_FRAMES];

    
    InitialWorker worker;

    bool initialWorkerFlag = false;
    int currBpm;
    double i = 0;
    

    Mat window; // OS window

    
    // Execute while ended by user
    for (int frames = 0; true; frames++) {
        
        // Handle frame rate
        // int frameRate = 15;
        
        // Grab video frame
        Mat in;
        cam >> in;
        
        // Push to buffer according to refresh rate
        imagesBuffer[frames % BUFFER_FRAMES] = in;
        
        // Initial compute in own thread
        if (frames == INITIAL_FRAMES) {
            boost::function<void()> th_func = boost::bind(&InitialWorker::Compute, &worker);
            boost::thread th(th_func);
        }
        
        // Check if initial compute is done
        if (worker.flag && !initialWorkerFlag) {
            initialWorkerFlag = true;
            currBpm = worker.bpm;
            cout << "FINISHED";
        }
        
        // Resize
        in = resizeImage(in, 700);
        
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
