# BPM: Pulse detection from video source
This is experimental app trying to detect your pulse from fine video source or camera source. See __documentation__ for more information.

## Dependencies
* C++
* OpenCv > 2
  * --with-ffmpeg support
  * --with-qt support
* Qt5 framework
* CMake > 3



## How to run
Install all dependencies at first.
### OS X
Prefered way via brew.
`brew install opencv -v --with-ffmpeg --with-qt --c++11`

`brew install qt5`

`brew linkapps qt5`

`brew install moc` - auto mock needed for qt
### Linux
[ ] TODO: Confirm OS X instructions working
### Windows
[ ] TODO: Check compatibility

---

App is compiled via CMake. Navigate to app folder. Folder must be *writable* (sudo permission needed otherwise), you could run:

`chmod 777 ./`

The run following commands:

`cmake .`

`make bpm_automoc`

`sudo make`

`./bpm`

For better performance you could try increasing priority of process (especially when using webcamera).

`$ nice -n -10 ./bpm`

Enjoy!
