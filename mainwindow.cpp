#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QThread>
#include <QPainter>
#include <QInputDialog>
#include <QtMath>
#include <QTime>
#include<iostream>
#pragma execution_character_set("utf-8")

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
   ui(new Ui::MainWindow)
{ 
    ui->setupUi(this);

    qDebug("ffmpeg versioin is: %u", avcodec_version());

    m_pPlayer = new VideoPlayer;
    connect(m_pPlayer,SIGNAL(sig_GetOneFrame(QImage)),this,SLOT(slotGetOneFrame(QImage)));

    m_strUrl = "rtsp://admin:haiber0702@192.168.1.52:554/12";
    ui->lineEditUrl->setText(m_strUrl);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotGetOneFrame(QImage img)
{
    ui->labelCenter->clear();
    if(m_kPlayState == RPS_PAUSE)
    {
       return;
    }

    m_Image = img;
    update(); //调用update将执行paintEvent函数
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    int showWidth = this->width()/2;
    int showHeight = this->height()/2;

    painter.setBrush(Qt::white);
    painter.drawRect(0, 0, this->width(), this->height()); //先画成白色

    if (m_Image.size().width() <= 0)
    {
        return;
    }

    //将图像按比例缩放
    QImage img = m_Image.scaled(QSize(showWidth, showHeight),Qt::KeepAspectRatio);
    img = img.mirrored(m_bHFlip, m_bVFlip);

//    int x = this->width() - img.width();
//    int y = this->height() - img.height();
//    x /= 2;
//    y /= 2;
    int move = 35;
    int x = img.width();
    int y = img.height();

    painter.drawImage(QPoint(0,0+move),img); //画出图像
    painter.drawImage(QPoint(0,y+move),img); //画出图像
    painter.drawImage(QPoint(x,0+move),img); //画出图像
    painter.drawImage(QPoint(x,y+move),img); //画出图像
}

void MainWindow::on_pushButton_toggled(bool checked)
{
    if (checked) //第一次按下为启动，后续则为继续
    {
        if(m_kPlayState == RPS_IDLE)
        {
            ui->lineEditUrl->setEnabled(false);
            m_strUrl = ui->lineEditUrl->text();
            m_pPlayer->startPlay(m_strUrl);

            ui->labelCenter->setText("rtsp网络连接中...");
        }
        m_kPlayState = RPS_RUNNING;
        ui->pushButton->setText("暂停");
    }
    else
    {
        m_kPlayState = RPS_PAUSE;
        ui->pushButton->setText("播放");
        ui->lineEditUrl->setEnabled(true);
    }
}

void MainWindow::on_checkBoxVFlip_clicked(bool checked)
{
    m_bVFlip = checked;
}

void MainWindow::on_checkBoxHFlip_clicked(bool checked)
{
    m_bHFlip = checked;
}
