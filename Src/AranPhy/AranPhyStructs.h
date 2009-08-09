#pragma once


enum BodyEnum
{
	BE_HAT,
	BE_GROIN_R,
	BE_GROIN_L,
	BE_CALF_R,
	BE_CALF_L,
	BE_FOOT_R,
	BE_FOOT_L,
	BE_COUNT,
	BE_NOT_BODY = 0x7fffffff
};

enum JointEnum
{
	JE_R1,
	JE_L1,
	JE_R2,
	JE_L2,
	JE_R3,
	JE_L3,
	JE_COUNT
};

enum JointAxisEnum
{
	JAE_R1X,
	JAE_R1Y,
	JAE_L1X,
	JAE_L1Y,
	JAE_R2X,
	JAE_L2X,
	JAE_R3X,
	JAE_R3Y,
	JAE_L3X,
	JAE_L3Y,
	JAE_COUNT
};

enum SwingLegEnum
{
	SLE_RIGHT,
	SLE_LEFT
};

static const int JointAxisEnumStringsCorr[] = {
	JE_R1,
	JE_R1,
	JE_L1,
	JE_L1,
	JE_R2,
	JE_L2,
	JE_R3,
	JE_R3,
	JE_L3,
	JE_L3
};

static const char* JointEnumStrings[] = {
	"R1XY",
	"L1XY",
	"R2X",
	"L2X",
	"R3XY",
	"L3XY"
};

static const char* JointAxisEnumStrings[] = {
	"R1X",
	"R1Y",
	"L1X",
	"L1Y",
	"R2X",
	"L2X",
	"R3X",
	"R3Y",
	"L3X",
	"L3Y"
};

static const char* BodyEnumStrings[] = {
	"HAT",
	"GROIN_R",
	"GROIN_L",
	"CALF_R",
	"CALF_L",
	"FOOT_R",
	"FOOT_L",
	"NOT_BODY"
};

/*!
@brief ODE 컨텍스트
*/
struct OdeSpaceContext
{
	dWorldID		world;			///< ODE World ID
	dSpaceID		space;			///< ODE Space ID
	dGeomID			plane;			///< ODE Plane ID: Z=0인 고정된 바닥(땅)
	dJointGroupID	contactGroup;	///< ODE JointGroup ID: 강체간 접촉 그룹 ID
};
