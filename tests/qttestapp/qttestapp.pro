TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../../

CONFIG += static

QT += webkit opengl network

unix {
    LIBS += $$PWD/../../libllqtwebkit.so
}

!mac {
unix {
    DEFINES += LL_LINUX
}
}

mac {
    DEFINES += LL_OSX
}


win32{
    DEFINES += _WINDOWS
    INCLUDEPATH += ../
    LIBS += -L../GL 
    DESTDIR=../GL
    debug {
      LIBS += $$PWD/../../Debug/llqtwebkitd.lib
    }
    release {
      LIBS += $$PWD/../../Release/llqtwebkit.lib
    }
}

# Input
SOURCES += main.cpp
FORMS += window.ui
