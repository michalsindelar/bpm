#include <imageOperation.h>

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

void adjustOutput (Mat image) {
    double alpha = 1;
    int beta = 100;
    
    /// Do the operation new_image(i,j) = alpha*image(i,j) + beta
    for( int y = 0; y < image.rows; y++ ){
        for( int x = 0; x < image.cols; x++ ) {
            for( int c = 0; c < 3; c++ ) {
                image.at<Vec3b>(y,x)[c] =
                saturate_cast<uchar>( alpha*( image.at<Vec3b>(y,x)[c] ) + beta );
            }
        }
    }
    
}

void fakeBeating (Mat image, double index, int maxValue) {
    sleep(1);
    double alpha = 1;

    double brightness = 30*sin(index);
    
    // Map to allowed brightness
    cout << brightness;
    
    /// Do the operation new_image(i,j) = alpha*image(i,j) + beta
    for( int y = 0; y < image.rows; y++ ){
        for( int x = 0; x < image.cols; x++ ) {
            for( int c = 0; c < 3; c++ ) {
                image.at<Vec3b>(y,x)[c] =
                saturate_cast<uchar>( alpha*( image.at<Vec3b>(y,x)[c] ) + brightness );
            }
        }
    }
}