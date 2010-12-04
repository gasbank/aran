LINK_DIRECTORIES(
)
ADD_EXECUTABLE(AranMath_Test
MathTest.cpp
version.h
)
INCLUDE_DIRECTORIES(
..
../AranMath
)
TARGET_LINK_LIBRARIES(AranMath_Test
AranMath
)
#ADD_MSVC_PRECOMPILED_HEADER(AranIk_Test BwPch.h BwPch.cpp)
