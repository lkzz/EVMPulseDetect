#ifndef FACETRACK_H
#define FACETRACK_H

#include <QObject>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/video/video.hpp>

class FaceTrack : public QObject
{
    Q_OBJECT
public:
    explicit FaceTrack(QObject *parent = 0);
    //参数设置
    void setVmin(int min);
    void setVmax(int max);
    void setWidth(int width);
    void setHeight(int height);

    //camshift处理过程,返回跟踪到的区域
    cv::RotatedRect camshiftProcess(cv::Mat &frame, cv::Rect &ROI);

    //进行卡尔曼滤波处理
    void kalmanProcess();

    //旋转得到的camshift区域
    cv::Mat rotatedProcess(cv::Mat &frame,cv::RotatedRect &rotRect);

    //将旋转后的区域变换为指定大小
    cv::Mat uniformSize(cv::Mat &input);
    //获取最终的结果
    cv::Mat getTrackBox();


    //画出跟踪到的区域
    void plotRect(cv::Mat &frame, cv::RotatedRect &trackBox);
signals:

public slots:
    void run(cv::Mat &frameIn, cv::Rect &faceRect);

private:
    //camshift跟踪
    int vmin;
    int vmax;
    int smin;
    int histSize;
    cv::Mat hsv;
    cv::Mat hue;
    cv::Mat mask;
    cv::Mat hist;
    //用于画出直方图
    bool showHist;
    cv::Mat histImg;
    cv::Mat backProject;

    //卡尔曼滤波
    cv::KalmanFilter *klFilter;
    const int stateNum;
    const int measureNum;
    cv::Mat measurement;
    cv::Mat statePost;
    cv::Point2f camCenter;
    cv::Point2f KFPredictCenter;
    cv::Point2f KFCorrectCenter;

    cv::RotatedRect trackBox;

    //期望输出的区域的宽度和宽度
    int globalwidth;
    int globalheight;
    //用于保存最终的结果
    cv::Mat uniMat;
};

#endif // FACETRACK_H
