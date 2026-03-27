QT       += core gui core-private

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    customhid.cpp \
    main.cpp \
    mainwindow.cpp \
    onewire.cpp

HEADERS += \
    customhid.h \
    hid/inc/hidapi.h \
    mainwindow.h \
    onewire.h \
    usertypes.h

linux:!android {
    SOURCES += hid/src/linux/hid.c
    DESTDIR = ../../Binary/linux
}

macx {
    SOURCES += hid/src/mac/hid.c
    DESTDIR = ../../Binary/mac
}

win32 {
    SOURCES += hid/src/windows/hid.c
    DESTDIR = ../../Binary/windows
}

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DESTDIR = ../../Binary

RESOURCES += res.qrc
