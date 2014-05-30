#include "facetrack.h"

FaceTrack::FaceTrack(QObject *parent) :
    QObject(parent),
    stateNum(4),
    measureNum(2),
    smin(30),
    vmin(10),
    vmax(255),
    histSize(30),
    showHist(false),
    globalwidth(250),
    globalheight(300)
{
    //卡尔曼滤波器初始化
    this->klFilter = new cv::KalmanFilter(stateNum,measureNum,0);
    this->klFilter->transitionMatrix= *(cv::Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
    cv::setIdentity(klFilter->measurementMatrix);
    cv::setIdentity(klFilter->processNoiseCov,cv::Scalar::all(1e-5));
    cv::setIdentity(klFilter->measurementNoiseCov,cv::Scalar::all(1e-1));
    //预测估计协方差矩阵;
    cv::setIdentity(klFilter->errorCovPost,cv::Scalar::all(1.0));

    this->measurement = cv::Mat_<float>(2,1);
    this->measurement.setTo(cv::Scalar::all(0));

}

//camshift处理过程实现
cv::RotatedRect FaceTrack::camshiftProcess(cv::Mat &frame,cv::Rect &ROI)
{
//    //保存上次的结果
//    this->lastROI = ROI;

    cv::Size fsize = frame.size();
    hsv  = cv::Mat(fsize,CV_8UC3);
    mask = cv::Mat(fsize,CV_8UC1);
    //颜色空间转换
    cv::cvtColor(frame,hsv,CV_RGB2HSV);

    /*得到二值的 MASK
    inRange函数的功能是检查输入数组每个元素大小是否在2个给定数值之间，可以有多通道,mask保存0通道的最小值，也就是h分量
    这里利用了hsv的3个通道，比较h,0~180,s,smin~256,v,min(vmin,vmax),max(vmin,vmax)。如果3个通道都在对应的范围内，则
    mask对应的那个点的值全为1(0xff)，否则为0(0x00)
    */
    cv::inRange(hsv,cv::Scalar(0,smin,MIN(vmin,vmax),0),
                cv::Scalar(180,256,MAX(vmin,vmax)),mask);

    //只提取 H 分量
    int ch[] = {0,0};
    hue.create(hsv.size(),hsv.depth());
    //将hsv第一个通道(也就是色调)的数复制到hue中，0索引数组
    cv::mixChannels(&hsv,1,&hue,1,ch,1);

    //用于camshift计算直方图
    float range[] = {0,180};
    const float* histRange = {range};

    cv::Mat roi(hue,ROI);
    cv::Mat maskroi(mask,ROI);
    //计算直方图
    cv::calcHist(&roi,1,0,maskroi,hist,1,&histSize,&histRange);
    //进行归一化
    cv::normalize(hist, hist, 0, 255, CV_MINMAX);

    if(this->showHist)
    {//画出直方图
        histImg = cv::Mat::zeros(frame.size(),CV_8UC3);
        histImg = cv::Scalar::all(0);
        int binW = histImg.cols / histSize;
        cv::Mat buf(1,histSize,CV_8UC3);
        //saturate_case函数为从一个初始类型准确变换到另一个初始类型
        for( int i = 0; i < histSize; i++ )
            buf.at<cv::Vec3b>(i) = cv::Vec3b(cv::saturate_cast<uchar>(i*180./histSize), 255, 255);
        cvtColor(buf, buf, CV_HSV2RGB);

        for( int i = 0; i < histSize; i++ )
        {
            int val = cv::saturate_cast<int>(hist.at<float>(i)*histImg.rows/255);
            cv::rectangle( histImg, cv::Point(i*binW,histImg.rows),
                           cv::Point((i+1)*binW,histImg.rows - val),
                           cv::Scalar(buf.at<cv::Vec3b>(i)), -1, 8 );
        }
    }
    //计算直方图的反向投影，计算hue图像0通道直方图hist的反向投影
    cv::calcBackProject(&hue,1,0,hist,backProject,&histRange);
    backProject &= mask;

    //调用camshift模块
    cv::RotatedRect trackBox = cv::CamShift(backProject,ROI,
                                   cv::TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER,100,1));

 //   this->plotRect(frame,result);
    return trackBox;
}

