#ifndef FACEDETECT_H
#define FACEDETECT_H

#include <QObject>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/video/video.hpp>

using namespace std;

class FaceDetect : public QObject
{
    Q_OBJECT
public:
    explicit FaceDetect(QObject *parent = 0);

    //检测人脸区域
    void detectFace(cv::Mat &Frame);

    //判断是否检测到人脸区域
    bool isDetected();

    //返回得到的人脸区域
    cv::Rect getWindow();

private:
    cv::CascadeClassifier cascade;
    string cascadeName;

    //用于存储检测的人脸区域
    cv::Rect detected;

private slots:
    //负责调用整个类的所有函数运行
    void run(cv::Mat &frame);

signals:

};

#endif // FACEDETECT_H
