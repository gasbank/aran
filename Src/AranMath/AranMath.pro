TEMPLATE = lib
CONFIG += dll precompile_header
DEPENDPATH += .

include(../PathInc.pro)

win32 {
DEFINES += WIN32 _USRDLL ARANMATH_EXPORTS
#CONFIG -= embed_manifest_dll
}

# Input
PRECOMPILED_HEADER = AranMathPCH.h
HEADERS += AranMathPCH.h \
           AranMathTypeDefs.h \
           ArnColorValue4f.h \
           ArnConsts.h \
           ArnIntersection.h \
           ArnMath.h \
           ArnMatrix.h \
           ArnPlane.h \
           ArnQuat.h \
           ArnVec3.h \
           ArnVec4.h \
           ArnViewportData.h \
           ArnVec3.inl \
           ArnMath.inl
SOURCES += AranMathPCH.cpp \
           ArnConsts.cpp \
           ArnIntersection.cpp \
           ArnMath.cpp \
           ArnMatrix.cpp \
           ArnPlane.cpp \
           ArnQuat.cpp \
           ArnVec3.cpp \
           ArnVec4.cpp
