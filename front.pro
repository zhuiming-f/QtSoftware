QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    mydialog.cpp \
    tasklist.cpp

HEADERS += \
    mainwindow.h \
    mydialog.h \
    tasklist.h

FORMS += \
    mainwindow.ui \
    mydialog.ui \
    tasklist.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#记得还禁用了C++ Clang Code Model插件
#禁用由于某些函数参数或某些变量未使用导致的警告
QMAKE_CXXFLAGS+=-Wno-unused-parameter
QMAKE_CXXFLAGS+=-Wno-unused-variable