//依据给出区域的中心坐标，进行卡尔曼滤波，只预测中心
void FaceTrack::kalmanProcess(cv::RotatedRect &rotRect)
{
    this->measurement.at<float>(0) = rotRect.center.x;
    this->measurement.at<float>(1) = rotRect.center.y;
    cv::Mat estimated;
    //进行 kalman 预测，可以得到预测坐标
    estimated = klFilter->correct(this->measurement);
    cv::Point2f statePt(estimated.at<float>(0),estimated.at<float>(1));
    estimatedCenter = statePt;
}

//画出跟踪到的区域
void FaceTrack::plotRect(cv::Mat &frame, cv::RotatedRect &trackBox)
{
    cv::Point2f vertices[4];
    trackBox.points(vertices);
    for(int i=0;i<4;i++)
    {
        cv::line(frame,vertices[i],vertices[(i+1)%4],cv::Scalar(0,255,0),2);
    }

    cv::Rect brect = trackBox.boundingRect();
    cv::rectangle(frame,brect,cv::Scalar(255,0,0),2);
}

cv::Mat FaceTrack::rotatedProcess(cv::Mat &frame, cv::RotatedRect &rotRect)
{
    float angle = rotRect.angle;
    if( angle>135.0 )
        angle = 180 - angle;

    std::cout<<angle<<std::endl;
    cv::Mat warp,rotAfter;
    warp = cv::getRotationMatrix2D(rotRect.center,(double)angle,1.0);
    cv::warpAffine(frame,rotAfter,warp,frame.size());
    cv::getRectSubPix(rotAfter,rotRect.size,rotRect.center,rotAfter);
    cv::cvtColor(rotAfter,rotAfter,CV_BGR2RGB);
    std::cout<<rotAfter.cols<<","<<rotAfter.rows<<std::endl;
    cv::namedWindow("Rotated Window");
    cv::imshow("Rotated Window",rotAfter);

//    //use minAreaRect
//    cv::Point2f vertices[4];
//    rotRect.points(vertices);
//    std::vector<cv::Point2f> vertVec;
//    cv::RotatedRect calculateRect;
//    for(int i=0;i<4;i++)
//        vertVec.push_back(vertices[i]);
//    calculateRect = cv::minAreaRect(vertVec);
//    cv::Mat bg;
//    frame.copyTo(bg);
//    cv::cvtColor(bg,bg,CV_BGR2RGB);
//    for(int i=0;i<4;i++)
//    {
//        cv::line(bg,vertVec[i],vertVec[(i+1)%4],cv::Scalar(0,0,255));
//    }
//    cv::namedWindow("minAreaRect");
//    cv::imshow("minAreaRect",bg);

    cv::waitKey(10);
//    if( c == 27 )
//        break;

    return rotAfter;
}

void FaceTrack::setVmin(int min)
{
    this->vmin = min;
}

void FaceTrack::setVmax(int max)
{
    this->vmax = max;
}

//期望的宽度设置
void FaceTrack::setWidth(int width)
{
    this->globalwidth = width;
}
//期望的长度设置
void FaceTrack::setHeight(int height)
{
    this->globalheight = height;
}


//输出结果尺度调整
cv::Mat FaceTrack::uniformSize(cv::Mat &input)
{
    cv::Mat output;
    if(input.cols > this->globalwidth && input.rows > this->globalheight)
    {
        double x = (input.cols - globalwidth)/2.0;
        double y = (input.rows - globalheight)/2.0;
        cv::Rect rect(x,y,globalwidth,globalheight);
        output = input(rect);
    }else
    {
        cv::resize(input,output,cv::Size(globalwidth,globalheight),0,0,CV_INTER_CUBIC);
    }

    return output;

}

//通过信号和槽机制完成主要的函数调用
void FaceTrack::run(cv::Mat &frameIn, cv::Rect &faceRect)
{
    this->trackBox = this->camshiftProcess(frameIn,faceRect);
 //   this->kalmanProcess(rotRect);
    cv::Mat rotMat = this->rotatedProcess(frameIn,trackBox);
    cv::Mat uniMat = this->uniformSize(rotMat);
    cv::namedWindow("Uniform Window");
    cv::imshow("Uniform Window",uniMat);

    cv::waitKey(10);
    //画出rotatedRect
    this->plotRect(frameIn,trackBox);
}


