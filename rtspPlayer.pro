#-------------------------------------------------
#
# Project created by QtCreator
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia

TARGET = rtspPlayer
TEMPLATE = app


SOURCES += main.cpp \
    videoplayer.cpp \
    #mainwindow1.cpp   \
    mainwindow.cpp

HEADERS  += \
    videoplayer.h \
    #mainwindow1.h  \
    mainwindow.h

FORMS    += \
    mainwindow.ui

INCLUDEPATH+=$$PWD/ffmpeg/include

LIBS += $$PWD/ffmpeg/lib/x64/avcodec.lib \
        $$PWD/ffmpeg/lib/x64/avdevice.lib \
        $$PWD/ffmpeg/lib/x64/avfilter.lib \
        $$PWD/ffmpeg/lib/x64/avformat.lib \
        $$PWD/ffmpeg/lib/x64/avutil.lib \
        $$PWD/ffmpeg/lib/x64/postproc.lib \
        $$PWD/ffmpeg/lib/x64/swresample.lib \
        $$PWD/ffmpeg/lib/x64/swscale.lib
