ARAN_SRC_ROOT = "/devel/devel/aran"
CML_ROOT = "/devel/devel/cml-1_0_1"
TINYXML_ROOT = "/devel/devel/tinyxml_2_5_3"
WORKING_ROOT = "/devel/devel/Working"

INCLUDEPATH += .
INCLUDEPATH += "$${ARAN_SRC_ROOT}/Src"
INCLUDEPATH += "$${ARAN_SRC_ROOT}/Src/Aran"
INCLUDEPATH += "$${ARAN_SRC_ROOT}/Src/AranMath"
INCLUDEPATH += "$${ARAN_SRC_ROOT}/Src/AranGl"
INCLUDEPATH += "$${ARAN_SRC_ROOT}/Src/AranPhy"
INCLUDEPATH += "$${ARAN_SRC_ROOT}/Src/AranIk"
INCLUDEPATH += "$${CML_ROOT}"
INCLUDEPATH += "$${TINYXML_ROOT}"
LIBPATH += "$${WORKING_ROOT}"
LIBPATH += "$${TINYXML_ROOT}"

CONFIG(debug, debug|release) {
    #error(Building debug.)
    TARGET = $$join(TARGET,,,D)
} else {
    #error(Building release.)
}

DESTDIR = "$${WORKING_ROOT}"

OBJECTS_DIR = ./Build
UI_DIR = ./Build
RCC_DIR = ./Build

CXXFLAGS += -fPIC
