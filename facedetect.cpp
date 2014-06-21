#include "facedetect.h"

FaceDetect::FaceDetect(QObject *parent) :
    QObject(parent)
{
    //导入训练文件，该文件为opencv自带文件
    cascadeName = "/home/kevin/program/EVMPulseDetect/EVMPulseDetect/haarcascades/haarcascade_frontalface_alt2.xml";
}

//感兴趣区域检测
void FaceDetect::detectFace(cv::Mat &frame)
{
    if(!cascade.load(this->cascadeName))
    {
        cout<<"无法加载级联分类器文件！"<<endl;
    }

    //转化为灰度图片，用于harris特征检测
    cv::Mat gray;
    cv::cvtColor(frame,gray,CV_RGB2GRAY);
    //加入直方图均衡
    cv::equalizeHist(gray,gray);
    std::vector<cv::Rect> faces;

    //建立缩小的图片，加快检测速度
    int scale = 3;
    cv::Mat smallImage(cvRound(gray.rows/scale),cvRound(gray.cols/scale),CV_8UC1);
    //改变图像大小，使用双线性差值
    cv::resize(gray,smallImage,smallImage.size(),0,0,CV_INTER_LINEAR);
    //多尺度的人脸检测
    cascade.detectMultiScale(smallImage,faces,1.2,4,CV_HAAR_SCALE_IMAGE,cv::Size(50,50));//

//    //定义一个矩阵用来存储检测到的一个人脸
//    cv::Mat faceROI(smallImage.rows*scale,smallImage.cols*scale,CV_8UC3,cv::Scalar(0,0,0));
//    //定义一个矩阵用来存储检测到的前额区域
//    cv::Mat foreheadROI(smallImage.rows*scale,smallImage.cols*scale,CV_8UC3,cv::Scalar(0,0,0));

    for(int i=0;i<(int)faces.size();i++)
    {
        double temp = faces[i].width * scale * 0.20;       //我们只用原来窗口大小的60%大小宽度，高度不变
        double lx = cvRound(faces[i].x * scale + temp);
        double ly = cvRound(faces[i].y * scale);
        cv::Point leftUpCorner(lx,ly);    //左上角

        double rx = cvRound(faces[i].x * scale + faces[i].width * scale - temp);
        double ry = cvRound(faces[i].y * scale + faces[i].height * scale);
        cv::Point rightDownCorner(rx,ry);    //右下角
        cv::rectangle(frame,leftUpCorner,rightDownCorner,cv::Scalar(0,255,0),2,8,0);        //在帧上画出矩形框

        //获取人脸区域
        this->detected = cvRect(lx,ly,rx-lx,ry-ly);
 //       faceROI = frame(faceRect);
 //       emit this->showHist(faceROI);//发送信号，更新人脸区域的直方图

        //获取前额区域坐标
        double fWidth =  (rx-lx) * 0.5;//前额区域宽度为原来人脸区域宽度的一半
        double fHeight = (ry-ly) * 0.15; //高度为原来的0.3
        double fx = lx + (rx - lx - fWidth)/2.0;
        double fy = ly + 0.05 * (ry-ly);   //距上边界0.1倍的原来区域的宽度

        cv::Point flPoint(cvRound(fx),cvRound(fy));
        cv::Point frPoint(cvRound(fx+fWidth),cvRound(fy+fHeight));
        //画出前额区域
        cv::rectangle(frame,flPoint,frPoint,cv::Scalar(255,0,0),2,8,0);

//        //获取前额区域
//        CvRect foreheadRect = cvRect(fx,fy,fWidth,fHeight);
//        foreheadROI = frame(foreheadRect);
    }
}

//判断是否检测到人脸区域
bool FaceDetect::isDetected()
{
    if(this->detected.width>0 && this->detected.height>0)
        return true;
    else
        return false;
}

//返回得到的人脸矩形框
cv::Rect FaceDetect::getWindow()
{
    return this->detected;
}

//调动整个类的运行
void FaceDetect::run(cv::Mat &frame)
{
    this->detectFace(frame);
}
