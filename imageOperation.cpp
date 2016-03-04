#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include "imageOperation.h"

Mat resizeImage (Mat image, const double width) {
    // Check if image has size
    if (image.rows == 0) {
        return image;
    }

    // Resize in
    Size origSize = image.size();

    // Keep aspect ratio
    double aspect = (double) origSize.width / origSize.height;

    // Compute height
    double height = width / aspect;

    // Resize
    resize(image, image, Size(width, height), 0, 0, INTER_LINEAR);

    return image;
}

vector<Rect> detectFace (Mat image) {
    CascadeClassifier face_cascade;
    face_cascade.load("/Users/michal/Dev/bpm/haarcascade_frontalface_alt.xml");

    // Create bw mat
    Mat frame_gray;
    cvtColor(image, frame_gray, CV_BGR2GRAY );

    // Init faces
    vector<Rect> faces;
    face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(80, 80) );

    return faces;
}

void fakeBeating (Mat image, double index, int maxValue) {

    double alpha = 1;
    double brightness = 30*sin(index * 4);
    
    
    // Mask init
    Mat mask; image.copyTo(mask);
    mask.setTo(Scalar(0,0,0));
    
    // Load face cascade
    CascadeClassifier face_cascade;
    face_cascade.load("/Users/michal/Dev/bpm/haarcascade_frontalface_alt.xml");
    
    // Create bw mat
    Mat frame_gray;
    cvtColor(image, frame_gray, CV_BGR2GRAY );

    // Init faces
    vector<Rect> faces;
    face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(80, 80) );
    
    // Mask filled
    for( size_t i = 0; i < faces.size(); i++ ) {
        Point center( faces[i].x + int(faces[i].width*0.5f), faces[i].y + int(faces[i].height * 0.5f));
        ellipse( mask, center, Size( int(faces[i].width*0.6f), int(faces[i].height*0.7f)), 0, 0, 360, Scalar( 255, 255, 255 ), -1, 8, 0 );
    }
    
    // Blur mask
    // GaussianBlur(mask, newMask, Size(10,10), 0, 0);
    
    for( int y = 0; y < image.rows; y++ ){
        for( int x = 0; x < image.cols; x++ ) {
            if (mask.at<Vec3b>(y,x)[1] == 0) continue;
            
            for( int c = 0; c < 3; c++ ) {
                image.at<Vec3b>(y,x)[c] =
                saturate_cast<uchar>( alpha*( image.at<Vec3b>(y,x)[c] ) + brightness );
            }
        }
    }
}

