#include <skinDetect.h>

enum Lighting { DAYLIGHT, DAYLIGHT2, INCANDESCENT, FLUORESCENT, DUSK };

/* function converts original frame from video to HSV image,
 then creates HSV mask that contains all skin colored objects. */
void createHsvMask(Mat src, Mat& dst, Lighting light)
{
    int hMin, sMin, vMin;
    int hMax, sMax, vMax;
    
    /*HSV coefficients of skin color in different lighting
     USE INCANDESCENT - WORKS BEST (WITH LIGHT BULB) !! */
    switch (light)
    {
        case DAYLIGHT:
            hMin = 0; sMin = 0; vMin = 20;
            hMax = 50; sMax = 40; vMax = 100;
            break;
            
        case DAYLIGHT2:
            hMin = 300; sMin = 0; vMin = 40;
            hMax = 360; sMax = 40; vMax = 100;
            break;
            
        case INCANDESCENT:
            hMin = 0; sMin = 0; vMin = 30;
            hMax = 60; sMax = 40; vMax = 100;
            break;
            
        case FLUORESCENT:
            hMin = 0; sMin = 0; vMin = 20;
            hMax = 70; sMax = 60; vMax = 100;
            break;
            
        case DUSK:
            hMin = 220; sMin = 0; vMin = 20;
            hMax = 360; sMax = 50; vMax = 80;
            break;
            
        default:
            break;
    }
    
    /*GIMP HSV coeff. (max) 360 100 100
     OPENCV HSV coeff. (max) 180 255 255 */
    Scalar skinColorMin((int)hMin / 2, (int)sMin * 2.55, (int)vMin * 2.55);
    Scalar skinColorMax((int)hMax / 2, (int)sMax * 2.55, (int)vMax * 2.55);
    
    GaussianBlur(src, src, Size(5, 5), 1.5, 1.5);
    cvtColor(src, src, CV_BGR2HSV);
    inRange(src, skinColorMin, skinColorMax, dst);
}

void detectSkin (Mat mask) {
    Mat source;
    mask.copyTo(source);
    
    /* creating HSV mask of image */
    Mat hsvMask;
    createHsvMask(source, hsvMask, DAYLIGHT);
    //dilate(hsvMask, hsvMask, Mat(5, 5, CV_16U));
    
    /* labeling of components */
    vector<vector<Point>> contours;
    Mat contourMask;
    vector<Vec4i> hierarchy;
    hsvMask.copyTo(contourMask);
    findContours(contourMask, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
    
    /* choose only biggest components, eliminating noise etc */
    mask.setTo(Scalar(0,0,0));
    vector<vector<Point>> chosenContours;
    for (int i = 0; i < contours.size(); i++) {
        double a = contourArea(contours[i], false);
        
        if (a >= 500) {
            drawContours(mask, contours, i, Scalar(255, 255, 255), FILLED, 8, hierarchy);
            chosenContours.push_back(contours[i]);
        }
    }
}