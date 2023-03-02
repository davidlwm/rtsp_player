#include "videoplayer.h"
#include <stdio.h>
#include<iostream>

using namespace std;

void VideoPlayer::startPlay(QString url)
{
    //调用start函数将会自动执行run函数
    m_strFileName = url;
    this->start();
}

void VideoPlayer::run()
{
    AVFormatContext *pFormatCtx; //音视频封装格式上下文结构体
    AVCodecContext  *pCodecCtx;  //音视频编码器上下文结构体
    AVCodec *pCodec; //音视频编码器结构体
    AVFrame *pFrame; //存储一帧解码后像素数据
    AVFrame *pFrameRGB;
    AVPacket *pPacket; //存储一帧压缩编码数据
    uint8_t *pOutBuffer;
    static struct SwsContext *pImgConvertCtx;

    avformat_network_init();   //初始化FFmpeg网络模块
    av_register_all();         //初始化FFMPEG  调用了这个才能正常适用编码器和解码器

    //Allocate an AVFormatContext.
    pFormatCtx = avformat_alloc_context();

    //AVDictionary
    AVDictionary *avdic=nullptr;
    char option_key[]="rtsp_transport";
    char option_value[]="udp";
    av_dict_set(&avdic,option_key,option_value,0);
    char option_key2[]="max_delay";
    char option_value2[]="100";
    av_dict_set(&avdic,option_key2,option_value2,0);

    if (avformat_open_input(&pFormatCtx, m_strFileName.toLocal8Bit().data(), nullptr, &avdic) != 0)
    {
        printf("can't open the file. \n");
        return;
    }

    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
    {
        printf("Could't find stream infomation.\n");
        return;
    }

    //查找视频中包含的流信息，音频流先不处理
    int videoStreamIdx = -1;
    qDebug("apFormatCtx->nb_streams:%d", pFormatCtx->nb_streams);
    for (int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIdx = i; //视频流
        }
    }
    if (videoStreamIdx == -1)
    {
        printf("Didn't find a video stream.\n"); //没有找到视频流
        return;
    }

    //查找解码器
    qDebug("avcodec_find_decoder...");
    pCodecCtx = pFormatCtx->streams[videoStreamIdx]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == nullptr)
    {
        printf("Codec not found.\n");
        return;
    }
    pCodecCtx->bit_rate =0;   //初始化为0
    pCodecCtx->time_base.num=1;  //下面两行：一秒钟25帧
    pCodecCtx->time_base.den=10;
    pCodecCtx->frame_number=1;  //每包一个视频帧

    //打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0)
    {
        printf("Could not open codec.\n");
        return;
    }

    //将解码后的YUV数据转换成RGB32
    pImgConvertCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                     pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                     AV_PIX_FMT_RGB32, SWS_BICUBIC, nullptr, nullptr, nullptr);

    int numBytes = avpicture_get_size(AV_PIX_FMT_RGB32, pCodecCtx->width,pCodecCtx->height);

    pFrame     = av_frame_alloc();
    pFrameRGB  = av_frame_alloc();
    pOutBuffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, pOutBuffer, AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height);

    pPacket = (AVPacket *) malloc(sizeof(AVPacket)); //分配一个packet
    int y_size = pCodecCtx->width * pCodecCtx->height;
    av_new_packet(pPacket, y_size); //分配packet的数据

    while (1)
    {
        if (av_read_frame(pFormatCtx, pPacket) < 0)
        {
            break; //这里认为视频读取完了
        }

        if (pPacket->stream_index == videoStreamIdx)
        {
            int got_picture;
            int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,pPacket);
            if (ret < 0)
            {
                printf("decode error.\n");
                return;
            }

            if (got_picture)
            {
                sws_scale(pImgConvertCtx, (uint8_t const * const *) pFrame->data, pFrame->linesize,
                          0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

                //把这个RGB数据 用QImage加载
                QImage tmpImg((uchar *)pOutBuffer, pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB32);
                QImage image = tmpImg.copy(); //把图像复制一份 传递给界面显示
                emit sig_GetOneFrame(image);  //发送信号
            }
        }
        av_free_packet(pPacket);
        //msleep(0.02);
    }

    av_free(pOutBuffer);
    av_free(pFrameRGB);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
}
