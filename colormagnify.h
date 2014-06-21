#ifndef COLORMAGNIFY_H
#define COLORMAGNIFY_H

#include <QObject>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "SpatialFilter.h"
#include <iostream>

//将滤波器类型声明为枚举类型
enum spatialFilterType {LAPLACIAN, GAUSSIAN};
enum temporalFilterType {IIR, IDEAL};

class ColorMagnify : public QObject
{
  Q_OBJECT
public:
  explicit ColorMagnify(QObject *parent = 0);

  //input frame
void setInput(cv::Mat &frame);

// create frame buffer
void createInputBuffer();

// return the size of the video frame
cv::Size getFrameSize();

// return the frame rate
double getFrameRate();

//设置帧率
void setFrameRate(double rate);

// set spatial filter
void setSpatialFilter(spatialFilterType type);

// set temporal filter
void setTemporalFilter(temporalFilterType type);

// color magnification
void colorMagnify();

//get output result
cv::Mat getOutput();

private:
//current frame
cv::Mat currentFrame;

// video frame rate
double rate;

// total number of frames
long length;

// current level of pyramid
int curLevel;

// number of digits in output image filename
int digits;
// extension of output images
std::string extension;
// spatial filter type
spatialFilterType spatialType;
// temporal filter type
temporalFilterType temporalType;
// level numbers of image pyramid
int levels;
// amplification factor
float alpha;

// cut-off wave length
float lambda_c;
// low cut-off
float fl;
// high cut-off
float fh;
// chromAttenuation
float chromAttenuation;
// delta
float delta;
// extraggon factor
float exaggeration_factor;
// lambda
float lambda;

// low pass filters for IIR
std::vector<cv::Mat> lowpass1;
std::vector<cv::Mat> lowpass2;

//temp frame vector
std::vector<cv::Mat> inputBuffer;
std::vector<cv::Mat> outputBuffer;

// get the next frame if any
bool getNextFrame(cv::Mat& frame);

// spatial filtering
bool spatialFilter(const cv::Mat &src, std::vector<cv::Mat> &pyramid);

// temporal filtering
void temporalFilter(const cv::Mat &src,
                    cv::Mat &dst);

// temporal IIR filtering
void temporalIIRFilter(const cv::Mat &src,
                    cv::Mat &dst);

// temporal ideal bandpass filtering
void temporalIdealFilter(const cv::Mat &src,
                         cv::Mat &dst);

// amplify motion
void amplify(const cv::Mat &src, cv::Mat &dst);

// attenuate I, Q channels
void attenuate(cv::Mat &src, cv::Mat &dst);

// concat images into a large Mat
void concat(const std::vector<cv::Mat> &frames, cv::Mat &dst);

// de-concat the concatnate image into frames
void deConcat(const cv::Mat &src, const cv::Size &frameSize, std::vector<cv::Mat> &frames);

// create an ideal bandpass processor
void createIdealBandpassFilter(cv::Mat &filter, double fl, double fh, double rate);


signals:
  void showMagnify(cv::Mat frame);
public slots:
  //负责调动整个类的运行
  void mainloop(cv::Mat &input);

};

#endif // COLORMAGNIFY_H
