#include "imageOperation.h"

Mat resizeImage (Mat image, const double width, int interpolation) {
    // Check if image has size
    if (image.rows == 0) {
        return image;
    }

    // Resize
    resize(image, image, getResizedSize(image.size(), width), 0, 0, interpolation);

    return image;
}

Size getResizedSize(Size origSize, const double width) {
    // Keep aspect ratio
    double aspect = (double) origSize.width / origSize.height;
    return Size(width, width / aspect);
}

Mat resizeImage(Mat image, const double width) {
    return resizeImage(image, width, INTER_LINEAR);
}

void resizeCropVideo(vector<Mat> &video, int width) {
    for (int i = 0; i < BUFFER_FRAMES; i++) {
        video[i] = resizeImage(video[i], width, INTER_LINEAR);
        video[i] = cropImageBorder(video[i], ERASED_BORDER_WIDTH);
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

void amplifyChannels(vector<Mat>& channels, int r, int g, int b) {
    channels[RED_CHANNEL] = channels[RED_CHANNEL] * r;
    channels[GREEN_CHANNEL] = channels[GREEN_CHANNEL] * g;
    channels[BLUE_CHANNEL] = channels[BLUE_CHANNEL] * b;
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

float freqToBpmMapper(int fps, int framesCount, int index) {
    return (int) (round(SECONDS_IN_MINUTE * fps * (index+1)) / framesCount);
}

float findStrongestRowFreq(vector<int> row, int framesCount, int fps) {
    // Create matrix from intensitySum
    float maxValue = 0;
    for (int i = 0; i < row.size(); i++) {
        maxValue = (row[i] > maxValue) ? row[i] : maxValue;
    }

    Mat rowMat = Mat::zeros(1, row.size(), CV_32F);

    // Normalization
    for (int i = 0; i < row.size(); i++) {
        rowMat.at<float>(i) = row[i] / maxValue;
    }

    return findStrongestRowFreq(rowMat, framesCount, fps);
}

float findStrongestRowFreq(Mat row, int framesCount, int fps) {
    // DFT of intensities
    float maxFreq = 0;
    int maxFreqLoc = 0;
    int bpm = 0;

    row.convertTo(row, CV_32FC1);

    // Compute dft
    Mat planes[] = {Mat_<float>(row), Mat::zeros(row.size(), CV_32FC1)};
    Mat complexI;

    // Add to the expanded another plane with zeros
    merge(planes, 2, complexI);

    // Compute dft
    dft(complexI, complexI);

    split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
    magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
    Mat magI = planes[0];

    // We need only positive values
    for (int i = 1; i < framesCount; i++) {
        bpm = freqToBpmMapper(fps, framesCount, i);

        // TODO: This should be connected with bpm!
        // TODO: Define cut-off freq to constants
        if (bpm < 50) continue; // This is under low frequency
        if (bpm > 180) continue; // This is over high frequency

        if (magI.at<float>(i) > maxFreq) {
            maxFreq = magI.at<float>(i);
            maxFreqLoc = i;
        }
    }
    return freqToBpmMapper(fps, framesCount, maxFreqLoc);
}

Mat maskingCoeffs(int width, float fl, float fh, int fps) {
    Mat row(1, width, CV_32FC1);

    // Create row 0.25 - 0.5 ----- FRAME RATE
    for (int i = 0; i < width; i++) {
        int bpm = freqToBpmMapper(fps, width, i);
        float value = (bpm < fl || bpm > fh) ? 0 : 1;
        row.at<float>(0, i) = value;
    }

    return row;
}

/**
* BINOMIAL 5 - kernel
* 1 4 6 4 1
* 4 16 24 16 4
* 6 24 36 24 6
* 4 16 24 16 4
* 1 4 6 4 1
*/
Mat binom5Kernel() {
    Mat kernel(5, 5, CV_32FC1);

    // 1st row
    kernel.at<float>(0,0) = 1.0f;
    kernel.at<float>(1,0) = 4.0f;
    kernel.at<float>(2,0) = 6.0f;
    kernel.at<float>(3,0) = 4.0f;
    kernel.at<float>(4,0) = 1.0f;

    // 2nd row
    kernel.at<float>(0,1) = 4.0f;
    kernel.at<float>(1,1) = 16.0f;
    kernel.at<float>(2,1) = 24.0f;
    kernel.at<float>(3,1) = 16.0f;
    kernel.at<float>(4,1) = 4.0f;

    // 3rd row
    kernel.at<float>(0,2) = 6.0f;
    kernel.at<float>(1,2) = 24.0f;
    kernel.at<float>(2,2) = 36.0f;
    kernel.at<float>(3,2) = 24.0f;
    kernel.at<float>(4,2) = 6.0f;

    // 4th row
    kernel.at<float>(0,3) = 4.0f;
    kernel.at<float>(1,3) = 16.0f;
    kernel.at<float>(2,3) = 24.0f;
    kernel.at<float>(3,3) = 16.0f;
    kernel.at<float>(4,3) = 4.0f;

    // 5th row
    kernel.at<float>(0,4) = 1.0f;
    kernel.at<float>(1,4) = 4.0f;
    kernel.at<float>(2,4) = 6.0f;
    kernel.at<float>(3,4) = 4.0f;
    kernel.at<float>(4,4) = 1.0f;

    kernel = kernel/256.0f;

    return kernel;
}
void blurDn(Mat & frame, int level, Mat kernel) {
    if (level == 1) return;
    if (level > 1) blurDn(frame, level-1, kernel);

    // Condition, but anyway this shouldn't happen at all
    if (frame.cols < 10 || frame.rows < 10) return;

    // resize 1/2
    resize(frame, frame, Size(frame.size().width / 2, frame.size().height / 2), 0, 0, INTER_NEAREST);

    // blur via binomial filter
    filter2D(frame, frame, -1, kernel);

}

vector<int> countIntensities(vector<Mat> &video) {
    vector <int> intensitySum(video.size());
    Size videoFrame(video[0].cols, video[0].rows);

    for (int frame = 0; frame < video.size(); frame++) {
        uint8_t* pixelPtr = (uint8_t*)video[frame].data;
        int cn = video[frame].channels();
        Scalar_<uint8_t> bgrPixel;
        for(int i = 0; i < videoFrame.height; i++) {
            for(int j = 0; j < videoFrame.width; j++) {
                float tmp = pixelPtr[i*video[frame].cols*cn + j*cn + 0] + pixelPtr[i*video[frame].cols*cn + j*cn + 1] + pixelPtr[i*video[frame].cols*cn + j*cn + 2];
                intensitySum.at(frame) += (int)tmp;
            }
        }
    }
    return intensitySum;
}
void saveIntensities(vector<Mat>& video, string filename) {
    ofstream myfile;
    myfile.open((string) PROJECT_DIR + "/"  + filename, ios::out);

    vector<int> intensitySum = countIntensities(video);

    for (int i = 0; i < video.size(); i++) {
        myfile << intensitySum.at(i);
        myfile << "\n";
    }
    myfile.close();
}

void generateTemporalSpatialImages(vector<vector<Mat> > temporalSpatialStack) {
    for(int i = 0; i < 14; i++) {
        Mat img;

        vector<Mat> channels;
        for (int channel = 0; channel < 3; channel++) {
            Mat tmp;
            temporalSpatialStack[channel][i].convertTo(tmp, CV_8UC1);
            channels.push_back(tmp);
        }
        merge(channels, img);

        imwrite( (string) PROJECT_DIR+"/images/temporalSpatial/"+to_string(i)+".jpg", img );
    }
}
