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


void fakeBeating (Mat image, double index, int maxValue, Rect face) {

    double alpha = 1;
    double brightness = 30*sin(index * 4);

    // Mask init
    Mat mask; image.copyTo(mask);
    mask.setTo(Scalar(0,0,0));

    // Mask filled
    Point center( face.x + int(face.width*0.5f), face.y + int(face.height * 0.5f));
    ellipse( mask, center, Size( int(face.width*0.6f), int(face.height*0.7f)), 0, 0, 360, Scalar( 255, 255, 255 ), -1, 8, 0 );

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


void controlFacePlacement (Rect & roi, const Size frame) {
    int maxXIndex = roi.x + roi.width + ERASED_BORDER_WIDTH;
    int maxYIndex = roi.y + roi.height + ERASED_BORDER_WIDTH;

    roi.width = (maxXIndex > frame.width) ?
        roi.width - (maxXIndex - frame.width) :
        roi.width;
    roi.height = (maxYIndex > frame.height) ?
         roi.height - (maxYIndex - frame.height) :
         roi.height;
}

Mat cropImageBorder (Mat image, int borderWidth) {
    Rect roi(borderWidth, borderWidth, image.cols - 2*borderWidth, image.rows - 2*borderWidth);
    return image(roi);
}

// Create a YIQ image from the RGB image using an approximation of NTSC conversion(ref: "YIQ" Wikipedia page).
// Remember to free the generated YIQ image.
// Taket from http://www.shervinemami.info/colorConversion.html
Mat convertImageRGBtoYIQ(Mat img)
{
    uchar r, g, b;
    double y, i, q;

    Mat out(img.rows, img.cols, CV_8UC3);

    /* convert image from RGB to YIQ */

    int m=0, n=0;
    for(m=0; m<img.rows; m++)
    {
        for(n=0; n<img.cols; n++)
        {
            r = img.data[m*img.step + n*3 + 2];
            g = img.data[m*img.step + n*3 + 1];
            b = img.data[m*img.step + n*3 ];
            y = 0.299*r + 0.587*g + 0.114*b;
            i = 0.596*r - 0.275*g - 0.321*b;
            q = 0.212*r - 0.523*g + 0.311*b;

            out.data[m*img.step+n*3 +2] = y;
            out.data[m*img.step+n*3 +1] = CV_CAST_8U((int)(i));
            out.data[m*img.step+n*3 ] = CV_CAST_8U((int)(q));

        }
    }
    return out;
}

// Create an RGB image from the YIQ image using an approximation of NTSC conversion(ref: "YIQ" Wikipedia page).
// Remember to free the generated RGB image.
// Taken from http://www.shervinemami.info/colorConversion.html
Mat convertImageYIQtoRGB( Mat img)
{
    uchar r, g, b;
    double y, i, q;

    Mat out(img.rows, img.cols, CV_8UC3);
    int type = out.type();
    /* convert image from RGB to YIQ */

    int m=0, n=0;
    for(m=0; m<img.rows; m++)
    {
        for(n=0; n<img.cols; n++)
        {
            y = img.data[m*img.step + n*3 + 2];
            i = img.data[m*img.step + n*3 + 1];
            q = img.data[m*img.step + n*3 ];

            r =  y  + 0.9563 * i + 0.6210 * q;
            g =  y  - 0.2721 * i - 0.6474 * q;
            b =  y  - 1.1070 * i + 1.7046 * q;

            out.data[m*img.step+n*3 +2] = r;
            out.data[m*img.step+n*3 +1] = CV_CAST_8U((int)(g));
            out.data[m*img.step+n*3 ] = CV_CAST_8U((int)(b));

        }
    }
    return out;
}

void cvtColor2(Mat src, Mat & dst, int code) {
    switch (code) {
        case CV2_BGR2YIQ :
            dst = convertImageRGBtoYIQ(src);
        break;
        case CV2_YIQ2BGR :
            dst = convertImageYIQtoRGB(src);
            break;
        default :
            dst = convertImageYIQtoRGB(src);
        break;
    }
}