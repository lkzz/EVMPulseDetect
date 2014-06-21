#-------------------------------------------------
#
# Project created by QtCreator 2014-05-27T19:28:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EVMPulseDet
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    facedetect.cpp \
    facetrack.cpp \
    SpatialFilter.cpp \
    colormagnify.cpp \
    featuresdetect.cpp

HEADERS  += mainwindow.h \
    facedetect.h \
    facetrack.h \
    SpatialFilter.h \
    colormagnify.h \
    featuresdetect.h

FORMS    += mainwindow.ui

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv

}

Win32 {
INCLUDEPATH += C:\OpenCV2.2\include\
LIBS += -LC:\OpenCV2.2\lib \
    -lopencv_core220 \
    -lopencv_highgui220 \
    -lopencv_imgproc220 \
    -lopencv_features2d220 \
    -lopencv_calib3d220
}

RESOURCES += \
    icons.qrc
