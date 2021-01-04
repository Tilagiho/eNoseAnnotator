TEMPLATE = app
QT       += core gui serialport svg opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++14
QMAKE_CXXFLAGS += -D_GLIBCXX_USE_CXX11_ABI=0 -DDLIB_NO_GUI_SUPPORT

# QMAKE_LFLAGS += -no-pie

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# include git version macro
include(gitversion.pri)

# for travis-ci.com:
# create appdir structure
linux-g++ {
    isEmpty(PREFIX) {
        PREFIX = /usr
    }
    target = $$PREFIX/bin
    INSTALLS += target
}

# configurations for QCrashHandler (google breakpad wrapper):
include($$PWD/lib/QCrashHandler/src/qcrashhandler.pri)

CONFIG(debug, debug|release) {
    TARGET = appd
} else {
    TARGET = app
    # create debug symbols for release builds
    CONFIG *= force_debug_info
    QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO -= -O2
}


SOURCES += \
    classes/aclass.cpp \
    classes/annotation.cpp \
    classes/controler.cpp \
    classes/datasource.cpp \
    classes/enosecolor.cpp \
    classes/fakedatasource.cpp \
    classes/functionalisation.cpp \
    classes/leastsquaresfitter.cpp \
    classes/measurementdata.cpp \
    classes/mvector.cpp \
    classes/torchclassifier.cpp \
    classes/usbdatasource.cpp \
    classes/curvefitworker.cpp \
    lib/comboboxitemdelegate.cpp \
    lib/dlib/dlib/all/source.cpp \
    lib/spoiler.cpp \
    main.cpp \
    widgets/acirclewidget.cpp \
    widgets/addattributedialog.cpp \
    widgets/attributeeditor.cpp \
    widgets/bargraphwidget.cpp \
    widgets/classifierwidget.cpp \
    widgets/classinputdialog.cpp \
    widgets/classselector.cpp \
    widgets/convertwizard.cpp \
    widgets/curvefitwizard.cpp \
    widgets/functionalisationdialog.cpp \
    widgets/generalsettings.cpp \
    widgets/infowidget.cpp \
    widgets/linegraphwidget.cpp \
    widgets/mainwindow.cpp \
    widgets/setsensorfailuresdialog.cpp \
    widgets/sourcedialog.cpp \
    widgets/usbsettingswidget.cpp \

HEADERS += \
    classes/aclass.h \
    classes/annotation.h \
    classes/classifier_definitions.h \
    classes/controler.h \
    classes/datasource.h \
    classes/defaultSettings.h \
    classes/enosecolor.h \
    classes/fakedatasource.h \
    classes/functionalisation.h \
    classes/leastsquaresfitter.h \
    classes/measurementdata.h \
    classes/mvector.h \
    classes/torchclassifier.h \
    classes/usbdatasource.h \
    classes/curvefitworker.h \
    lib/comboboxitemdelegate.h \
    lib/spoiler.h \
    widgets/acirclewidget.h \
    widgets/addattributedialog.h \
    widgets/attributeeditor.h \
    widgets/bargraphwidget.h \
    widgets/classifierwidget.h \
    widgets/classinputdialog.h \
    widgets/classselector.h \
    widgets/convertwizard.h \
    widgets/curvefitwizard.h \
    widgets/fixedplotmagnifier.h \
    widgets/fixedplotzoomer.h \
    widgets/functionalisationdialog.h \
    widgets/generalsettings.h \
    widgets/infowidget.h \
    widgets/linegraphwidget.h \
    widgets/mainwindow.h \
    widgets/setsensorfailuresdialog.h \
    widgets/sourcedialog.h \
    widgets/usbsettingswidget.h \

FORMS += \
    widgets/acirclewidget.ui \
    widgets/addattributedialog.ui \
    widgets/attributeeditor.ui \
    widgets/classifierwidget.ui \
    widgets/classselector.ui \
    widgets/generalsettings.ui \
    widgets/infowidget.ui \
    widgets/mainwindow.ui \
    widgets/sourcedialog.ui \
    widgets/usbsettingswidget.ui

# Default rules for deployment.
# qnx: target.path = /tmp/$${TARGET}/bin
# else: unix:!android: target.path = /opt/$${TARGET}/bin
# !isEmpty(target.path): INSTALLS += target

DISTFILES += \
    documentation/source/overview.qdoc \
    icon.png \
    project.qdocconf \
    readme

RESOURCES += \
    eNoseAnnotator.qrc

# libtorch
win32: LIBS += -L$$PWD/lib/libtorch/lib -ltorch -lc10 -ltorch_cpu
unix:!macx: LIBS += -L$$PWD/lib/libtorch/lib -ltorch -lc10 -ltorch_cpu
unix:!macx: QMAKE_RPATHDIR += $$PWD/lib/libtorch/lib

INCLUDEPATH += $$PWD/lib/libtorch/include
DEPENDPATH += $$PWD/lib/libtorch/include
INCLUDEPATH += $$PWD/lib/libtorch/include/torch/csrc/api/include
DEPENDPATH += $$PWD/lib/libtorch/include/torch/csrc/api/include

# dlib
LIBS += -L$$PWD/lib/dlib
INCLUDEPATH += $$PWD/lib/dlib
DEPENDPATH += $$PWD/lib/dlib

#qwt
unix: QWT_ROOT = /usr/local/qwt-6.1.5
win32: QWT_ROOT = C:/qwt-6.1.5

include ( $$QWT_ROOT/features/qwt.prf )
LIBS += -L$$QWT_ROOT/lib -lqwt
INCLUDEPATH += $$QWT_ROOT/include
DEPENDPATH += $$QWT_ROOT/include

#INCLUDEPATH += $$PWD/lib/qwt-6.1.5/src
#DEPENDPATH += $$PWD/lib/qwt-6.1.5/src
#LIBS += -L$$PWD/lib/qwt-6.1.5/lib -lqwt
#win32: DEFINES += QWT_DLL QWT_NO_OPENGL
#unix {
# include ( $$(HOME)/qwt-6.1.5/features/qwt.prf )
# INCLUDEPATH += $$PWD/lib/qwt-6.1.5/src
# DEPENDPATH += $$PWD/lib/qwt-6.1.5/src
# LIBS += -L$$PWD/lib/qwt-6.1.5/lib -lqwt
#}
#win32: include ( C:/qwt-6.1.5/features/qwt.prf )
