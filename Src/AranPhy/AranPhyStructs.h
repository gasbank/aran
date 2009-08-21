/*!
 * @file AranPhyStructs.h
 * @author Geoyeob Kim
 * @date 2009
 *
 * ARAN Physics Package (AranPhy) Common Structure Definition
 */
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

/*!
 * @brief ODE 컨텍스트
 *
 * OpenDE의 실행 컨텍스트 자료를 모아놓은 구조체입니다.
 * SimWorld 객체에 고유하게 하나씩 할당됩니다.
 */
struct OdeSpaceContext
{
	dWorldID			world;								///< ODE World ID
	dSpaceID			space;								///< ODE Space ID
	dGeomID				plane;								///< ODE Plane ID: Z=0인 고정된 바닥(땅)
	dJointGroupID		contactGroup;						///< ODE JointGroup ID: 강체간 접촉 그룹 ID
	static const int	MAXIMUM_CONTACT_COUNT = 100;		///< 최대 접촉점 확인 개수
	int					numContact;							///< 접촉점 저장 공간의 유효한 요소 개수
	dContact			contacts[MAXIMUM_CONTACT_COUNT];	///< 접촉점 저장 공간
};
