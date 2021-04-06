QT += core gui websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    "C:/Program Files (x86)/taglib/include/taglib" \

LIBS += \
    -L"C:\taglib\taglib" -ltag

LIBS += C:/C/bass.lib

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    playlistreader.cpp \
    settingswindow.cpp \
    song.cpp

HEADERS += \
    mainwindow.h \
    playlistreader.h \
    settingswindow.h \
    song.h

FORMS += \
    mainwindow.ui \
    settingswindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32: LIBS += -L$$PWD/'../../../../../Program Files (x86)/taglib/lib/' -ltag

INCLUDEPATH += $$PWD/'../../../../../Program Files (x86)/taglib/include'
DEPENDPATH += $$PWD/'../../../../../Program Files (x86)/taglib/include'

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/'../../../../../Program Files (x86)/taglib/lib/tag.lib'
else:win32-g++: PRE_TARGETDEPS += $$PWD/'../../../../../Program Files (x86)/taglib/lib/libtag.a'

win32: LIBS += -L$$PWD/'../../../../../Program Files (x86)/taglib/lib/' -ltag

INCLUDEPATH += $$PWD/'../../../../../Program Files (x86)/taglib/include'
DEPENDPATH += $$PWD/'../../../../../Program Files (x86)/taglib/include'

RESOURCES += \
    resources.qrc
