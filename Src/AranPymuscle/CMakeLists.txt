LINK_DIRECTORIES(
${CODEBASE}src/CHOLMOD/Debug
)
ADD_EXECUTABLE(AranPymuscle
BwAppContext.cpp BwAppContext.h
BwDrawingOptionsWindow.cpp BwDrawingOptionsWindow.h
BwMain.cpp BwMain.h
BwOpenGlWindow.cpp BwOpenGlWindow.h
BwPch.cpp BwPch.h
BwPlaybackSlider.cpp BwPlaybackSlider.h
BwTopWindow.cpp BwTopWindow.h
SliderInput.cpp SliderInput.h
BwDebugPrintOptionsWindow.cpp BwDebugPrintOptionsWindow.h
BwRbTrackingOptionsWindow.cpp BwRbTrackingOptionsWindow.h
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
../pymuscle/core
../pymuscle/parser
../pymuscle/optimizer
../pymuscle/rs
../pymuscle
../Jarvis
${CODEBASE}freetype-2.3.9/include
${CODEBASE}opende/include
${CODEBASE}fltk-1.1.10
${CODEBASE}src/glew-1.5.7/include
${CODEBASE}src/UFconfig
${CODEBASE}src/UMFPACK/Include
${CODEBASE}src/AMD/Include
${CODEBASE}src/CHOLMOD/Include
${CODEBASE}src/libconfig-1.3.2
${MOSEK_ROOT}6/tools/platform/win32x86/h
)
TARGET_LINK_LIBRARIES(AranPymuscle
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
pymcore
pymoptimizer
pymparser
pymrs
DevIL
)
ADD_MSVC_PRECOMPILED_HEADER(AranPymuscle BwPch.h BwPch.cpp)
