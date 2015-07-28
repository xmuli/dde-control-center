QT += widgets

HEADERS    = frame.h \
             interfaces.h \
    homescreen.h \
    modulemetadata.h \
    constants.h \
    sidebar.h \
    contentview.h \
    dclickablelabel.h \
    dtipsframe.h
SOURCES    = frame.cpp \
             main.cpp \
    homescreen.cpp \
    sidebar.cpp \
    contentview.cpp \
    dclickablelabel.cpp \
    dtipsframe.cpp

TARGET     = dde-control-center
DESTDIR    = $$_PRO_FILE_PWD_/../

CONFIG += c++11
