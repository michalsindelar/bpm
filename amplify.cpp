//
// Created by Michal on 30/01/16.
//

#include "amplify.h"

#define BLUE_CHANNEL 0
#define GREEN_CHANNEL 1
#define RED_CHANNEL 2

void amplifySpatial(vector<Mat>& video, vector<Mat>& out, int & bpm, double alpha, int lowLimit, int highLimit, int videoRate, int framesCount, int level) {

    // Allocate stack
    vector<Mat> stack;

    // Create gdown stack
    buildGDownStack(video, stack, framesCount, level);

    // Filtering
    bandpass(stack, out, lowLimit, highLimit, videoRate, framesCount);

    // Count intensities
    bpm = computeBpm(countIntensities(out));

    // Clear data
    stack.clear();
}

// Based on https://github.com/diego898/matlab-utils/blob/master/toolbox/EVM_Matlab/build_GDown_stack.m
// TODO: Check wheter rgb separate channels needed
void buildGDownStack(vector<Mat>& video, vector<Mat>& stack, int framesCount, int level) {
    Mat kernel = binom5Kernel();
    for (int i = 0; i < framesCount; i++) {
        // Result image push to stack
        Mat tmp = blurDn(video[i], level, kernel);
        stack.push_back(tmp.clone());
        tmp.release();
    }
    kernel.release();
}

/**
 * Downsample 2x and blur image in pyramid level
 * TODO: Blurring should be via  binomial filter see link
 * http://optica.csic.es/projects/tools/steer/1.matlabPyrTools/namedFilter.html

 * Each chanel separate
 */
Mat blurDn(Mat frame, int level, Mat kernel) {
    if (level == 1) return frame;
    if (level > 1) frame = blurDn(frame, level-1, kernel);

    // resize 1/2
    resize(frame, frame, Size(frame.size().width / 2, frame.size().height / 2), 0, 0, INTER_LINEAR);

    // FLoat at first
    cvtColor(frame, frame, CV_32F);
    // Convert to hsv (similar as ntsc)
    cvtColor(frame, frame, CV_BGR2HSV);

    // blur via binomial filter
    filter2D(frame, frame, -1, kernel);

    // Convert back
    cvtColor(frame, frame, CV_HSV2BGR);
    cvtColor(frame, frame, CV_32F);

    // This normalization is super important !!!!
    normalize(frame, frame, 0, 1, NORM_MINMAX, CV_32F);

    return frame;
}

void bandpass(vector<Mat>& video, vector<Mat>& filtered, int lowLimit, int highLimit, int videoRate, int framesCount) {
    // TODO: Describe
    int height =  video[0].size().height;
    int width =  video[0].size().width;

    // TODO: Connect with main class
    // http://vgg.fiit.stuba.sk/2012-05/frequency-domain-filtration/
    int fps = 20;
    int fl = 60/60; // Low freq cut-off
    int fh = 200/60; // High freg cut-off

    // Prepare freq.
    // Create mask
    Mat mask = maskingCoeffs(video.size(), fps, fl, fh);

    // Create time stack change
    vector <vector<Mat> > timeStack(3);

    // Must be in color channels
    createTimeChangeStack(video, timeStack, RED_CHANNEL);
    createTimeChangeStack(video, timeStack, GREEN_CHANNEL);
    createTimeChangeStack(video, timeStack, BLUE_CHANNEL);

    //    Mat kernelSpec = maskKernel( getOptimalDFTSize(video[0].cols), getOptimalDFTSize(video[0].rows), video.size(), fps, fl, fh);
    //    float amplCoeffs[] = {50*0.2f, 50*0.2f, 50};

    for (int i = 0; i < timeStack[0].size(); i++) {
        for (int channel = 0; channel < 3; channel++) {
                for (int row = 0; row < timeStack[channel][i].rows; row++) {

                    // FFT
                    Mat fourierTransform;
                    dft(timeStack[channel][i].row(row), fourierTransform, cv::DFT_SCALE|cv::DFT_COMPLEX_OUTPUT);

                    // MASKING
                    fourierTransform = fourierTransform.mul(mask);

                    // IFFT
                    dft(fourierTransform, timeStack[channel][i].row(row), cv::DFT_INVERSE|cv::DFT_REAL_OUTPUT);

                    // RELEASE
                    fourierTransform.release();
            }
        }
    }
    mask.release();
    inverseCreateTimeChangeStack(timeStack, filtered);

}

