QT += core gui opengl

unix:!macx {
    LIBS += -lGLU
    QMAKE_CXXFLAGS += -std=c++11
}
macx {
    QMAKE_CFLAGS_X86_64 += -mmacosx-version-min=10.7
    QMAKE_CXXFLAGS_X86_64 = $$QMAKE_CFLAGS_X86_64
}

TARGET = final
TEMPLATE = app

# If you add your own folders, add them to INCLUDEPATH and DEPENDPATH, e.g.
INCLUDEPATH += glm lib shaders
DEPENDPATH += glm lib shaders

SOURCES += main.cpp \
    mainwindow.cpp \
    view.cpp \
    lib/ResourceLoader.cpp \
    camera.cpp \
    painterlyrender.cpp

HEADERS += mainwindow.h \
    view.h \
    lib/ResourceLoader.h \
    camera.h \
    painterlyrender.h

OTHER_FILES += \
    shaders/shader.frag \
    shaders/shader.vert \
    shaders/ssao.frag \
    shaders/grid.frag \
    shaders/blur.frag \
    shaders/dof.frag

FORMS += mainwindow.ui

macx {
    QMAKE_CXXFLAGS_WARN_ON -= -Warray-bounds -Wc++0x-compat
}

win32:CONFIG(release, debug|release): LIBS += -L/course/cs123/lib/glew/glew-1.10.0/lib/release/ -lGLEW
else:win32:CONFIG(debug, debug|release): LIBS += -L/course/cs123/lib/glew/glew-1.10.0/lib/debug/ -lGLEW
else:unix: LIBS += -L/usr/local/Cellar/glew/1.11.0/lib/ -lGLEW

LIBS += -L/course/cs123/lib/glew/glew-1.10.0/include -lGLEW
INCLUDEPATH += /course/cs123/lib/glew/glew-1.10.0/include
DEPENDPATH += /course/cs123/lib/glew/glew-1.10.0/include

INCLUDEPATH+=/usr/local/Cellar/glew/1.11.0/include
DEPENDPATH+=/usr/local/Cellar/glew/1.11.0/include

RESOURCES += \
    Resources.qrc \
    ImageResources.qrc
