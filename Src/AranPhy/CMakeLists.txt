ADD_LIBRARY(AranPhy SHARED
AngularJoint.cpp AngularJoint.h
AranPhy.cpp AranPhy.h
AranPhyPCH.cpp AranPhyPCH.h
ArnPhyBox.cpp ArnPhyBox.h
BallSocketJoint.cpp BallSocketJoint.h
Biped.cpp Biped.h
GeneralBody.cpp GeneralBody.h
GeneralJoint.cpp GeneralJoint.h
HingeJoint.cpp HingeJoint.h
LinearJoint.cpp LinearJoint.h
SimWorld.cpp SimWorld.h
SliderJoint.cpp SliderJoint.h
UniversalJoint.cpp UniversalJoint.h
UtilFunc.cpp UtilFunc.h
)
INCLUDE_DIRECTORIES(
..
../AranMath
../Aran
)
ADD_DEFINITIONS(
-D_USRDLL
-DARANPHY_EXPORTS
-DdDOUBLE
)
TARGET_LINK_LIBRARIES(AranPhy
AranMath
Aran
ode_doubled
)
ADD_MSVC_PRECOMPILED_HEADER(AranPhy AranPhyPCH.h AranPhyPCH.cpp)
