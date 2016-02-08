
#include "../Bpm.h"

#define BUFFER_FRAMES 40
#define FRAME_RATE 15
#define CAMERA_INIT 10

int main (int argc, const char * argv[]) {
    Bpm bpm = Bpm();

    // HERE WE GO!!
    bpm.run();
}
