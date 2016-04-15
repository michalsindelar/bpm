## Pulse detection from video source
This is experimental app trying to detect your pulse from fine video source or camera source. See __documentation__ for more information.

### Dependencies
* OpenCv
* C++
* CMake
* ffmpeg library (optional) for better video processing

### How to run
Install all dependencies at first. App is compiled via CMake. Navigate to app folder then run:

## OS X
Prefered way via brew.
`brew install opencv -v --with-ffmpeg --with-qt --c++11`
`brew install qt5`
`brew linkapps qt5`

### ffmpeg library
* Rotate video counterclockwise
** ffmpeg -i 2016-04-06_dad_89.MOV -vf "transpose=2" 2016-04-06_dad_89.MOV
* Fetch frames from video
** ffmpeg -i vid.mp4 -vf fps=30 filename%03d.bmp