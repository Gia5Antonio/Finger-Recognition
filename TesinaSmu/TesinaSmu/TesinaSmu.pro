#-------------------------------------------------
#
# Project created by QtCreator 2015-07-31T10:14:21
#
#-------------------------------------------------

QT       += core

QT       += multimedia

QT       -= gui

TARGET = TesinaSmu
CONFIG   += console
CONFIG   -= app_bundle

CONFIG += mobility
MOBILITY = multimedia


TEMPLATE = app


SOURCES += main.cpp \
    mano.cpp \
    immagine.cpp \
    roi.cpp




INCLUDEPATH +=C:\opencv\build\include

LIBS +=-L C:\opencv\source-built\lib \
-lopencv_calib3d249 \
-lopencv_contrib249 \
-lopencv_core249 \
-lopencv_features2d249 \
-lopencv_flann249 \
-lopencv_gpu249 \
-lopencv_highgui249 \
-lopencv_imgproc249 \
-lopencv_ml249 \
-lopencv_nonfree249 \
-lopencv_objdetect249 \
-lopencv_video249

HEADERS += \
    mano.h \
    immagine.h \
    roi.h

