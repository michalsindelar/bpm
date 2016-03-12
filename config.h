#ifndef CONFIG_H
#define CONFIG_H

// define your version_libinterface
#define PROJECT_DIR "/Users/michal/Dev/bpm"

#define BUFFER_FRAMES 50
#define FRAME_RATE 10
#define LOOP_WAIT_TIME_MS (1000 / FRAME_RATE)
#define LOOP_WAIT_TIME_MUS (CLOCKS_PER_SEC / FRAME_RATE)
#define CAMERA_INIT 10
#define RESIZED_FRAME_WIDTH 600

#endif // CONFIG_H
