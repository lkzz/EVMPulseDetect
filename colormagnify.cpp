#include "colormagnify.h"

ColorMagnify::ColorMagnify(QObject *parent) :
  QObject(parent),
  rate(0),
  length(50),
  curLevel(0),
  digits(0),
  levels(5),
  extension(".avi"),
  alpha(1000),
  fl(0.83),
  fh(10.0),
  lambda_c(80),
  chromAttenuation(0.1),
  delta(0),
  exaggeration_factor(0.2),
  lambda(0)
{
}

void ColorMagnify::setInput(cv::Mat &frame)
{
  frame.copyTo(this->currentFrame);
}

void ColorMagnify::createInputBuffer()
{

  inputBuffer.push_back(this->currentFrame);
  if(inputBuffer.size() > this->length)
    {
      std::vector<cv::Mat> tempVector;
      std::vector<cv::Mat>::iterator iterEnd = inputBuffer.end();
      std::vector<cv::Mat>::iterator iterBegin = iterEnd - length;
      tempVector.assign(iterBegin,iterEnd);
      inputBuffer.swap(tempVector);
    }
}

cv::Size ColorMagnify::getFrameSize()
{
  return this->currentFrame.size();
}

/**
 * spatialFilter	-	spatial filtering an image
 *
 * @param src		-	source image
 * @param pyramid	-	destinate pyramid
 */
bool ColorMagnify::spatialFilter(const cv::Mat &src, std::vector<cv::Mat> &pyramid)
{
    switch (spatialType) {
    case LAPLACIAN:     // laplacian pyramid
        return buildLaplacianPyramid(src, levels, pyramid);
        break;
    case GAUSSIAN:      // gaussian pyramid
        return buildGaussianPyramid(src, levels, pyramid);
        break;
    default:
        return false;
        break;
    }
}

/**
 * temporalFilter	-	temporal filtering an image
 *
 * @param src	-	source image
 * @param dst	-	destinate image
 */
void ColorMagnify::temporalFilter(const cv::Mat &src,
                                    cv::Mat &dst)
{
    switch(temporalType) {
    case IIR:       // IIR bandpass filter
        temporalIIRFilter(src, dst);
        break;
    case IDEAL:     // Ideal bandpass fColorMagnifyilter
        temporalIdealFilter(src, dst);
        break;
    default:
        break;
    }
    return;
}

/**
 * temporalIIRFilter	-	temporal IIR filtering an image
 *                          (thanks to Yusuke Tomoto)
 * @param pyramid	-	source image
 * @param filtered	-	filtered result
 *
 */
void ColorMagnify::temporalIIRFilter(const cv::Mat &src,
                                    cv::Mat &dst)
{
    cv::Mat temp1 = (1-fh)*lowpass1[curLevel] + fh*src;
    cv::Mat temp2 = (1-fl)*lowpass2[curLevel] + fl*src;
    lowpass1[curLevel] = temp1;
    lowpass2[curLevel] = temp2;
    dst = lowpass1[curLevel] - lowpass2[curLevel];
}

/**
 * temporalIdalFilter	-	temporal IIR filtering an image pyramid of concat-frames
 *                          (Thanks to Daniel Ron & Alessandro Gentilini)
 *
 * @param pyramid	-	source pyramid of concatenate frames
 * @param filtered	-	concatenate filtered result
 *
 */
