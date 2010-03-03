TEMPLATE = app
CONFIG += precompile_header

# CONFIG += console
DEPENDPATH += . \
    Build
include(../PathInc.pro)
win32 { 
    DEFINES += WIN32
    
    # CONFIG -= embed_manifest_exe
    CONFIG(debug, debug|release) { 
        LIBS += $${WORKING_ROOT}/AranMathD.lib
        LIBS += $${WORKING_ROOT}/AranD.lib
        LIBS += $${WORKING_ROOT}/AranIkD.lib
        LIBS += $${WORKING_ROOT}/AranPhyD.lib
        LIBS += $${WORKING_ROOT}/AranGlD.lib
    }
    else { 
        LIBS += $${WORKING_ROOT}/AranMath.lib
        LIBS += $${WORKING_ROOT}/Aran.lib
        LIBS += $${WORKING_ROOT}/AranIk.lib
        LIBS += $${WORKING_ROOT}/AranPhy.lib
        LIBS += $${WORKING_ROOT}/AranGl.lib
    }
}
unix { 
    CONFIG(debug, debug|release):LIBS += -lAranD \
        -lAranMathD \
        -lAranGlD \
        -lAranIkD \
        -lAranPhyD \
        -lIL \
        -lode
    else:LIBS += -lAran \
        -lAranMath \
        -lAranGl \
        -lAranIk \
        -lAranPhy \
        -lIL \
        -lode
}
QT += opengl
DEFINES += dDOUBLE

# Input
PRECOMPILED_HEADER = TreePch.h
HEADERS += Main.h \
    TreeMainWindow.h \
    TreeModel.h \
    TreeItem.h \
    TreePch.h \
    SceneGraphModel.h \
    SceneGraphItem.h \
    GlWindow.h \
    ViewOptions.h \
    NodeProperties.h \
    NodePropertiesModel.h \
    IkSolverProperties.h \
    IkSolverTreeModel.h \
    IkSolverNodeModel.h
SOURCES += Main.cpp \
    TreeMainWindow.cpp \
    TreeModel.cpp \
    TreeItem.cpp \
    SceneGraphModel.cpp \
    SceneGraphItem.cpp \
    GlWindow.cpp \
    ViewOptions.cpp \
    NodeProperties.cpp \
    NodePropertiesModel.cpp \
    IkSolverProperties.cpp \
    IkSolverTreeModel.cpp \
    IkSolverNodeModel.cpp
FORMS += 
