cmake_minimum_required(VERSION 3.3)
project(bpm)

# Opencv
find_package( OpenCV REQUIRED)
find_package(Boost COMPONENTS log log_setup thread filesystem system)

# Define current folder
set(PROJECT_DIR "${CMAKE_BINARY_DIR}")
configure_file(config.h.in ${CMAKE_SOURCE_DIR}/config.h)

include_directories(${OpenCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
include_directories(${CMAKE_BINARY_DIR})


set(SOURCE_FILES
    bpm/main.cpp
    imageOperation.cpp
    imageOperation.h
    skinDetect.cpp
    skinDetect.h
    amplify.cpp
    amplify.h
    Bpm.cpp
    Bpm.h
    AmplificationWorker.cpp
    AmplificationWorker.h
    FaceDetectorWorker.cpp
    FaceDetectorWorker.h
)


add_executable(bpm ${SOURCE_FILES})

target_link_libraries(bpm ${OpenCV_LIBS} ${Boost_LIBRARIES})
