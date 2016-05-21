#include "imageOperation.h"

Mat resizeImage(Mat image, const double width, int interpolation) {
    // Check if image has size
    if (image.rows == 0) {
        return image;
    }
    // Resize
    resize(image, image, getResizedSize(image.size(), width), 0, 0, interpolation);
    return image;
}

Size getResizedSize(const Size origSize, const double width) {
    // Keep aspect ratio
    double aspect = (double) origSize.width / origSize.height;
    return Size((int) round(width), (int) round(width / aspect));
}

int setDoubleDownscalingLevel(int origWidth, int resultWidth) {
    int doubleDownscalingLevel = 0;
    for (int i = 0; ; i++) {
        if (origWidth < resultWidth) {
            doubleDownscalingLevel = (abs(resultWidth - origWidth) < abs(resultWidth - (origWidth * 2))) ?
                                     i : i-1;
            break;
        }
        origWidth /= 2;
    }
    return doubleDownscalingLevel;
}

Mat resizeImage(Mat image, const double width) {
    return resizeImage(image, width, INTER_LINEAR);
}

void resizeCropVideo(vector<Mat> &video, int width) {
    for (int i = 0; i < video.size(); i++) {
        video[i] = resizeImage(video[i], width, INTER_LINEAR);
        video[i] = cropImageBorder(video[i], ERASED_BORDER_WIDTH);
    }
}

void pyrUpVideo(vector<Mat> &video, vector<Mat> &dst, Size size, int level) {
    for (int i = 0; i < video.size(); i++) {
        for (int j = 0; j < level; j++) {
            pyrUp(video[i], video[i]);
        }
        dst.push_back(video[i]);
        unifyMatSize(dst[i], size);
    }
}

void unifyMatSize(Mat & frame, Size unifiedSize) {
    // Control if adding border needed
    int widthDiff = unifiedSize.width - frame.cols ;
    int heightDiff = unifiedSize.height - frame.rows ;

    if (widthDiff == 0 && heightDiff == 0) return;

    if (widthDiff > 0 && heightDiff > 0) {
        copyMakeBorder(frame, frame, 0, heightDiff, 0, widthDiff, BORDER_REPLICATE);
    } else if (widthDiff < 0 && heightDiff < 0) {
        frame = frame(Rect(0, 0, frame.cols + widthDiff, frame.rows + heightDiff));
    } else if (widthDiff < 0) {
        frame = frame(Rect(0, 0, frame.cols + widthDiff, frame.rows));
        copyMakeBorder(frame, frame, 0, heightDiff, 0, 0, BORDER_REPLICATE);
    } else if (heightDiff < 0) {
        frame = frame(Rect(0, 0, frame.cols, frame.rows + heightDiff));
        copyMakeBorder(frame, frame, 0, 0, 0, widthDiff, BORDER_REPLICATE);
    }
}

void normalizeVid(vector<Mat> &video, int min, int max, int type) {
    for (int i = 0; i < video.size(); i++) {
        normalize(video[i], video[i], min, max, type);
    }
}

void cropToVideo(vector<Mat> src, vector<Mat>& dst, int borderWidth) {
    for (int i = 0; i < src.size(); i++) {
        Mat tmp = src[i];
        dst.push_back(cropImageBorder(tmp, borderWidth));
    }
}

void cropToVideo(vector<Mat> src, vector<Mat>& dst, Rect roi) {
    // Check roi placement
    handleRoiPlacement(roi, src[0].size());

    for (int i = 0; i < src.size(); i++) {
        Mat tmp = src[i];
        dst.push_back(tmp(roi));
    }
}


void handleRoiPlacement(Rect &roi, const Size frame) {
    handleRoiPlacement(roi, frame, 0);
}

