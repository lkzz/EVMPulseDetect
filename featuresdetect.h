#ifndef FEATURESDETECT_H
#define FEATURESDETECT_H

#include <QObject>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/highgui/highgui.hpp>

class FeaturesDetect : public QObject
{
  Q_OBJECT
public:
  explicit FeaturesDetect(QObject *parent = 0);

    void setMaxNum(int num);
private:

  //上一帧
  cv::Mat lastFrame;

  cv::Mat grayFirst;
  cv::Mat gray;
  cv::Mat grayPrevious;


  // keep the good points
  std::vector<cv::Point2f> initialPoints;
  std::vector<cv::Point2f> trackedPoints;
  std::vector<uchar> status; // status of tracked features
  std::vector<float> err;    // error in tracking
  std::vector<cv::Point2f> pointsToTrack[2];

  //max number of feature points
  int maxNum;

signals:

public slots:
  void mainloop(cv::Mat &frame);
};

#endif // FEATURESDETECT_H
