//
// Created by Michal on 30/01/16.
//

#include "amplify.h"

#define BLUE_CHANNEL 0
#define GREEN_CHANNEL 1
#define RED_CHANNEL 2


void amplifySpatial(const vector<Mat> video, vector<Mat>& out, int & bpm, double alpha, int lowLimit, int highLimit, int framesCount, int level) {

    // Allocate stack
    vector<Mat> stack;

    // Create gdown stack
    buildGDownStack(video, stack, framesCount, level);

    // Filtering
    bandpass(stack, out, bpm, lowLimit, highLimit, framesCount);

    // Save
    // saveIntensities(stack, "int.txt");

    // Analyze intensities
    bpm = findStrongestRowFreq(countIntensities(out));

    // Clear data
    stack.clear();
}

// Based on https://github.com/diego898/matlab-utils/blob/master/toolbox/EVM_Matlab/build_GDown_stack.m
// TODO: Check wheter rgb separate channels needed
void buildGDownStack(const vector<Mat> video, vector<Mat>& stack, int framesCount, int level) {
    Mat kernel = binom5Kernel();
    for (int i = 0; i < framesCount; i++) {
        Mat out = video[i].clone();

        // TODO: REWRITE ctColor2 to float
        cvtColor2(out, out, CV2_BGR2YIQ); // returns CV_8UC3

        // TODO: This solves rounding to int at first and than back to float
        out.convertTo(out, CV_32FC3);

        blurDn(out, level, kernel);

        out.convertTo(out, CV_8UC3);

        cvtColor2(out, out, CV2_YIQ2BGR); // returns CV_8UC3

        out.convertTo(out, CV_32FC3);

        stack.push_back(out);

    }
    kernel.release();
}

/**
 * Downsample 2x and blur image in pyramid level
 * TODO: Blurring should be via  binomial filter see link
 * http://optica.csic.es/projects/tools/steer/1.matlabPyrTools/namedFilter.html

 * Each chanel separate
 */
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

void bandpass(vector<Mat>& video, vector<Mat>&out, int & bpm, int lowLimit, int highLimit, int framesCount) {
    // TODO: Describe
    int height =  video[0].size().height;
    int width =  video[0].size().width;

    // TODO: Connect with main class
    // http://vgg.fiit.stuba.sk/2012-05/frequency-domain-filtration/
    int fl = 60/60; // Low freq cut-off
    int fh = 120/60; // High freg cut-off


    // Create time stack change
    vector <vector<Mat> > temporalSpatialStack(3);

    // Must be in color channels
    // TODO: Merge
    createTemporalSpatial(video, temporalSpatialStack, RED_CHANNEL);
    createTemporalSpatial(video, temporalSpatialStack, GREEN_CHANNEL);
    createTemporalSpatial(video, temporalSpatialStack, BLUE_CHANNEL);

    int bruteBpmSum = 0;
    int numOfSamples = 0;

    // First of all we need to find strongest frequency for all
    float strongestTimeStackFreq = findStrongestRowFreq(countIntensities(video));

    // Create mask based on strongest frequency
    Mat mask = maskingCoeffs(video.size(),  strongestTimeStackFreq-15, strongestTimeStackFreq+15);

    for (int i = 0; i < temporalSpatialStack[0].size(); i++) {
        for (int channel = 0; channel < 3; channel++) {
            for (int row = 0; row < temporalSpatialStack[channel][i].rows; row++) {
                // FFT
                Mat fourierTransform;
                dft(temporalSpatialStack[channel][i].row(row), fourierTransform, cv::DFT_SCALE | cv::DFT_COMPLEX_OUTPUT);

                // MASKING
                Mat planes[] = {Mat::zeros(fourierTransform.size(), CV_32F), Mat::zeros(fourierTransform.size(), CV_32F)};

                // Real & imag part
                split(fourierTransform, planes);

                // Masking parts
//                planes[0] = planes[0].mul(mask);
//                planes[1] = planes[1].mul(mask);


                // Stretch
//                normalize(planes[0], planes[0], 0, 255);

                // Merge back
                merge(planes, 2, fourierTransform);

                // IFFT
                dft(fourierTransform, fourierTransform, cv::DFT_INVERSE|cv::DFT_REAL_OUTPUT);

                // COPY BACK
                fourierTransform.copyTo(temporalSpatialStack[channel][i].row(row));


                // RELEASE
                fourierTransform.release();
            }
        }
    }
    bpm = round(strongestTimeStackFreq);
    inverseTemporalSpatial(temporalSpatialStack, out);
}


