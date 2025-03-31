QT       += core gui charts positioning

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dataanalyzer.cpp \
    database.cpp \
    httpclient.cpp \
    jsonhandler.cpp \
    main.cpp \
    mainwindow.cpp \
    stationmanager.cpp

HEADERS += \
    dataanalyzer.h \
    database.h \
    httpclient.h \
    jsonhandler.h \
    mainwindow.h \
    stationmanager.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += -L$$PWD/../../../vcpkg/installed/x64-windows/lib -lcurl

INCLUDEPATH += C:/Users/bartl/vcpkg/installed/x64-windows/include
DEFINES += JSON_USE_IMPLICIT_CONVERSIONS=1
DEFINES += WIN32_LEAN_AND_MEAN
LIBS += -lole32 -loleaut32

