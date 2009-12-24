TEMPLATE = app
CONFIG += precompile_header

# CONFIG += console
DEPENDPATH += . \
    Build
include(/devel/devel/aran/Src/PathInc.pro)
QT += opengl
DEFINES += dDOUBLE
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
    IkSolverTreeModel.h
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
    IkSolverTreeModel.cpp
FORMS += 
