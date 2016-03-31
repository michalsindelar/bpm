
#include "../Bpm.h"

int main (int argc, const char * argv[]) {
    Bpm bpm = Bpm(VIDEO_STATIC_SOURCE_MODE, FOURIER_MASK_MODE, 1.5f);

    // HERE WE GO!!
    bpm.run();
}
