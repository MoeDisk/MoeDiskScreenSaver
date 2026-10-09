cmake_minimum_required(VERSION 3.16)
project(MoeDiskScreenSaver VERSION 1.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

set(SOURCES
    src/main.cpp
    src/appshell.cpp
    src/saverdebug.cpp
    src/geometry.cpp
    src/logoloader.cpp
    src/movinglogo.cpp
    src/saverwindow.cpp
    src/configdialog.cpp
    src/launcherwindow.cpp
    src/refreshtestwindow.cpp
    resources/resources.qrc
)

if(WIN32)
    list(APPEND SOURCES src/platformargs_win.cpp resources/version.rc)
else()
    list(APPEND SOURCES src/platformargs_unix.cpp)
endif()

if(APPLE)
    list(APPEND SOURCES resources/dvd-princess.icns)
    set_source_files_properties(resources/dvd-princess.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
endif()

if(WIN32)
    add_executable(MoeDiskScreenSaver WIN32 ${SOURCES})
else()
    add_executable(MoeDiskScreenSaver ${SOURCES})
endif()

target_link_libraries(MoeDiskScreenSaver PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets)

if(APPLE)
    set_target_properties(MoeDiskScreenSaver PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "MoeDiskScreenSaver"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.moedisk.screensaver"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (C) MoeDisk"
        MACOSX_BUNDLE_ICON_FILE "dvd-princess.icns"
    )
endif()

install(TARGETS MoeDiskScreenSaver
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

install(FILES packaging/linux/moedisk-screensaver.desktop
    DESTINATION share/applications
)

if(UNIX AND NOT APPLE)
    install(FILES resources/dvd-princess-icon.png
        DESTINATION share/icons/hicolor/512x512/apps
        RENAME moedisk-screensaver.png
    )
endif()
