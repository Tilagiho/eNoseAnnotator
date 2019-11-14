QT += testlib
QT += gui
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

SOURCES +=  tst_enoseannotator.cpp \
    tst_mvector.cpp \
    ../app/mvector.cpp \

HEADERS += \
    ../app/mvector.h \
