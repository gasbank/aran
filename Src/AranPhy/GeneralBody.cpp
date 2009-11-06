#include "AranPhyPCH.h"
#include "GeneralBody.h"
#include "UtilFunc.h"
#include "ArnXformable.h"
#include "AranPhy.h"
#include "Structs.h"
#include "ArnSkeleton.h"
#include "ArnBone.h"
#include "ArnIntersection.h"
#include "SimWorld.h"

GeneralBody::GeneralBody(const OdeSpaceContext* osc)
: ArnObject(NDT_RT_GENERALBODY)
, m_osc(osc)
, m_body(0)
, m_geom(0)
, m_isContactGround(false)
, m_mass(0)
, m_abbt(ABBT_UNKNOWN)
, m_amdt(AMDT_UNKNOWN)
, m_xformable(0)
, m_bFixed(false)
{
	if (osc)
		m_body = dBodyCreate(m_osc->world);
}

GeneralBody::~GeneralBody()
{
}

void
GeneralBody::createGeomBox(const ArnVec3& dim)
{
	assert(m_geom == 0);
	m_geom = dCreateBox(m_osc->space, dim.x, dim.y, dim.z);
	dGeomSetBody(m_geom, m_body);
}

void
GeneralBody::createGeomCapsule(double radius, double height)
{
	assert(m_geom == 0);
	m_geom = dCreateCapsule(m_osc->space, radius, height);
	dGeomSetBody(m_geom, m_body);
}

void
GeneralBody::setBodyPosition(const ArnVec3& comPos)
{
	dBodySetPosition(m_body, comPos.x, comPos.y, comPos.z);
}

void
GeneralBody::getGeomSize(dVector3 out) const
{
	switch (dGeomGetClass(m_geom))
	{
		case dBoxClass:
			dGeomBoxGetLengths(m_geom, out);
			return;
		case dCapsuleClass:
		{
			double radius, length;
			dGeomCapsuleGetParams(m_geom, &radius, &length);
			out[0] = 2*radius;
			out[1] = 2*radius;
			out[2] = length;
			return;
		}
		default:
			abort();
	}
	return;
}

void
GeneralBody::getState(GeneralBodyState& gbs) const
{
	getPosition(&gbs.pos);
	getQuaternion(&gbs.quat);
	getLinearVel(&gbs.linVel);
	getAngularVel(&gbs.angVel);
}

void
GeneralBody::setState(const GeneralBodyState& gbs)
{
	dQuaternion odeQ = { gbs.quat.w, gbs.quat.x, gbs.quat.y, gbs.quat.z }; // odeQ[0] is the scalar component
	dBodySetPosition(m_body, gbs.pos[0], gbs.pos[1], gbs.pos[2]);
	dBodySetQuaternion(m_body, odeQ);
	dBodySetLinearVel(m_body, gbs.linVel[0], gbs.linVel[1], gbs.linVel[2]);
	dBodySetAngularVel(m_body, gbs.angVel[0], gbs.angVel[1], gbs.angVel[2]);
}

ArnVec3
GeneralBody::getRotationEuler() const
{
	const dReal* odeQuat = dBodyGetQuaternion(m_body);
	ArnQuat q(odeQuat[1], odeQuat[2], odeQuat[3], odeQuat[0]); // odeQuat[0] is the scalar component
	return ArnQuatToEuler(&q);
}

void
GeneralBody::addExternalForceOnCom(double x, double y, double z)
{
	dBodyAddForce(m_body, x, y, z);
}

void
GeneralBody::reset()
{
	dBodyID body = getBodyId();
	dQuaternion q = {1.0, 0.0, 0.0, 0.0};
	dBodySetAngularVel(body, 0.0, 0.0, 0.0);
	dBodySetLinearVel(body, 0.0, 0.0, 0.0);
	dBodySetQuaternion(body, q);
	dBodySetPosition(body, m_com0.x, m_com0.y, m_com0.z);
	dBodySetForce(body, 0.0, 0.0, 0.0);
	dBodySetTorque(body, 0.0, 0.0, 0.0);
	setContactGround(false);
}

bool
GeneralBody::isContactGround() const
{
	return m_isContactGround;
}

void
GeneralBody::render()
{
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
}

ArnVec3*
GeneralBody::getPosition( ArnVec3* pos ) const
{
	return ArnVec3Assign(pos, dBodyGetPosition(m_body));
}

