cmake_minimum_required(VERSION 3.3)
project(bpm)

# Opencv
find_package( OpenCV REQUIRED)
find_package(Boost COMPONENTS log log_setup thread filesystem system REQUIRED)
find_package (Qt4 REQUIRED)
include(${QT_USE_FILE})
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Define current folder
set(PROJECT_DIR "${CMAKE_SOURCE_DIR}")
configure_file(constants.h.in ${CMAKE_SOURCE_DIR}/constants.h)

include_directories(${OpenCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
include_directories(${CMAKE_BINARY_DIR})

set(SOURCE_FILES
        main.cpp
    lib/imageOperation.cpp
    lib/imageOperation.h
    lib/BpmWindow.cpp
    lib/BpmWindow.h
    lib/Middleware.cpp
    lib/Middleware.h
    lib/Detector.cpp
    lib/Detector.h
    lib/BpmVideoProcessor.cpp
    lib/BpmVideoProcessor.h
    lib/ModeSelectorWindow.cpp
    lib/ModeSelectorWindow.h
    lib/stabilization.cpp
    lib/stabilization.h
)

add_executable(bpm ${SOURCE_FILES})

target_link_libraries(bpm ${OpenCV_LIBS} ${Boost_LIBRARIES} Qt4::QtGui)