void ColorMagnify::temporalIdealFilter(const cv::Mat &src,
                                          cv::Mat &dst)
{
    cv::Mat channels[3];

    // split into 3 channels
    cv::split(src, channels);

    for (int i = 0; i < 3; ++i){

        cv::Mat current = channels[i];  // current channel
        cv::Mat tempImg;

        int width = cv::getOptimalDFTSize(current.cols);
        int height = cv::getOptimalDFTSize(current.rows);

        cv::copyMakeBorder(current, tempImg,
                           0, height - current.rows,
                           0, width - current.cols,
                           cv::BORDER_CONSTANT, cv::Scalar::all(0));

        // do the DFT
        cv::dft(tempImg, tempImg, cv::DFT_ROWS | cv::DFT_SCALE);

        // construct the filter
        cv::Mat filter = tempImg.clone();
        createIdealBandpassFilter(filter, fl, fh, rate);

        // apply filter
        cv::mulSpectrums(tempImg, filter, tempImg, cv::DFT_ROWS);

        // do the inverse DFT on filtered image
        cv::idft(tempImg, tempImg, cv::DFT_ROWS | cv::DFT_SCALE);

        // copy back to the current channel
        tempImg(cv::Rect(0, 0, current.cols, current.rows)).copyTo(channels[i]);
    }
    // merge channels
    cv::merge(channels, 3, dst);

    // normalize the filtered image
    cv::normalize(dst, dst, 0, 1, CV_MINMAX);
}

/**
 * amplify	-	ampilfy the motion
 *
 * @param filtered	- motion image
 */
void ColorMagnify::amplify(const cv::Mat &src, cv::Mat &dst)
{
    float currAlpha;
    switch (spatialType) {
    case LAPLACIAN:
        //compute modified alpha for this level
        currAlpha = lambda/delta/8 - 1;
        currAlpha *= exaggeration_factor;
        if (curLevel==levels || curLevel==0)     // ignore the highest and lowest frequency band
            dst = src * 0;
        else
            dst = src * cv::min(alpha, currAlpha);
        break;
    case GAUSSIAN:
        dst = src * alpha;
        break;
    default:
        break;
    }
}

/**
 * concat	-	concat all the frames into a single large Mat
 *              where each column is a reshaped single frame
 *
 * @param frames	-	frames of the video sequence
 * @param dst		-	destinate concatnate image
 */
void ColorMagnify::concat(const std::vector<cv::Mat> &frames,
                            cv::Mat &dst)
{
    cv::Size frameSize = frames.at(0).size();
    cv::Mat temp(frameSize.width*frameSize.height, length, CV_32FC3);
    for (int i = 0; i < length; ++i) {
        // get a frame if any
        cv::Mat input = frames.at(i);
        // reshape the frame into one column
        cv::Mat reshaped = input.reshape(3, input.cols*input.rows).clone();
        cv::Mat line = temp.col(i);
        // save the reshaped frame to one column of the destinate big image
        reshaped.copyTo(line);
    }
    temp.copyTo(dst);
}

/**
 * deConcat	-	de-concat the concatnate image into frames
 *
 * @param src       -   source concatnate image
 * @param framesize	-	frame size
 * @param frames	-	destinate frames
 */
void ColorMagnify::deConcat(const cv::Mat &src,
                              const cv::Size &frameSize,
                              std::vector<cv::Mat> &frames)
{
    for (int i = 0; i < length; ++i) {    // get a line if any
        cv::Mat line = src.col(i).clone();
        cv::Mat reshaped = line.reshape(3, frameSize.height).clone();
        frames.push_back(reshaped);
    }
}

/**
 * createIdealBandpassFilter	-	create a 1D ideal band-pass filter
 *
 * @param filter    -	destinate filter
 * @param fl        -	low cut-off
 * @param fh		-	high cut-off
 * @param rate      -   sampling rate(i.e. video frame rate)
 */
void ColorMagnify::createIdealBandpassFilter(cv::Mat &filter, double fl, double fh, double rate)
{
    int width = filter.cols;
    int height = filter.rows;

    fl = 2 * fl * width / rate;
    fh = 2 * fh * width / rate;

    double response;

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            // filter response
            if (j >= fl && j <= fh)
                response = 1.0f;
            else
                response = 0.0f;
            filter.at<float>(i, j) = response;
        }
    }
}


/**
 * setSpatialFilter	-	set the spatial filter
 *
 * @param type	-	spatial filter type. Could be:
 *					1. LAPLACIAN: laplacian pyramid
 *					2. GAUSSIAN: gaussian pyramid
 */
