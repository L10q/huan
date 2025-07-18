QT       += core gui mqtt

QT += network

QT += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    v4l2api.cpp

HEADERS += \
    fsmpBeeper.h \
    fsmpElectric.h \
    fsmpEvents.h \
    fsmpFan.h \
    fsmpLed.h \
    fsmpLight.h \
    fsmpTempHum.h \
    fsmpVibrator.h \
    mainwindow.h \
    v4l2api.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
