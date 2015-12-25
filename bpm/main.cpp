#include <iostream>
#include <ctime>
#include <thread>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include <imageOperation.h>

using namespace cv;
using namespace std;

void adjustOutput (Mat image) {
    double alpha = 2;
    int beta = 2;
    
    /// Do the operation new_image(i,j) = alpha*image(i,j) + beta
    for( int y = 0; y < image.rows; y++ ){
        for( int x = 0; x < image.cols; x++ ) {
            for( int c = 0; c < 3; c++ ) {
                image.at<Vec3b>(y,x)[c] =
                saturate_cast<uchar>( alpha*( image.at<Vec3b>(y,x)[c] ) + beta );
            }
        }
    }

}

int readCamNormal () {
    VideoCapture cam(0);
    
    if(!cam.isOpened())
        return -1;
    
    // Execute while ended by user
    while (true) {
        
        // Handle frame rate
        // int frameRate = 15;
        
        // sleep(100);
        
        // Grab video frame
        Mat in;
        cam >> in;
        
        // Resize
        in = resizeImage(in, 300);
        
        // Output
        Mat out = in;
        
        // Adjustemnt of output
        adjustOutput(out);
        
        // Merge original + adjusted
        Mat window;
        hconcat(in, out, window);
        
        
        //put the image onto a screen
        imshow("video:", window);
        
        //press anything within the poped-up window to close this program
        if (waitKey(1) >= 0) break;
    }
    
    return 0;
}

int readCamAdjust () {
    VideoCapture cam(0);
    
    if(!cam.isOpened())
        return -1;
    
    // Execute while ended by user
    while (true) {
        
        // Handle frame rate
        // int frameRate = 15;
        
        // sleep(100);
        
        // Grab video frame
        Mat in;
        cam >> in;
        
        // Resize
        in = resizeImage(in, 300);
        
        // Output
        Mat out = in;
        
        // Adjustemnt of output
        adjustOutput(out);
        
        // Merge original + adjusted
        Mat window;
        hconcat(in, out, window);
        
        
        //put the image onto a screen
        imshow("video:", window);
        
        //press anything within the poped-up window to close this program
        if (waitKey(1) >= 0) break;
    }
    
    return 0;
}


int main (int argc, const char * argv[]) {
    VideoCapture cam(0);
    

    
    if(!cam.isOpened())
        return -1;

    
    // Execute while ended by user
    while (true) {
        
        // Handle frame rate
        // int frameRate = 15;

        // sleep(100);
        
        // Grab video frame
        Mat in;
        cam >> in;
        
        // Resize
        in = resizeImage(in, 300);
        
        // Output
        Mat out = in;

        // Adjustemnt of output
        adjustOutput(out);
        
        // Merge original + adjusted
        Mat window;
        hconcat(in, out, window);
        

        //put the image onto a screen
        imshow("video:", window);
        
        //press anything within the poped-up window to close this program
        if (waitKey(1) >= 0) break;
    }
}
