QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

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

# for travis-ci.com:
# create appdir structure
linux-g++ {
    isEmpty(PREFIX) {
        PREFIX = /usr
    }
    target = $$PREFIX/bin
    INSTALLS += target
}


SOURCES += \
    classes/aclass.cpp \
    classes/datasource.cpp \
    classes/measurementdata.cpp \
    classes/mvector.cpp \
    classes/sensorcolor.cpp \
    classes/usbdatasource.cpp \
    main.cpp \
    qcustomplot/qcustomplot.cpp \
    widgets/addattributedialog.cpp \
    widgets/attributeeditor.cpp \
    widgets/bargraphwidget.cpp \
    widgets/classifierwidget.cpp \
    widgets/classinputdialog.cpp \
    widgets/classselector.cpp \
    widgets/functionalisationdialog.cpp \
    widgets/generalsettings.cpp \
    widgets/infowidget.cpp \
    widgets/linegraphwidget.cpp \
    widgets/mainwindow.cpp \
    widgets/setsensorfailuresdialog.cpp \
    widgets/sourcedialog.cpp \
    widgets/usbsettingswidget.cpp


HEADERS += \
    classes/aclass.h \
    classes/datasource.h \
    classes/measurementdata.h \
    classes/mvector.h \
    classes/sensorcolor.h \
    classes/usbdatasource.h \
    qcustomplot/qcustomplot.h \
    widgets/addattributedialog.h \
    widgets/attributeeditor.h \
    widgets/bargraphwidget.h \
    widgets/classifierwidget.h \
    widgets/classinputdialog.h \
    widgets/classselector.h \
    widgets/functionalisationdialog.h \
    widgets/generalsettings.h \
    widgets/infowidget.h \
    widgets/linegraphwidget.h \
    widgets/mainwindow.h \
    widgets/setsensorfailuresdialog.h \
    widgets/sourcedialog.h \
    widgets/usbsettingswidget.h


FORMS += \
    widgets/addattributedialog.ui \
    widgets/attributeeditor.ui \
    widgets/bargraphwidget.ui \
    widgets/classifierwidget.ui \
    widgets/classselector.ui \
    widgets/functionalisationdialog.ui \
    widgets/generalsettings.ui \
    widgets/infowidget.ui \
    widgets/linegraphwidget.ui \
    widgets/mainwindow.ui \
    widgets/setsensorfailuresdialog.ui \
    widgets/sourcedialog.ui \
    widgets/usbsettingswidget.ui

# Default rules for deployment.
# qnx: target.path = /tmp/$${TARGET}/bin
# else: unix:!android: target.path = /opt/$${TARGET}/bin
# !isEmpty(target.path): INSTALLS += target

DISTFILES += \
    documentation/source/overview.qdoc \
    icon.png \
    readme \
    project.qdocconf

RESOURCES += \
    eNoseAnnotator.qrc
