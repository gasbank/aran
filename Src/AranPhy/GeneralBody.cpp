#include "AranPhyPCH.h"
#include "GeneralBody.h"
#include "UtilFunc.h"
#include "ArnXformable.h"

GeneralBody::GeneralBody(const OdeSpaceContext* osc)
: m_osc(osc)
, m_isContactGround(false)
, m_body(0)
, m_geom(0)
, m_abbt(ABBT_UNKNOWN)
, m_amdt(AMDT_UNKNOWN)
, m_xformable(0)
{
	if (osc)
		m_body = dBodyCreate(m_osc->world);
}

GeneralBody::~GeneralBody()
{
}

void GeneralBody::createGeomBox(const ArnVec3& dim)
{
	assert(m_geom == 0);
	m_geom = dCreateBox(m_osc->space, dim.x, dim.y, dim.z);
	dGeomSetBody(m_geom, m_body);
}

void GeneralBody::createGeomCapsule(double radius, double height)
{
	assert(m_geom == 0);
	m_geom = dCreateCapsule(m_osc->space, radius, height);
	dGeomSetBody(m_geom, m_body);
}

void GeneralBody::setBodyPosition(const ArnVec3& comPos)
{
	dBodySetPosition(m_body, comPos.x, comPos.y, comPos.z);
}

void GeneralBody::getGeomSize(dVector3 out) const
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

void GeneralBody::getState(GeneralBodyState& gbs) const
{
	getPosition(&gbs.pos);
	getQuaternion(&gbs.quat);
	getLinearVel(&gbs.linVel);
	getAngularVel(&gbs.angVel);
}

void GeneralBody::setState(const GeneralBodyState& gbs)
{
	dQuaternion odeQ = { gbs.quat.w, gbs.quat.x, gbs.quat.y, gbs.quat.z }; // ODE quaternion sequence is scalar first
	dBodySetPosition(m_body, gbs.pos[0], gbs.pos[1], gbs.pos[2]);
	dBodySetQuaternion(m_body, odeQ);
	dBodySetLinearVel(m_body, gbs.linVel[0], gbs.linVel[1], gbs.linVel[2]);
	dBodySetAngularVel(m_body, gbs.angVel[0], gbs.angVel[1], gbs.angVel[2]);
}

ArnVec3 GeneralBody::getRotationEuler() const
{
	const dReal* odeQuat = dBodyGetQuaternion(m_body);
	// TODO: ODE quaternion sequence
	ArnQuat q(odeQuat[1], odeQuat[2], odeQuat[3], odeQuat[0]); // ODE quaternion sequence is scalar first
	return ArnQuatToEuler(&q);
}

void GeneralBody::addExternalForceOnCom(double x, double y, double z)
{
	dBodyAddForce(m_body, x, y, z);
}

void GeneralBody::reset()
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

bool GeneralBody::isContactGround() const
{
	return m_isContactGround;
}

void GeneralBody::render()
{
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
}

ArnVec3* GeneralBody::getPosition( ArnVec3* pos ) const
{
	return ArnVec3Assign(pos, dBodyGetPosition(m_body));
}

ArnMatrix* GeneralBody::getRotationMatrix( ArnMatrix* rotMat ) const
{
	// TODO: ODE Matrix row or columnwise not clear
	return OdeMatrixToDx(rotMat, dBodyGetRotation(m_body));
}

ArnQuat* GeneralBody::getQuaternion( ArnQuat* q ) const
{
	// TODO: ODE Quaternion order not clear
	return ArnQuatAssign_ScalarLast(q, dBodyGetQuaternion(m_body));
}

ArnVec3* GeneralBody::getLinearVel( ArnVec3* linVel ) const
{
	return ArnVec3Assign(linVel, dBodyGetLinearVel(m_body));
}

ArnVec3* GeneralBody::getAngularVel( ArnVec3* angVel ) const
{
	return ArnVec3Assign(angVel, dBodyGetAngularVel(m_body));
}

void GeneralBody::setBoundingBoxSize( const ArnVec3& size )
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

void GeneralBody::setMassDistributionSize( const ArnVec3& size )
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

void GeneralBody::configureOdeContext( const OdeSpaceContext* osc )
{
	assert(m_osc == 0);
	assert(m_body == 0);
	assert(m_geom == 0);
	assert(m_abbt != ABBT_UNKNOWN);
	assert(m_amdt != AMDT_UNKNOWN);
	m_osc = osc;

	// Body and mass distribution setting
	m_body = dBodyCreate(m_osc->world);
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
}

void GeneralBody::notify()
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

void GeneralBody::setBodyData( void* data )
{
	dBodySetData(m_body, data);
}

void GeneralBody::setGeomData( void* data )
{
	dGeomSetData(m_geom, data);
}

void* GeneralBody::getGeomData() const
{
	return dGeomGetData(m_geom);
}