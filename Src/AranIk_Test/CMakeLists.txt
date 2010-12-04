LINK_DIRECTORIES(
)
ADD_EXECUTABLE(AranIk_Test
BwAppContext.cpp BwAppContext.h
BwDrawingOptionsWindow.cpp BwDrawingOptionsWindow.h
BwMain.cpp BwMain.h
BwOpenGlWindow.cpp BwOpenGlWindow.h
BwPch.cpp BwPch.h
BwPlaybackSlider.cpp BwPlaybackSlider.h
BwTopWindow.cpp BwTopWindow.h
SliderInput.cpp SliderInput.h
)
ADD_DEFINITIONS(
-DTIXML_USE_STL
-DdDOUBLE
-DBOOST_FILESYSTEM_VERSION=3
)
INCLUDE_DIRECTORIES(
..
../AranMath
../Aran
../AranPhy
../AranIk
../AranGl
${CODEBASE}freetype-2.3.9/include
${CODEBASE}opende/include
${CODEBASE}fltk-1.1.10
${CODEBASE}src/glew-1.5.7/include
${CODEBASE}src/libconfig-1.3.2
)
TARGET_LINK_LIBRARIES(AranIk_Test
AranMath
Aran
AranPhy
AranIk
AranGl
opengl32
glu32
glew32
fltkd
fltkgld
comctl32
wsock32
winmm
DevIL
)
ADD_MSVC_PRECOMPILED_HEADER(AranIk_Test BwPch.h BwPch.cpp)