void handleRoiPlacement(Rect &roi, const Size frame, int erasedBorder) {
    roi.x  = (roi.x < 0) ? 0 : roi.x;
    roi.y  = (roi.y < 0) ? 0 : roi.y;

    int maxXIndex = roi.x + roi.width + erasedBorder;
    int maxYIndex = roi.y + roi.height + erasedBorder;

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
    /* convert image from RGB to YIQ */

    int m=0, n=0;
    for(m=0; m<img.rows; m++)
    {
        for(n=0; n<img.cols; n++)
        {
            y = img.data[m*img.step + n*3 + 2];
            i = img.data[m*img.step + n*3 + 1];
            q = img.data[m*img.step + n*3 ];

            // TODO: Check data types

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

void amplifyChannels(Mat& frame, float r, float g, float b) {
    // MASKING
    vector<Mat> channels;
    // Real & imag part
    split(frame, channels);


    channels[RED_CHANNEL] = channels[RED_CHANNEL] * r;
    channels[GREEN_CHANNEL] = channels[GREEN_CHANNEL] * g;
    channels[BLUE_CHANNEL] = channels[BLUE_CHANNEL] * b;

    merge(channels, frame);
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

float freqToBpmMapper(double fps, int framesCount, int index) {
    return (int) (round((SECONDS_IN_MINUTE * (float) fps * (index+1)) / framesCount));
}

float findStrongestRowFreq(vector<double> row, int framesCount, double fps) {
    // Create matrix from intensitySum
    float maxValue = 0;
    float minValue = 0;
    for (int i = 0; i < row.size(); i++) {
        maxValue = (row[i] > maxValue) ? row[i] : maxValue;
        minValue = (row[i] < minValue) ? row[i] : minValue;
    }
    maxValue += minValue;

    Mat rowMat = Mat::zeros(1, row.size(), CV_32F);

    // Normalization
    for (int i = 0; i < row.size(); i++) {
        rowMat.at<float>(i) += minValue;
        rowMat.at<float>(i) = row[i] / maxValue;
    }

    return findStrongestRowFreq(rowMat, framesCount, fps);
}

float findStrongestRowFreq(Mat row, int framesCount, double fps) {
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
        if (bpm < CUTOFF_FL) continue; // This is under low frequency
        if (bpm > CUTOFF_FH) continue; // This is over high frequency

        if (magI.at<float>(i) > maxFreq) {
            maxFreq = magI.at<float>(i);
            maxFreqLoc = i;
        }
    }
    return freqToBpmMapper(fps, framesCount, maxFreqLoc);
}


Mat generateFreqMask(float freq, int framesCount, double fps) {
    float halfRange = FREQ_MASK_WIDTH / 2.0f;
    float fl, fh;

    // Compute fl & fh
    if (freq - halfRange < CUTOFF_FL) {
        fl = CUTOFF_FL;
        fh = CUTOFF_FH + FREQ_MASK_WIDTH;
    }
    else if (freq + halfRange > CUTOFF_FH) {
        fh = CUTOFF_FH;
        fl = CUTOFF_FH - FREQ_MASK_WIDTH;
    }
    else {
        fl = freq - halfRange;
        fh = freq + halfRange;
    }

    return maskingCoeffs(framesCount,  fl, fh, fps);
}

Mat maskingCoeffs(int width, float fl, float fh, double fps) {
    Mat row(1, width, CV_32FC1);

    // Create row 0.25 - 0.5 ----- FRAME RATE
    for (int i = 0; i < width; i++) {
        int bpm = freqToBpmMapper(fps, width, i);
        float value = (bpm < fl || bpm > fh) ? 0 : 1;
        row.at<float>(0, i) = value;
    }

    return row;
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

// Move outside
vector<double> countIntensities(vector<Mat> video) {
    return countIntensities(video, 1, 1, 1);
}

vector<double> countIntensities(vector<Mat> video, float r, float g, float b) {
    vector <double> intensities(video.size());
    for (int i = 0; i < video.size(); i ++) {
        Scalar frameSum = sum(video[i]);
        intensities.at(i) =
            r * frameSum[RED_CHANNEL] +
            g * frameSum[GREEN_CHANNEL] +
            b * frameSum[BLUE_CHANNEL];
    }
    return intensities;
}

vector<double> countOutsideIntensities(vector<Mat> video, Rect face, float r, float g, float b, int computeType) {
    vector <double> intensities(video.size());
    int stripHeight = (int) round((video[0].rows * 0.5));
    for (int i = 0; i < video.size(); i ++) {
        // Merge 2 side strips - consedering as bg
        Mat strips;
        hconcat(video[i](Rect(0,0,face.x, stripHeight)).clone(), video[i](Rect(face.x + face.width, 0, video[0].cols - (face.x + face.width), stripHeight)).clone(), strips);

        if (DATA_LOGGING) {
            imwrite( (string) PROJECT_DIR+"/images/forehead/strips"+to_string(i)+".jpg", strips);
        }

        Scalar frameMeans, frameSum;
        switch (computeType) {
            case AVG_COMPUTE:
                frameSum = sum(strips);
                intensities.at(i) =
                    r * frameSum[RED_CHANNEL] +
                    g * frameSum[GREEN_CHANNEL] +
                    b * frameSum[BLUE_CHANNEL];
            break;
            case MEDIAN_COMPUTE:
                frameMeans = mean(strips);
                intensities.at(i) =
                    r * frameMeans[RED_CHANNEL] +
                    g * frameMeans[GREEN_CHANNEL] +
                    b * frameMeans[BLUE_CHANNEL];
            break;
            default:
                frameMeans = mean(strips);
                intensities.at(i) = frameMeans[GREEN_CHANNEL];
            break;
        }

    }
    return intensities;
}

vector<double> countMedianValues(vector<Mat> video, int channel) {
    vector <double> values(video.size());
    for (int i = 0; i < video.size(); i++) {
        Scalar color = mean(video[i]);
        values.at(i) = color[channel];
    }
    return values;
}

void saveIntensities(vector<double> intensities, string filename) {
    ofstream myfile;
    myfile.open(filename, ios::out);
    cout << filename;
    for (int i = 0; i < intensities.size(); i++) {
        myfile << intensities.at(i);
        myfile << "\n";
    }
    myfile.close();
}


void suppressGlobalChanges(vector<double>& localIntensities, vector<double> globalIntensities) {
    saveIntensities(localIntensities, ((string) DATA_DIR)+"/localOrig.txt");

    saveIntensities(globalIntensities, ((string) DATA_DIR)+"/global.txt");

    for(int i = 0; i < localIntensities.size(); i++) {
        localIntensities[i] -= globalIntensities[i];
    }

    saveIntensities(localIntensities, ((string) DATA_DIR)+"/localAfterSupression.txt");
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

void printIterationRow(vector<Mat> video, int framesCount, double fps, int realBpm, ofstream &file) {
    vector<int> bpms(4);
    // [red, greeb, blue, rgb]
    bpms[0] = (int) round(findStrongestRowFreq(countIntensities(video, 1, 0, 0), framesCount, fps));
    bpms[1] = (int) round(findStrongestRowFreq(countIntensities(video, 0, 1, 0), framesCount, fps));
    bpms[2] = (int) round(findStrongestRowFreq(countIntensities(video, 0, 0, 1), framesCount, fps));
    bpms[3] = (int) round(findStrongestRowFreq(countIntensities(video), framesCount, fps));

    int closestValue = abs(bpms[0] - realBpm);
    for (int i = 1; i < bpms.size(); i++) {
        closestValue = (abs(bpms[i] - realBpm) < closestValue) ?
            abs(bpms[i] - realBpm) :
            closestValue;
    }

    // Print to file
    for (int i = 0; i < bpms.size(); i++) {
        // If closest value print different
        if (bpms[i] == closestValue) {
            file << ">> ";
            file << bpms[i];
            file << " <<";
        } else {
            file << bpms[i];
        }
        file << ",";

        // Print differnce
        file << abs(bpms[i]-realBpm);
        if (i != (int) (bpms.size() - 1)) {
            file <<",";
        }
    }
    file << "\n";
}

void printIterationHead(ofstream &file) {
    file << "RED";
    file << ", ";
    file << "RED diff";
    file << ",";
    file << "GREEN";
    file << ", ";
    file << "GREEN diff";
    file << ",";
    file << "BLUE";
    file << ", ";
    file << "BLUE diff";
    file << ",";
    file << "RGB";
    file << ", ";
    file << "RGB diff";
    file << "\n";
}

Point2d getCenter(Size size) {
    return Point2d(size.width/2, size.height/2);
}

double getDistance(Point2d a, Point2d b) {
    return sqrt(pow(abs(a.x - b.x),2) + pow(abs(a.y - b.y),2));
}

void printRectOnFrame(Mat &frame, Rect rect, Scalar color) {
    rectangle( frame, Point(rect.x, rect.y), Point(rect.x + rect.width, rect.y + rect.height), color);
}

void fillRoiInVideo(vector<Mat> src, vector<Mat> & dst, Rect roi, Scalar color) {
    roi.y = 0;
    roi.height = src[0].rows;

    handleRoiPlacement(roi, src[0].size());
    for (int i = 0; i < src.size(); i++) {
        dst.push_back(src[i]);
        rectangle(dst[i], roi, color, CV_FILLED);

        rectangle(dst[i], Rect(0, src[i].rows - 200, src[i].cols, 200), color, CV_FILLED);
    }
}
