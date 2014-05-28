#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //初始化
    timer = new QTimer(this);
    faceDet  = new FaceDetect;

    //信号与槽实现连接
    connect(timer,SIGNAL(timeout()),this,SLOT(mainLoop()));
    connect(this,SIGNAL(getFace(cv::Mat&)),faceDet,SLOT(run(cv::Mat&)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpenCam_triggered()
{
    //打开摄像头
    cam.open(0);
    if(!cam.isOpened())
        QMessageBox::warning(this,tr("Error Information!"),tr("Can't open webcam!"));

    //开始计时，超时则发出信号(每秒25帧)
    timer->start(40);
}

void MainWindow::mainLoop()
{
    //读入帧
    cam.read(this->frame);
    cv::cvtColor(frame,frame,CV_BGR2RGB);

    //我们使用人脸跟踪获得初始的camshift窗口
    if(this->windowInit.width <= 0 || this->windowInit.height <= 0)
    {
        emit this->getFace(frame);
        if(faceDet->isDetected())
        {
            this->windowInit = faceDet->getWindow();
        }
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

void MainWindow::on_actionQuit_triggered()
{
    qApp->exit();
}

void MainWindow::on_actionCloseCam_triggered()
{
    cam.release();
}
