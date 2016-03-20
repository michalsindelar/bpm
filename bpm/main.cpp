
#include "../Bpm.h"

int main (int argc, const char * argv[]) {
    Bpm bpm = Bpm(CAMERA_SOURCE_MODE, FOURIER_MASK_MODE, 1.5f);
//    bpm.setMode(FAKE_BEATING_MODE);

    // HERE WE GO!!
    bpm.run();
}
