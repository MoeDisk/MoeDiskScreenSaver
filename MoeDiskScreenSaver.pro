QT += core gui widgets
CONFIG += c++11 warn_on
TEMPLATE = app
TARGET = MoeDiskScreenSaver

win32 {
    CONFIG += windows
    CONFIG -= console
    RC_FILE = resources/version.rc
    VERSION = 1.0.1.0
    QMAKE_TARGET_COMPANY = MoeDisk
    QMAKE_TARGET_PRODUCT = "MoeDiskScreenSaver"
    QMAKE_TARGET_DESCRIPTION = "MoeDiskScreenSaver"
    QMAKE_TARGET_COPYRIGHT = "Copyright (C) MoeDisk"
}

macx: ICON = resources/dvd-princess.icns

SOURCES += \
    src/main.cpp \
    src/appshell.cpp \
    src/saverdebug.cpp \
    src/geometry.cpp \
    src/logoloader.cpp \
    src/movinglogo.cpp \
    src/saverwindow.cpp \
    src/configdialog.cpp \
    src/launcherwindow.cpp \
    src/refreshtestwindow.cpp

win32: SOURCES += src/platformargs_win.cpp
unix:!win32: SOURCES += src/platformargs_unix.cpp

HEADERS += \
    src/platformargs.h \
    src/saverdebug.h \
    src/geometry.h \
    src/logoloader.h \
    src/movinglogo.h \
    src/saverwindow.h \
    src/configdialog.h \
    src/launcherwindow.h \
    src/refreshtestwindow.h \
    src/uilanguage.h

RESOURCES += resources/resources.qrc

DISTFILES += \
    resources/DVDVideo360.png \
    resources/dvd-princess.icns \
    resources/dvd-princess.ico \
    resources/dvd-princess-icon.png