float findStrongestTimeStackFreq(vector <vector<Mat> > timeStack) {
    int bruteBpmSum = 0;
    int numOfSamples = 0;

    vector <int> histogram(timeStack[0].size() * 3 * timeStack[0][0].rows);
    fill(histogram.begin(), histogram.end(), 0);

    for (int i = 0; i < timeStack[0].size(); i++) {
        for (int channel = 0; channel < 3; channel++) {
            for (int row = 0; row < timeStack[channel][i].rows; row++) {

                // Strongest frequency in row
                float rowFreq = findStrongestRowFreq(timeStack[channel][i].row(row).clone());

                // Store in histogram
                histogram.at((int)rowFreq) = histogram.at((int)rowFreq) + 1;

                // Aggregate all rows strongest freqs
                bruteBpmSum += rowFreq;

                // Increment samples counter
                numOfSamples++;

            }
        }
    }

    // Prepared for finding most used freq
    std::vector<int>::iterator result;
    result = max_element(histogram.begin(), histogram.end());
    auto frequency = distance(histogram.begin(), result);

    // Average frequency
    float averageFreq = bruteBpmSum / (float)numOfSamples;

    return averageFreq;
}

// Assume video is single channel
void createTemporalSpatial(vector<Mat> &video, vector<vector<Mat> > &dst, int channel) {

    // DST vector
    // video[0].size().width - vectors count
    // width: video.size()
    // height video[0].size().height
    int dstVectorCount = video[0].size().width;
    int dstVectorWidth = (int) video.size();
    int dstVectorHeight = video[0].size().height;

    for (int i = 0; i < dstVectorCount; i++) {
        // One frame
        Mat frame(dstVectorHeight, dstVectorWidth, CV_32FC1);
        for (int j = 0; j < dstVectorWidth; j++) {
            vector<Mat> channels;
            video[j].convertTo(video[j], CV_32FC3);
            split(video[j],channels);

            for(int k = 0; k < dstVectorHeight; k++) {
                frame.at<float>(k,j) = channels[channel].at<float>(k,i);
            }
        }
        dst[channel].push_back(frame);
        frame.release();
    }
}

void inverseTemporalSpatial(vector<vector<Mat> > &stack, vector<Mat> &dst) {

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

        // Merge channels into colorFrame
        Mat outputFrame;
        merge(channels, outputFrame);

        // Convert to basic CV_8UC3
        outputFrame.convertTo(outputFrame, CV_8UC3);

        // Amplify frame's channels
        amplifyChannels(channels, 2, 0, 0);

        // in range [0,255]
        normalize(outputFrame, outputFrame, 0, 255, NORM_MINMAX );

        dst.push_back(outputFrame);
        channels.clear();
        outputFrame.release();
    }
}


// Checked - same behavior as in matlab
Mat maskingCoeffs(int width, float fl, float fh) {
    Mat row(1, width, CV_32FC1);

    // Create row 0.25 - 0.5 ----- FRAME RATE
    for (int i = 0; i < width; i++) {
        int bpm = freqToBpmMapper(FRAME_RATE, BUFFER_FRAMES, i);
        float value = (bpm < fl || bpm > fh) ? 0 : 1;
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
    myfile.open((string) PROJECT_DIR + "/"  + filename, ios::out);

    vector<int> intensitySum = countIntensities(video);

    for (int i = 0; i < video.size(); i++) {
        myfile << intensitySum.at(i);
        myfile << "\n";
    }
    myfile.close();
}

float findStrongestRowFreq(vector<int> row) {
    // Create matrix from intensitySum

    // TODO: Function for normalize
    // Mat rowMat = stdNormalize(row);
    float maxValue = 0;
    for (int i = 0; i < row.size(); i++) {
        maxValue = (row[i] > maxValue) ? row[i] : maxValue;
    }

    Mat rowMat = Mat::zeros(1, row.size(), CV_32F);
    for (int i = 0; i < row.size(); i++) {
        rowMat.at<float>(i) = row[i] / maxValue;
    }

    return findStrongestRowFreq(rowMat);
}


float findStrongestRowFreq(Mat row) {
    // DFT of intensities
    // Find max value & locaiton
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
    for (int i = 1; i < BUFFER_FRAMES; i++) {
        bpm = freqToBpmMapper(FRAME_RATE, BUFFER_FRAMES, i);

        // TODO: This should be connected with bpm!
        // TODO: Define cut-off freq to constants
        if (bpm < 50) continue; // This is under low frequency
        if (bpm > 180) continue; // This is over high frequency

        if (magI.at<float>(i) > maxFreq) {
            maxFreq = magI.at<float>(i);
            maxFreqLoc = i;
        }
    }
    return freqToBpmMapper(FRAME_RATE, BUFFER_FRAMES, maxFreqLoc);
}


float freqToBpmMapper(int fps, int framesCount, int index) {
    return (int) (round(SECONDS_IN_MINUTE * fps * (index+1)) / framesCount);
}

void resizeCropVideo(vector<Mat> &video, int width) {
    for (int i = 0; i < BUFFER_FRAMES; i++) {
        video[i] = resizeImage(video[i], width, INTER_LINEAR);
        video[i] = cropImageBorder(video[i], ERASED_BORDER_WIDTH);
    }
}

/**
* TODO: Move outside!!!
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
