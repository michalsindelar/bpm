
#include "../Bpm.h"

void my_button_cb(int state, void* userdata) {
    // convert userdata to the right type


}
int main (int argc, const char * argv[]) {

    // Mode selector
    Mat window;
    window = Mat(600, 800, CV_8UC3);
    window = Scalar(255,255,255);
    string windowName = "OS: Mode selector";
//    namedWindow( windowName, CV_WINDOW_AUTOSIZE);

//    while (true) {
//        imshow(windowName, window);
//        waitKey(1);
//    }

    Bpm bpm = Bpm(VIDEO_REAL_SOURCE_MODE, FOURIER_MASK_MODE, 1.5f);

    // HERE WE GO!!
    bpm.run();
}
