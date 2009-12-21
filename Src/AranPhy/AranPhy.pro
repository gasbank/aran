TEMPLATE = lib
CONFIG += dll precompile_header
DEPENDPATH += .

include(../PathInc.pro)

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
