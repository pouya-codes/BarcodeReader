#-------------------------------------------------
#
# Project created by QtCreator 2017-04-21T11:26:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = olmpiad
TEMPLATE = app
CONFIG += c++11

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opencv
unix: PKGCONFIG += zbar

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui




LIBS +=   -lboost_system \
-lboost_filesystem


