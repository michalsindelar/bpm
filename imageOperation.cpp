#include <imageOperation.h>
#include <skinDetect.h>

Mat resizeImage (Mat image, const double width) {
    // Resize in
    Size origSize = image.size();
    
    // Keep aspect ratio
    double aspect = (double) origSize.width / origSize.height;
    
    // Compute heigh
    double height = width / aspect;
    
    // Resize
    resize(image, image, Size(width, height), 0, 0, INTER_LINEAR);
    
    return image;
}


void fakeBeating (Mat image, double index, int maxValue) {

    double alpha = 1;
    double brightness = 30*sin(index * 4);
    
    
    Mat mask; image.copyTo(mask);
    detectSkin(mask);
    
    /// Do the operation new_image(i,j) = alpha*image(i,j) + beta
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

