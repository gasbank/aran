win32 {
ARAN_SRC_ROOT = "d:/Devel3/aran"
CML_ROOT = "d:/devel3/cml-1_0_1"
TINYXML_ROOT = "d:/Devel3/tinyxml_2_5_3"
WORKING_ROOT = "D:/Devel3/Working"

BOOST_PATH = "e:/boost_1_37_0"
DEVIL_PATH = "D:/Devel3/DevIL-SDK-x86-1.7.8"
OPENDE_PATH = "D:/Devel3/opende"
ZLIB_PATH = D:/Devel3/zlib-1.2.3
DIRECTX_PATH = "C:/Program Files/Microsoft DirectX SDK (June 2008)"
}

unix {
ARAN_SRC_ROOT = "/devel/devel/aran"
CML_ROOT = "/devel/devel/cml-1_0_1"
TINYXML_ROOT = "/devel/devel/tinyxml_2_5_3"
WORKING_ROOT = "/devel/devel/Working"
}

INCLUDEPATH += .
INCLUDEPATH += "$${ARAN_SRC_ROOT}/Src"
INCLUDEPATH += "$${ARAN_SRC_ROOT}/Src/Aran"
INCLUDEPATH += "$${ARAN_SRC_ROOT}/Src/AranMath"
INCLUDEPATH += "$${ARAN_SRC_ROOT}/Src/AranGl"
INCLUDEPATH += "$${ARAN_SRC_ROOT}/Src/AranPhy"
INCLUDEPATH += "$${ARAN_SRC_ROOT}/Src/AranIk"
INCLUDEPATH += "$${CML_ROOT}"
INCLUDEPATH += "$${TINYXML_ROOT}"

win32 {
INCLUDEPATH += $${BOOST_PATH}
INCLUDEPATH += $${DEVIL_PATH}/include
INCLUDEPATH += $${OPENDE_PATH}/include
INCLUDEPATH += $${ZLIB_PATH}
INCLUDEPATH += $${DIRECTX_PATH}/Include

# Common libraries for win32 build
LIBS += opengl32.lib glu32.lib winmm.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib user32.lib
}

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

unix {
CXXFLAGS += -fPIC
}
