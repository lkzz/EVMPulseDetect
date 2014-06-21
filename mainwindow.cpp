#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //初始化
    timer = new QTimer(this);
    //人脸检测
    faceDet  = new FaceDetect;
    //人脸跟踪
    faceTrk = new FaceTrack;

    //特征点检测
    featureDet = new FeaturesDetect;

    //颜色放大
    colormagnify = new ColorMagnify;

    //定时器设置
    this->timeout = 40;

    //信号与槽实现连接
    //帧读取
    connect(timer,SIGNAL(timeout()),this,SLOT(mainLoop()));
    //检测人脸
    connect(this,SIGNAL(getFace(cv::Mat&)),faceDet,SLOT(run(cv::Mat&)));
    //跟踪人脸
    connect(this,SIGNAL(trackFace(cv::Mat&,cv::Rect&)),faceTrk,SLOT(run(cv::Mat&,cv::Rect&)));
    //检测特征点
    connect(this,SIGNAL(detectFeatures(cv::Mat&)),featureDet,SLOT(mainloop(cv::Mat&)));
    //颜色放大
    connect(this,SIGNAL(magnify(cv::Mat&)),colormagnify,SLOT(mainloop(cv::Mat&)));
    //放大结果显示
    connect(colormagnify,SIGNAL(showMagnify(cv::Mat)),this,SLOT(showMagnify(cv::Mat)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpenCam_triggered()
{
//    //打开摄像头
    cam.open(0);
//  QString fileName = QFileDialog::getOpenFileName(this,
//                                                  tr("Open Video"),
//                                                  ".",
//                                                  tr("Video Files (*.avi *.mov *.mpeg *.mp4)"));

//    if(!fileName.isEmpty())
//      {
//        cam.open(fileName.toStdString());
//      }

    if(!cam.isOpened())
        QMessageBox::warning(this,tr("Error Information!"),tr("Can't open webcam!"));
    //开始计时，超时则发出信号
    timer->start(timeout);
    colormagnify->setFrameRate(1000/timeout);
}

void MainWindow::mainLoop()
{
    //读入帧
    cam.read(this->frame);
//    cv::resize(frame,frame,cv::Size(ui->labelCam->width(),ui->labelCam->height()));
    cv::cvtColor(frame,frame,CV_BGR2RGB);

//    //先将视频依据 y 轴翻转
    cv::flip(frame,frame,1);

    //我们使用人脸跟踪获得初始的camshift窗口
    if(this->windowInit.width <= 0 || this->windowInit.height <= 0)
    {
        emit this->getFace(frame);
        if(faceDet->isDetected())
        {
            this->windowInit = faceDet->getWindow();
        }
    }else
    {
        emit this->trackFace(frame,this->windowInit);
        //获取跟踪的结果
        this->trackedMat = faceTrk->getTrackBox();

        emit this->detectFeatures(trackedMat);

        cv::imshow("Rotated Window",trackedMat);

        cv::waitKey(10);
//        emit this->magnify(trackedMat);|
//        emit this->magnify(frame);
    }
    //显示帧
    displayFrame(frame);
}

//帧显示函数
void MainWindow::displayFrame(cv::Mat &frame)
{

    // display image in the label
    qimg = QImage((const unsigned char*)(frame.data),
                     frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    ui->labelCam->setPixmap(QPixmap::fromImage(qimg));
    ui->labelCam->repaint();
}

//放大结果显示
void MainWindow::showMagnify(cv::Mat frame)
{
//  cv::cvtColor(frame,frame,CV_BGR2RGB);
  QImage qimg1 = QImage((const unsigned char*)(frame.data),
                        frame.cols, frame.rows, frame.step, QImage::Format_RGB888);


  ui->labelMagnify->setPixmap(QPixmap::fromImage(qimg1));
  ui->labelMagnify->repaint();

}

void MainWindow::on_actionQuit_triggered()
{
  cam.release();
  qApp->exit();
}

void MainWindow::on_actionCloseCam_triggered()
{
//  timer->disconnect();
    cam.release();
}
