TEMPLATE = lib
CONFIG += dll precompile_header
DEPENDPATH += .

include(../PathInc.pro)

win32 {
    DEFINES += WIN32 \
        _USRDLL \
        ARANPHY_EXPORTS
    #CONFIG -= embed_manifest_dll
    CONFIG(debug, debug|release) {
        LIBS += $${WORKING_ROOT}/AranMathD.lib
        LIBS += $${WORKING_ROOT}/AranD.lib
        LIBS += $${OPENDE_PATH}/lib/DebugDoubleDLL/ode_doubled.lib
    }
    else {
        LIBS += $${WORKING_ROOT}/AranMath.lib
        LIBS += $${WORKING_ROOT}/Aran.lib
        LIBS += $${OPENDE_PATH}/lib/ReleaseDoubleDLL/ode_double.lib
    }
}

# OpenDE physics engine with double precision
DEFINES += dDOUBLE

# Input
PRECOMPILED_HEADER = AranPhyPCH.h
HEADERS += AngularJoint.h \
           AranPhy.h \
           AranPhyPCH.h \
           AranPhyStructs.h \
           ArnPhyBox.h \
           BallSocketJoint.h \
           Biped.h \
           GeneralBody.h \
           GeneralJoint.h \
           HingeJoint.h \
           LinearJoint.h \
           SimWorld.h \
           SliderJoint.h \
           targetver.h \
           UniversalJoint.h \
           UtilFunc.h \
           GeneralJoint.inl
SOURCES += AngularJoint.cpp \
           AranPhy.cpp \
           AranPhyPCH.cpp \
           ArnPhyBox.cpp \
           BallSocketJoint.cpp \
           Biped.cpp \
           GeneralBody.cpp \
           GeneralJoint.cpp \
           HingeJoint.cpp \
           LinearJoint.cpp \
           SimWorld.cpp \
           SliderJoint.cpp \
           UniversalJoint.cpp \
           UtilFunc.cpp