ArnMatrix*
GeneralBody::getRotationMatrix( ArnMatrix* rotMat ) const
{
	// TODO: ODE Matrix row or columnwise not clear
	return OdeMatrixToDx(rotMat, dBodyGetRotation(m_body));
}

ArnQuat*
GeneralBody::getQuaternion( ArnQuat* q ) const
{
	return ArnQuatAssign_ScalarFirst(q, dBodyGetQuaternion(m_body));
}

ArnVec3*
GeneralBody::getLinearVel( ArnVec3* linVel ) const
{
	return ArnVec3Assign(linVel, dBodyGetLinearVel(m_body));
}

ArnVec3*
GeneralBody::getAngularVel( ArnVec3* angVel ) const
{
	return ArnVec3Assign(angVel, dBodyGetAngularVel(m_body));
}

void
GeneralBody::setBoundingBoxSize( const ArnVec3& size )
{
	switch (m_abbt)
	{
	case ABBT_BOX:
		m_boundingSize = size;
		break;
	default:
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
}

void
GeneralBody::setMassDistributionSize( const ArnVec3& size )
{
	switch (m_amdt)
	{
	case AMDT_BOX:
		m_massDistSize = size;
		break;
	case AMDT_CAPSULE:
		assert(size.z == 0);
		m_massDistSize = size;
		break;
	default:
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
}

void
GeneralBody::configureOdeContext( const OdeSpaceContext* osc )
{
	assert(m_osc == 0);
	assert(m_body == 0);
	assert(m_geom == 0);
	assert(m_abbt != ABBT_UNKNOWN);
	assert(m_amdt != AMDT_UNKNOWN);
	m_osc = osc;

	// Body and mass distribution setting
	m_body = dBodyCreate(m_osc->world);
	dBodySetData(getBodyId(), reinterpret_cast<void*>(getObjectId()));
	dMass mass;
	dMassSetZero(&mass);
	switch (m_amdt)
	{
	case AMDT_BOX:
		dMassSetBoxTotal(&mass, m_mass, m_massDistSize.x, m_massDistSize.y, m_massDistSize.z);
		break;
	case AMDT_CAPSULE:
		assert(m_massDistSize[2] == 0);
		dMassSetCapsuleTotal(&mass, m_mass, 3 /* Z-axis */, m_massDistSize[0], m_massDistSize[1]);
		break;
	default:
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
	dBodySetMass(m_body, &mass);
	dBodySetPosition(m_body, m_com0.x, m_com0.y, m_com0.z);

	// Geom and bounding box setting
	switch (m_abbt)
	{
	case ABBT_BOX:
		createGeomBox(m_boundingSize);
		break;
	case ABBT_CAPSULE:
		assert(m_boundingSize[2] == 0);
		createGeomCapsule(m_boundingSize[0], m_boundingSize[1]);
		break;
	default:
		break;
	}

	dQuaternion odeQ = { m_quat0.w, m_quat0.x, m_quat0.y, m_quat0.z };
	dBodySetQuaternion(m_body, odeQ);
	// TODO: Rigid body linear and angualr damping constants
	//dBodySetAngularDamping(m_body, 0.01);
	//dBodySetLinearDamping(m_body, 0.01);

	if (isFixed())
	{
		dBodySetKinematic(m_body);
	}
}

void
GeneralBody::notify() const
{
	if (m_xformable)
	{
		const dReal* pos = getPosition();
		const dReal* rot = getQuaternion();
		m_xformable->setLocalXform_Trans(ArnVec3(pos[0], pos[1], pos[2]));
		m_xformable->setLocalXform_Rot(ArnQuat(rot[1], rot[2], rot[3], rot[0]));
		m_xformable->recalcLocalXform();
	}
}

void
GeneralBody::setBodyData( void* data )
{
	dBodySetData(m_body, data);
}

void
GeneralBody::setGeomData( void* data )
{
	dGeomSetData(m_geom, data);
}

void*
GeneralBody::getGeomData() const
{
	return dGeomGetData(m_geom);
}

void
GeneralBody::setLinearVel(float x, float y, float z)
{
	dBodySetLinearVel(m_body, x, y, z);
}

static void
CalculateLumpedComAndMass(ArnVec3* com, float* mass, dBodyID body, dBodyID parentBody, int depth)
{
	assert(com && mass);
	int numJoint = dBodyGetNumJoints(body);
	const dReal* bodyPos = dBodyGetPosition(body);
	dMass bodyMass;
	dBodyGetMass(body, &bodyMass);

	dReal subMass = bodyMass.mass;
	ArnVec3 subCom = ArnVec3(bodyPos[0], bodyPos[1], bodyPos[2]) * subMass;

#if defined(ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING) & 0
	for (int i = 0; i < depth; ++i)
		std::cout << "  ";
	unsigned int objectId = reinterpret_cast<unsigned long>(dBodyGetData(body)) & 0xffffffff;
	ArnObject* arnObj = ArnObject::getObjectById(objectId);
	assert(arnObj);
	std::cout << arnObj->getName() << " " << bodyPos[0] << " " << bodyPos[1] << " " << bodyPos[2] << std::endl;
#endif // ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING

	for (int i = 0; i < numJoint; ++i)
	{
		dJointID joint = dBodyGetJoint(body, i);
		int jointType = dJointGetType(joint);
		if (jointType != dJointTypeBall
		 && jointType != dJointTypeHinge
		 && jointType != dJointTypeUniversal)
		{
		 	continue;
		}
		dBodyID body2 = dJointGetBody(joint, 0);
		if (body2 == body)
			body2 = dJointGetBody(joint, 1);
		if (body2 == parentBody)
			continue;

		ArnVec3 tempSubCom(0, 0, 0);
		float tempSubMass = 0;
		CalculateLumpedComAndMass(&tempSubCom, &tempSubMass, body2, body, depth + 1);

		subCom += tempSubCom * tempSubMass;
		subMass += tempSubMass;
	}
	assert(subMass);

	*com = subCom / subMass;
	*mass = subMass;
}

void
GeneralBody::calculateLumpedComAndMass(ArnVec3* com, float* mass) const
{
	CalculateLumpedComAndMass(com, mass, getBodyId(), getBodyId(), 0);
}


static void
CalculateLumpedIntersection(std::vector<ArnVec3>& isects, const ArnPlane& plane, dBodyID body, dBodyID parentBody, int depth)
{
	int numJoint = dBodyGetNumJoints(body);
	dGeomID geom = dBodyGetFirstGeom(body);
	assert(dGeomGetClass(geom) == dBoxClass);
	dVector3 boxSize;
	dGeomBoxGetLengths(geom, boxSize);
	/*
	const dReal* boxPos = dGeomGetPosition(geom);
	const dReal* boxQ = dGeomGetRotation(geom);
	*/
	const dReal* boxPos = dBodyGetPosition(body);
	const dReal* boxQ = dBodyGetQuaternion(body);
	ArnXformedBoxPlaneIntersection(
		isects,
		ArnVec3(boxSize[0], boxSize[1], boxSize[2]),
		ArnVec3(boxPos[0], boxPos[1], boxPos[2]),
		ArnQuat(boxQ[1], boxQ[2], boxQ[3], boxQ[0]),
		plane
	);
#if defined(ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING) & 0
	for (int i = 0; i < depth; ++i)
		std::cout << "  ";
	unsigned int objectId = reinterpret_cast<unsigned long>(dBodyGetData(body)) & 0xffffffff;
	ArnObject* arnObj = ArnObject::getObjectById(objectId);
	assert(arnObj);
	std::cout << arnObj->getName() << " " << boxPos[0] << " " << boxPos[1] << " " << boxPos[2] << std::endl;
#endif // ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING

	for (int i = 0; i < numJoint; ++i)
	{
		dJointID joint = dBodyGetJoint(body, i);
		int jointType = dJointGetType(joint);
		if (jointType != dJointTypeBall
		 && jointType != dJointTypeHinge
		 && jointType != dJointTypeUniversal)
		{
		 	continue;
		}
		dBodyID body2 = dJointGetBody(joint, 0);
		if (body2 == body)
			body2 = dJointGetBody(joint, 1);
		if (body2 == parentBody)
			continue;

		CalculateLumpedIntersection(isects, plane, body2, body, depth + 1);
	}
}

static void
CalculateLumpedGroundIntersection(std::vector<ArnVec3>& isects, dBodyID body, dBodyID parentBody, int depth)
{
	int numJoint = dBodyGetNumJoints(body);
	dGeomID geom = dBodyGetFirstGeom(body);
	assert(dGeomGetClass(geom) == dBoxClass);
	dVector3 boxSize;
	dGeomBoxGetLengths(geom, boxSize);
	/*
	const dReal* boxPos = dGeomGetPosition(geom);
	const dReal* boxQ = dGeomGetRotation(geom);
	*/

	const dReal* boxPos = dBodyGetPosition(body);
	const dReal* boxQ = dBodyGetQuaternion(body);

	const float x = boxSize[0] / 2;
	const float y = boxSize[1] / 2;
	const float z = boxSize[2] / 2;

	ArnVec3 p[8];
	p[0].set(  x,  y,  z);
	p[1].set(  x,  y, -z);
	p[2].set(  x, -y,  z);
	p[3].set(  x, -y, -z);
	p[4].set( -x,  y,  z);
	p[5].set( -x,  y, -z);
	p[6].set( -x, -y,  z);
	p[7].set( -x, -y, -z);
	for (int i = 0; i < 8; ++i)
	{
		ArnMatrix rotMat;
		ArnQuat q(boxQ[1], boxQ[2], boxQ[3], boxQ[0]);
		q.getRotationMatrix(&rotMat);
		ArnVec3 tp;
		ArnVec3TransformCoord(&tp, &p[i], &rotMat);
		p[i] = tp;
		p[i].x += boxPos[0];
		p[i].y += boxPos[1];
		p[i].z += boxPos[2];

		//if (-0.001f <= p[i].z && p[i].z <= 0.001f)
		//{
			isects.push_back(p[i]);
		//}
	}

#if defined(ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING) & 0
	for (int i = 0; i < depth; ++i)
		std::cout << "  ";
	unsigned int objectId = reinterpret_cast<unsigned long>(dBodyGetData(body)) & 0xffffffff;
	ArnObject* arnObj = ArnObject::getObjectById(objectId);
	assert(arnObj);
	std::cout << arnObj->getName() << " " << boxPos[0] << " " << boxPos[1] << " " << boxPos[2] << std::endl;
#endif // ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING

	for (int i = 0; i < numJoint; ++i)
	{
		dJointID joint = dBodyGetJoint(body, i);
		int jointType = dJointGetType(joint);
		if (jointType != dJointTypeBall
		 && jointType != dJointTypeHinge
		 && jointType != dJointTypeUniversal)
		{
		 	continue;
		}
		dBodyID body2 = dJointGetBody(joint, 0);
		if (body2 == body)
			body2 = dJointGetBody(joint, 1);
		if (body2 == parentBody)
			continue;

		CalculateLumpedGroundIntersection(isects, body2, body, depth + 1);
	}
}

void
GeneralBody::calculateLumpedIntersection(std::vector<ArnVec3>& isects, const ArnPlane& plane) const
{
	CalculateLumpedIntersection(isects, plane, getBodyId(), getBodyId(), 0);
}

void
GeneralBody::calculateLumpedGroundIntersection(std::vector<ArnVec3>& isects) const
{
	CalculateLumpedGroundIntersection(isects, getBodyId(), getBodyId(), 0);
}

ArnVec3
ArnxGetJointAnchor(dJointID j)
{
	dVector3 anc;
	switch (dJointGetType(j))
	{
	case dJointTypeHinge:
		dJointGetHingeAnchor(j, anc);
		break;
	case dJointTypeUniversal:
		dJointGetUniversalAnchor(j, anc);
		break;
	case dJointTypeBall:
		dJointGetBallAnchor(j, anc);
		break;
	default:
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
	return ArnVec3(anc[0], anc[1], anc[2]);
}

static void
ArnxCreateLumpedArnSkeleton(ArnXformable* parent, SimWorldPtr swPtr, const GeneralBody* body, const GeneralBody* parentBody, const int depth)
{
	assert(body && parentBody);
	int numJoint = dBodyGetNumJoints(body->getBodyId());
	// 'parentWorldXform' is the frame located and oriented at the tail of 'parent'
	// only if 'parent' has a type of ArnBone.
	ArnMatrix parentWorldXform = parent->computeWorldXform(); // Get the frame located at the head.
	if (parent->getType() == NDT_RT_BONE)
	{
		// Move the frame along the bone direction to make it the tail frame of parent bone.
		ArnVec3 parentBoneTailDiff = parentWorldXform.getColumnVec3(1) * static_cast<ArnBone*>(parent)->getBoneLength();
		parentWorldXform.m[0][3] += parentBoneTailDiff.x;
		parentWorldXform.m[1][3] += parentBoneTailDiff.y;
		parentWorldXform.m[2][3] += parentBoneTailDiff.z;
	}
	ArnMatrix parentWorldXformInv;
	ArnMatrixInverse(&parentWorldXformInv, 0, &parentWorldXform);

	const ArnVec3& head(ArnConsts::ARNVEC3_ZERO);
	bool childBoneAdded = false;
	for (int i = 0; i < numJoint; ++i)
	{
		dJointID joint = dBodyGetJoint(body->getBodyId(), i);
		int jointType = dJointGetType(joint);
		if (jointType != dJointTypeBall
		 && jointType != dJointTypeHinge
		 && jointType != dJointTypeUniversal)
		{
		 	continue;
		}
		dBodyID body2Id = dJointGetBody(joint, 0);
		GeneralBodyPtr body2 = swPtr->getBodyByBodyIdFromSet(body2Id);
		assert(body2);
		if (body2->getBodyId() == body->getBodyId())
		{
			body2Id = dJointGetBody(joint, 1);
			body2 = swPtr->getBodyByBodyIdFromSet(body2Id);
			assert(body2);
		}
		if (body2->getBodyId() == parentBody->getBodyId())
		{
			continue;
		}
		ArnVec3 tail(ArnxGetJointAnchor(joint));
		ArnVec3TransformCoord(&tail, &tail, &parentWorldXformInv);
		const ArnVec3 boneDir = tail - head;
		float length = ArnVec3Length(boneDir);
		float roll = 0;
		ArnBone* b = ArnBone::createFrom(length, roll);
		ArnVec3 rotAxis = ArnVec3GetCrossProduct(ArnConsts::ARNVEC3_Y, boneDir);
		const float rotAngle = acos(ArnVec3Dot(ArnConsts::ARNVEC3_Y, boneDir/length));
		ArnQuat q;
		rotAxis /= ArnVec3Length(rotAxis);
		ArnQuaternionRotationAxis(&q, &rotAxis, rotAngle);
		b->setLocalXform_Rot(q);

		// TODO: Set the bone name accordingly
		std::string boneName("XYZ_");
		boneName += body->getName();
		boneName += "//";
		boneName += body2->getName();
		b->setName(boneName.c_str());

		if (parent->getType() == NDT_RT_BONE)
		{
			b->setLocalXform_Trans(ArnVec3(0, static_cast<ArnBone*>(parent)->getBoneLength(), 0));
		}
		b->recalcLocalXform();
		//b->getLocalXform().printFrameInfo();
		parent->attachChild(b);
		childBoneAdded = true;
		ArnxCreateLumpedArnSkeleton(b, swPtr, body2.get(), body, depth+1);
	}
	if (!childBoneAdded)
	{
		// No child bone added --> This is an endeffector.

		const dReal* bodyCom = dBodyGetPosition(body->getBodyId());
		ArnVec3 tail(bodyCom[0], bodyCom[1], bodyCom[2]);
		ArnVec3TransformCoord(&tail, &tail, &parentWorldXformInv);
		const ArnVec3 boneDir = tail - head;
		float length = ArnVec3Length(boneDir);
		float roll = 0;
		ArnBone* b = ArnBone::createFrom(length, roll);
		ArnVec3 rotAxis = ArnVec3GetCrossProduct(ArnConsts::ARNVEC3_Y, boneDir);
		const float rotAngle = acos(ArnVec3Dot(ArnConsts::ARNVEC3_Y, boneDir/length));
		ArnQuat q;
		rotAxis /= ArnVec3Length(rotAxis);
		ArnQuaternionRotationAxis(&q, &rotAxis, rotAngle);
		b->setLocalXform_Rot(q);

		// Set the bone name accordingly
		std::string boneName("E_");
		boneName += body->getName();
		b->setName(boneName.c_str());

		if (parent->getType() == NDT_RT_BONE)
		{
			b->setLocalXform_Trans(ArnVec3(0, static_cast<ArnBone*>(parent)->getBoneLength(), 0));
		}
		b->recalcLocalXform();
		//b->getLocalXform().printFrameInfo();
		parent->attachChild(b);
	}
}

ArnSkeleton*
GeneralBody::createLumpedArnSkeleton(SimWorldPtr swPtr) const
{
	ArnSkeleton* skel = ArnSkeleton::createFromEmpty();
	const ArnVec3 skelRootPos(getPosition()[0], getPosition()[1], getPosition()[2]);
	const ArnQuat skelRootQ(getQuaternion()[1], getQuaternion()[2], getQuaternion()[3], getQuaternion()[0]);
	skel->setLocalXform_Trans(skelRootPos);
	skel->setLocalXform_Rot(skelRootQ);
	skel->recalcLocalXform();
	skel->setName(getName());
	//skel->getLocalXform().printFrameInfo();
	ArnxCreateLumpedArnSkeleton(skel, swPtr, this, this, 0);
	return skel;
}

static float
ArnxCalculateLumpedVerticalIntersection(std::vector<ArnVec3>& isects, std::vector<std::vector<float> >& massMap, dBodyID body, dBodyID parentBody, int depth,  const float cx, const float cy, const float deviation, const int resolution, const int row, const int col, float maxMassMapVal)
{
	int numJoint = dBodyGetNumJoints(body);
	dGeomID geom = dBodyGetFirstGeom(body);
	assert(dGeomGetClass(geom) == dBoxClass);
	dVector3 boxSize;
	dGeomBoxGetLengths(geom, boxSize);
	const dReal* boxPos = dBodyGetPosition(body);
	const dReal* boxQ = dBodyGetQuaternion(body);

#if defined(ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING) & 0
	for (int i = 0; i < depth; ++i)
		std::cout << "  ";
	unsigned int objectId = reinterpret_cast<unsigned long>(dBodyGetData(body)) & 0xffffffff;
	ArnObject* arnObj = ArnObject::getObjectById(objectId);
	assert(arnObj);
	std::cout << arnObj->getName() << " " << boxPos[0] << " " << boxPos[1] << " " << boxPos[2] << std::endl;
#endif // ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING

	float massWeight = ArnXformedBoxVerticalLineIntersection(isects, ArnVec3(boxSize[0], boxSize[1], boxSize[2]), ArnVec3(boxPos[0], boxPos[1], boxPos[2]), ArnQuat(boxQ[1], boxQ[2], boxQ[3], boxQ[0]), cx + deviation / resolution * row,  cy + deviation / resolution * col);
	dMass mass;
	dBodyGetMass(body, &mass);
	massMap[row + resolution][col + resolution] += mass.mass * massWeight;
	if (maxMassMapVal < massMap[row + resolution][col + resolution])
		maxMassMapVal = massMap[row + resolution][col + resolution];

	for (int i = 0; i < numJoint; ++i)
	{
		dJointID joint = dBodyGetJoint(body, i);
		int jointType = dJointGetType(joint);
		if (jointType != dJointTypeBall
			&& jointType != dJointTypeHinge
			&& jointType != dJointTypeUniversal)
		{
			continue;
		}
		dBodyID body2 = dJointGetBody(joint, 0);
		if (body2 == body)
			body2 = dJointGetBody(joint, 1);
		if (body2 == parentBody)
			continue;

		float maxVal = ArnxCalculateLumpedVerticalIntersection(isects, massMap, body2, body, depth + 1, cx, cy, deviation, resolution, row, col, maxMassMapVal);
		if (maxMassMapVal < maxVal)
			maxMassMapVal = maxVal;
	}

	return maxMassMapVal;
}

float
GeneralBody::calculateLumpedVerticalIntersection( std::vector<ArnVec3>& isects, std::vector<std::vector<float> >& massMap, const float cx, const float cy, const float deviation, const int resolution ) const
{
	float maxMassMapVal = 0;
	for (int i = -resolution; i < resolution; ++i)
	{
		for (int j = -resolution; j < resolution; ++j)
		{
			float maxVal = ArnxCalculateLumpedVerticalIntersection(isects, massMap, getBodyId(), getBodyId(), 0, cx, cy, deviation, resolution, i, j, maxMassMapVal );
			if (maxMassMapVal < maxVal)
				maxMassMapVal = maxVal;
		}
	}
	return maxMassMapVal;
}
