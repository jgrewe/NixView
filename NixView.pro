#-------------------------------------------------
#
# Project created by QtCreator 2015-07-20T12:57:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NixView
TEMPLATE = app

QMAKE_CXXFLAGS+= -std=c++11


SOURCES += main.cpp\
        MainWindow.cpp \
    MainViewWidget.cpp \
    RawTreeView.cpp \
    Test/AnotherTree.cpp \
    InfoWidget/InfoWidget.cpp

HEADERS  += MainWindow.hpp \
    MainViewWidget.hpp \
    RawTreeView.hpp \
    Test/AnotherTree.hpp \
    InfoWidget/InfoWidget.hpp

FORMS    += MainWindow.ui \
    MainViewWidget.ui \
    RawTreeView.ui \
    Test/AnotherTree.ui \
    InfoWidget/InfoWidget.ui


#standard windows folder?
#win32:CONFIG(release, debug|release): LIBS += /usr/local/lib/release/ -lnix
#else:win32:CONFIG(debug, debug|release): LIBS += /usr/local/lib/debug/ -lnix
#else:unix: LIBS += /usr/local/lib/ -lnix

unix: LIBS += -L/usr/local/lib/ -lnix

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include