// Assume video is single channel
void createTimeChangeStack(vector<Mat>& video, vector <vector<Mat> >& dst, int channel) {

    // DST vector
    // video[0].size().width - vectors count
    // width: video.size()
    // height video[0].size().height
    int dstVectorCount = video[0].size().width;
    int dstVectorWidth = (int) video.size();
    int dstVectorHeight = video[0].size().height;

    for (int i = 0; i < dstVectorCount; i++) {
        // One frame
        Mat frame(dstVectorHeight, dstVectorWidth, CV_32F);
        for (int j = 0; j < dstVectorWidth; j++) {
            vector<Mat> channels;
            split(video[j],channels);

            for(int k = 0; k < dstVectorHeight; k++) {
                frame.at<float>(k,j) = channels[channel].at<float>(k,i);
            }
        }
        dst[channel].push_back(frame);
        frame.release();
    }
}

void inverseCreateTimeChangeStack(vector <vector<Mat> >& stack, vector<Mat>& dst) {

    int dstCount = stack[0][0].size().width;
    int dstWidth = (int) stack[0].size();
    int dstHeight = stack[0][0].size().height;

    for (int i = 0; i < dstCount; i++) {

        // Initialize channels
        // TODO: Can't be one line??
        vector<Mat> channels;
        for (int channel = 0; channel < 3; channel++) {
            channels.push_back(Mat(dstHeight, dstWidth, CV_32F));
        }

        for (int j = 0; j < dstWidth; j++) {
            for(int k = 0; k < dstHeight; k++) {
                for (int channel = 0; channel < 3; channel++) {
                    channels[channel].at<float>(k, j) = stack[channel][j].at<float>(k,i);
                }
            }
        }

        // Amplify frame's channels
        amplifyChannels(channels, 5, 1, 1);

        // Merge channels into colorFrame
        Mat colorFrame;
        merge(channels, colorFrame);

        // Convert to basic CV_8UC3 in range [0,255]
        colorFrame.convertTo(colorFrame, CV_8UC3, 255);

//        Rect roi(0, 0, colorFrame.cols, colorFrame.rows);
        dst.push_back(colorFrame);
        channels.clear();
        colorFrame.release();
    }
}


Mat maskingCoeffs(int width, int fps, int fl, int fh) {
    Mat row(1, width, CV_32FC2);

    // 1st row
    row.at<float>(0,0) = ((1.0f / 256.0f < fl) || (1.0f / 256.0f > fh)) ? 0 : 1;

    // Create row 0.25 - 0.5 ----- 30.0
    for (int i = 1; i < width; i++) {
        float value = (i-1)/( (float) width)* (float) fps;
        value = (value < fl || value > fh) ? 0 : 1;
        row.at<float>(0, i) = value;
    }
    return row;
}


Mat computeDFT(Mat image) {
    // copy the source image, on the border add zero values
    Mat planes[] = { Mat_< float> (image), Mat::zeros(image.size(), CV_32F) };
    // create a complex matrix
    Mat complex;
    merge(planes, 2, complex);
    dft(complex, complex, DFT_COMPLEX_OUTPUT);  // fourier transform
    return complex;
}

Mat updateResult(Mat complex) {
    Mat work;
    idft(complex, work);
    //  dft(complex, work, DFT_INVERSE + DFT_SCALE);
    Mat planes[] = {Mat::zeros(complex.size(), CV_32F), Mat::zeros(complex.size(), CV_32F)};
    split(work, planes);                // planes[0] = Re(DFT(I)), planes[1] = Im(DFT(I))

    magnitude(planes[0], planes[1], work);    // === sqrt(Re(DFT(I))^2 + Im(DFT(I))^2)
    normalize(work, work, 0, 1, NORM_MINMAX);

    return work;
}


