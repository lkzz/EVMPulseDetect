#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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


}
