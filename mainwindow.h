#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //显示帧
    void displayFrame(cv::Mat &frame);

private slots:
    //主要的功能调用槽
    void mainLoop();
    void on_actionOpenCam_triggered();

private:
    Ui::MainWindow *ui;
    //设置一个定时器，用于触发帧读取
    QTimer *timer;
    cv::VideoCapture cam;
    //用于保存摄像头捕获的当前帧
    cv::Mat frame;
    //用于显示帧
    QImage qimg;
};

#endif // MAINWINDOW_H