void ColorMagnify::setSpatialFilter(spatialFilterType type)
{
    spatialType = type;
}

/**
 * setTemporalFilter	-	set the temporal filter
 *
 * @param type	-	temporal filter type. Could be:
 *					1. IIR: second order(IIR) filter
 *					2. IDEAL: ideal bandpass filter
 */
void ColorMagnify::setTemporalFilter(temporalFilterType type)
{
    temporalType = type;
}

void ColorMagnify::setFrameRate(double rate)
{
  this->rate = rate;
}

double ColorMagnify::getFrameRate()
{
  return this->rate;
}

/**
 * colorMagnify	-	color magnification
 *
 */
void ColorMagnify::colorMagnify()
{
    // set filter
    setSpatialFilter(GAUSSIAN);
    setTemporalFilter(IDEAL);

    // current frame
    cv::Mat input;
    // output frame
    cv::Mat output;
    // motion image

    cv::Mat motion;
    // temp image
    cv::Mat temp;

    // down-sampled frames
    std::vector<cv::Mat> downSampledFrames;
    // filtered frames
    std::vector<cv::Mat> filteredFrames;

    // concatenate image of all the down-sample frames
    cv::Mat videoMat;
    // concatenate filtered image
    cv::Mat filtered;

    // 1. spatial filtering
    int num=0;
    while (num < this->length) {
        input = this->inputBuffer.at(num).clone();
        input.convertTo(temp, CV_32FC3);
        // spatial filtering
        std::vector<cv::Mat> pyramid;
        spatialFilter(temp, pyramid);
        downSampledFrames.push_back(pyramid.at(levels-1));
        // update process
    ++num;
    }

    // 2. concat all the frames into a single large Mat
    // where each column is a reshaped single frame
    // (for processing convenience)
    concat(downSampledFrames, videoMat);

    // 3. temporal filtering
    temporalFilter(videoMat, filtered);

    // 4. amplify color motion
    amplify(filtered, filtered);

    // 5. de-concat the filtered image into filtered frames
    deConcat(filtered, downSampledFrames.at(0).size(), filteredFrames);

    // 6. amplify each frame
    // by adding frame image and motions
    // and write into video

    std::cout<<filteredFrames.size();
    for (int i=0; i<length; ++i) {
        // up-sample the motion image
        upsamplingFromGaussianPyramid(filteredFrames.at(i), levels, motion);
        cv::resize(motion,motion,cv::Size(inputBuffer.at(i).cols,inputBuffer.at(i).rows),CV_INTER_CUBIC);
        motion.convertTo(motion,inputBuffer.at(i).type());
        temp = inputBuffer.at(i) + motion;
        output = temp.clone();
        double minVal, maxVal;
        minMaxLoc(output, &minVal, &maxVal); //find minimum and maximum intensities
        output.convertTo(output, CV_8UC3, 255.0/(maxVal - minVal),
                  -minVal * 255.0/(maxVal - minVal));


        this->outputBuffer.push_back(output);
        if((int)outputBuffer.size() > this->length)
          {
            std::vector<cv::Mat> tempVector;
            std::vector<cv::Mat>::iterator iterEnd = outputBuffer.end();
            std::vector<cv::Mat>::iterator iterBegin = iterEnd - length;
            tempVector.assign(iterBegin,iterEnd);
            outputBuffer.swap(tempVector);
          }
    }
 }

//返回buffer的顶层 frame
cv::Mat ColorMagnify::getOutput()
{
  return outputBuffer.at(length-1);
}

//接收信号
void ColorMagnify::mainloop(cv::Mat &input)
{
  this->setInput(input);
  this->createInputBuffer();
  if(!(this->inputBuffer.size() < this->length))
    {
      this->colorMagnify();
      emit showMagnify(outputBuffer.at(0));
    }
}
