TEMPLATE = lib
CONFIG += dll precompile_header
DEPENDPATH += .

include(../PathInc.pro)

# Input
PRECOMPILED_HEADER = AranIkPCH.h
HEADERS += AranIk.h \
           AranIkPCH.h \
           ArnIkSolver.h \
           Jacobian.h \
           LinearR2.h \
           LinearR3.h \
           LinearR4.h \
           MathMisc.h \
           MatrixRmn.h \
           Node.h \
           RgbImage.h \
           Spherical.h \
           Tree.h \
           VectorRn.h
SOURCES += AranIk.cpp \
           AranIkPCH.cpp \
           ArnIkSolver.cpp \
           Jacobian.cpp \
           LinearR2.cpp \
           LinearR3.cpp \
           LinearR4.cpp \
           MatrixRmn.cpp \
           Misc.cpp \
           Node.cpp \
           RgbImage.cpp \
           Tree.cpp \
           VectorRn.cpp
