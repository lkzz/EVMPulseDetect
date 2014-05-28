#ifndef FACETRACK_H
#define FACETRACK_H

#include <QObject>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/video/video.hpp>

class FaceTrack : public QObject
{
    Q_OBJECT
public:
    explicit FaceTrack(QObject *parent = 0);

signals:

public slots:
    void run(cv::Mat &frame,cv::Rect &faceRect);

private:
    cv::Mat hsvROI;

};

#endif // FACETRACK_H