void amplifyChannels(vector<Mat>& channels, int r, int g, int b) {
    channels[RED_CHANNEL] = channels[RED_CHANNEL] * r;
    channels[GREEN_CHANNEL] = channels[GREEN_CHANNEL] * g;
    channels[BLUE_CHANNEL] = channels[BLUE_CHANNEL] * b;

}


vector<int> countIntensities(vector<Mat> &video) {
    vector <int> intensitySum(BUFFER_FRAMES);
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
    myfile.open(filename, ios::out);

    vector<int> intensitySum = countIntensities(video);

    for (int i = 0; i < video.size(); i++) {
        myfile << intensitySum.at(i);
        myfile << "\n";
    }
    myfile.close();
}

int computeBpm(vector<int> intensitySum) {

    int intensityCount = BUFFER_FRAMES;

    // Normalize intensities
//    normalize(intensitySum, intensitySum);

    // DFT of intensities
    Mat fa(intensitySum);
    fa.convertTo(fa, CV_32FC1);
    dft(fa, fa, DFT_REAL_OUTPUT);

    // Find max value & locaiton
    float maxFreq = 0;
    int maxFreqLoc = 0;

    int lowFreq = (int) BUFFER_FRAMES / (3*FRAME_RATE);

    // We need only positive values
    for (int i = 1; i < BUFFER_FRAMES; i++) {
        // This is under low frequency

        // TODO: How is this computed
        if (i < lowFreq) continue;

        fa.at<float>(i) = abs(fa.at<float>(i));
        if (fa.at<float>(i) > maxFreq) {
            maxFreq = fa.at<float>(i);
            maxFreqLoc = i;
        }
    }
    int returnVal = round(60 * FRAME_RATE * maxFreqLoc / BUFFER_FRAMES);
    return returnVal;
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
    Mat kernel(5, 5, CV_32F);

    // 1st row
    kernel.at<float>(0,0) = 1.0f / 256.0f;
    kernel.at<float>(1,0) = 4.0f / 256.0f;
    kernel.at<float>(2,0) = 6.0f / 256.0f;
    kernel.at<float>(3,0) = 4.0f / 256.0f;
    kernel.at<float>(4,0) = 1.0f / 256.0f;

    // 2nd row
    kernel.at<float>(0,1) = 4.0f / 256.0f;
    kernel.at<float>(1,1) = 16.0f / 256.0f;
    kernel.at<float>(2,1) = 24.0f / 256.0f;
    kernel.at<float>(3,1) = 16.0f / 256.0f;
    kernel.at<float>(4,1) = 4.0f / 256.0f;

    // 3rd row
    kernel.at<float>(0,2) = 6.0f / 256.0f;
    kernel.at<float>(1,2) = 24.0f / 256.0f;
    kernel.at<float>(2,2) = 36.0f / 256.0f;
    kernel.at<float>(3,2) = 24.0f / 256.0f;
    kernel.at<float>(4,2) = 6.0f / 256.0f;


    // 4th row
    kernel.at<float>(0,3) = 4.0f / 256.0f;
    kernel.at<float>(1,3) = 16.0f / 256.0f;
    kernel.at<float>(2,3) = 24.0f / 256.0f;
    kernel.at<float>(3,3) = 16.0f / 256.0f;
    kernel.at<float>(4,3) = 4.0f / 256.0f;

    // 5th row
    kernel.at<float>(0,4) = 1.0f / 256.0f;
    kernel.at<float>(1,4) = 4.0f / 256.0f;
    kernel.at<float>(2,4) = 6.0f / 256.0f;
    kernel.at<float>(3,4) = 4.0f / 256.0f;
    kernel.at<float>(4,4) = 1.0f / 256.0f;

    return kernel;
}
