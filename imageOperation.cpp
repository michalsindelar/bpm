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