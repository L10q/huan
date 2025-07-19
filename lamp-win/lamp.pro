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
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# 添加Vosk语音识别库支持
# 添加头文件路径
INCLUDEPATH += $$PWD/include

# 添加库文件路径（根据个人系统调整）
LIBS += -L$$PWD/lib -lvosk
LIBS += -L"D:\Downloads\APP\OpenBLAS-0.3.29_x64\lib" -lopenblas

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc
