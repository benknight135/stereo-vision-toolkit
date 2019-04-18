#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T16:38:42
#
# Copyright I3D Robotics Ltd, 2017
# Authors: Josh Veitch-Michaelis, Ben Knight
#
#-------------------------------------------------

QT += core gui concurrent widgets xml

TARGET = stereo_vision_toolkit
TEMPLATE = app vcapp

#CONFIG += console
CONFIG += warn_on
CONFIG += doc

RC_FILE = icon.rc

RESOURCES += $$_PRO_FILE_PWD_/resources/qdarkstyle/style.qrc

include($$_PRO_FILE_PWD_/resources/QtAwesome/QtAwesome.pri)

VPATH = $$_PRO_FILE_PWD_/src
INCLUDEPATH += $$_PRO_FILE_PWD_/src

SOURCES += main.cpp\
        mainwindow.cpp \
    calibrationdialog.cpp \
    stereocalibrate.cpp \
    chessboard.cpp \
    calibrateconfirmdialog.cpp \
    calibratefromimagesdialog.cpp \
    abstractstereocamera.cpp \
    abstractstereomatcher.cpp \
    cameraopencv.cpp \
    stereocameraopencv.cpp \
    stereocameradeimos.cpp \
    stereocamerafromvideo.cpp \
    matcheropencvblock.cpp \
    matcherwidgetopencvblock.cpp \
    matcheropencvsgbm.cpp \
    matcherwidget.cpp \
    matcherwidgetopencvsgbm.cpp \
    disparityviewer.cpp \
    paramfile.cpp \
    cameradisplaywidget.cpp \
    stereocameraphobos.cpp \
    cameraimagingsource.cpp \
    matcherjrsgm.cpp \
    matcherwidgetjrsgm.cpp


HEADERS  += mainwindow.h \
    calibrationdialog.h \
    stereocalibrate.h \
    chessboard.h \
    calibrateconfirmdialog.h \
    calibratefromimagesdialog.h \
    abstractstereocamera.h \
    abstractstereomatcher.h \
    cameraopencv.h \
    stereocameraopencv.h \
    stereocameradeimos.h \
    stereocamerafromvideo.h \
    matcheropencvblock.h \
    matcherwidgetopencvblock.h \
    matcheropencvsgbm.h \
    matcherwidget.h \
    matcherwidgetopencvsgbm.h \
    disparityviewer.h \
    paramfile.h \
    cameradisplaywidget.h \
    stereocameraphobos.h \
    cameraimagingsource.h \
    matcherjrsgm.h \
    matcherwidgetjrsgm.h


FORMS    += mainwindow.ui \
    calibrationdialog.ui \
    calibrateconfirmdialog.ui \
    calibratefromimagesdialog.ui \
    matcherwidgetopencvblock.ui \
    matcherwidgetopencvsgbm.ui \
    disparityviewer.ui \
    cameradisplaywidget.ui \
    matcherwidgetjrsgm.ui

# For building in a single folder
CONFIG(debug, debug|release) {
    DESTDIR = debug
    OBJECTS_DIR = .obj_debug
    MOC_DIR     = .moc_debug
}else {
    DESTDIR = release
    OBJECTS_DIR = .obj
    MOC_DIR     = .moc
}

INCLUDEPATH += "$$_PRO_FILE_PWD_/3rd_party/opencv/include"
INCLUDEPATH += "$$_PRO_FILE_PWD_/3rd_party/VTK/include/vtk-7.0"
INCLUDEPATH += "$$_PRO_FILE_PWD_/3rd_party/PCL/include/pcl-1.8"
INCLUDEPATH += "$$_PRO_FILE_PWD_/3rd_party/hidapi/include"
INCLUDEPATH += "$$_PRO_FILE_PWD_/3rd_party/tis/include"
INCLUDEPATH += "$$_PRO_FILE_PWD_/3rd_party/yaml-cpp/include"
INCLUDEPATH += "$$_PRO_FILE_PWD_/3rd_party/jr/include"

LIBS += -L"$$_PRO_FILE_PWD_/3rd_party/jr/lib/PhobosIntegration" -lPhobosIntegration

CONFIG(debug, debug|release) {
    message("Debug mode")
    LIBS += -L"$$_PRO_FILE_PWD_/3rd_party/pcl/lib"
    LIBS += -L"$$_PRO_FILE_PWD_/3rd_party/vtk/lib"
    LIBS += -L"$$_PRO_FILE_PWD_/3rd_party/opencv/lib"
    LIBS += -L"$$_PRO_FILE_PWD_/3rd_party/hidapi/lib/debug" -lhidapi
    LIBS += -L"$$_PRO_FILE_PWD_/3rd_party/tis/lib/debug" -lTIS_UDSHL11d_x64
    LIBS += -lpcl_visualization_debug -lpcl_io_debug -lpcl_common_debug -lpcl_filters_debug
    LIBS += -lopencv_ximgproc341d -lopencv_core341d -lopencv_highgui341d -lopencv_calib3d341d -lopencv_videoio341d -lopencv_imgproc341d -lopencv_imgcodecs341d
    @#ifdef CUDA
    LIBS += -lopencv_cudastereo341d -lopencv_cudawarping341d
    @#endif
}else {
    message("Release mode")
    LIBS += -L"$$_PRO_FILE_PWD_/3rd_party/pcl/lib"
    LIBS += -L"$$_PRO_FILE_PWD_/3rd_party/vtk/lib"
    LIBS += -L"$$_PRO_FILE_PWD_/3rd_party/opencv/lib"
    LIBS += -L"$$_PRO_FILE_PWD_/3rd_party/hidapi/lib/release" -lhidapi
    LIBS += -L"$$_PRO_FILE_PWD_/3rd_party/tis/lib/release" -lTIS_UDSHL11_x64
    LIBS += -lpcl_visualization_release -lpcl_io_release -lpcl_common_release -lpcl_filters_release
    LIBS += -lopencv_ximgproc341 -lopencv_core341 -lopencv_highgui341 -lopencv_calib3d341 -lopencv_videoio341 -lopencv_imgproc341 -lopencv_imgcodecs341
    @#ifdef CUDA
    LIBS += -lopencv_cudastereo341 -lopencv_cudawarping341
    @#endif
}

LIBS += -lvtkCommonCore-7.0 -lvtkCommonDataModel-7.0 -lvtkGUISupportQt-7.0 -lvtkViewsQt-7.0 -lvtkViewsCore-7.0 -lvtkRenderingQt-7.0  -lvtkCommonMath-7.0 -lvtkRenderingCore-7.0 -lvtkIOCore-7.0

# Required for PCL
LIBS += -L"$$_PRO_FILE_PWD_/3rd_party/boost/lib"
INCLUDEPATH += "$$_PRO_FILE_PWD_/3rd_party/eigen"
INCLUDEPATH += "$$_PRO_FILE_PWD_/3rd_party/boost/include"

# Directshow class IDs
LIBS += -lstrmiids

DISTFILES += $$_PRO_FILE_PWD_/resources/fonts/fontawesome-webfont.ttf

isEmpty(TARGET_EXT) {
    win32 {
        TARGET_CUSTOM_EXT = .exe
    }
    macx {
        TARGET_CUSTOM_EXT = .app
    }
} else {
    TARGET_CUSTOM_EXT = $${TARGET_EXT}
}

win32 {
    DEPLOY_COMMAND = windeployqt
}
macx {
    DEPLOY_COMMAND = macdeployqt
}

CONFIG( debug, debug|release ) {
    # debug
    DEPLOY_TARGET = $$shell_quote($$shell_path($${_PRO_FILE_PWD_}/../build/debug/$${TARGET}$${TARGET_CUSTOM_EXT}))
} else {
    # release
    DEPLOY_TARGET = $$shell_quote($$shell_path($${_PRO_FILE_PWD_}/../build/release/$${TARGET}$${TARGET_CUSTOM_EXT}))
}

#  # Uncomment the following line to help debug the deploy command when running qmake
#  warning($${DEPLOY_COMMAND} $${DEPLOY_TARGET})

# Use += instead of = if you use multiple QMAKE_POST_LINKs
QMAKE_POST_LINK += $${DEPLOY_COMMAND} $${DEPLOY_TARGET}

CONFIG( doc ){
    QMAKE_POST_LINK += && cd $${_PRO_FILE_PWD_} && doxygen
}
