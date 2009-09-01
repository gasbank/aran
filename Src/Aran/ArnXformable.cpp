#include "AranPCH.h"
#include "ArnXformable.h"
#include "ArnIpo.h"
#include "ArnMath.h"
#include "Animation.h"
#include "ArnAnimationController.h"
#include "ArnConsts.h"
#include "ArnAction.h"

ArnXformable::ArnXformable(NODE_DATA_TYPE ndt)
: ArnNode(ndt)
, m_bVisible(true)
, m_localXform(ArnConsts::ARNMAT_IDENTITY)
, m_localXform_Scale(ArnConsts::ARNVEC3_ONE)
, m_localXform_Rot(ArnConsts::ARNQUAT_IDENTITY)
, m_localXform_Trans(ArnConsts::ARNVEC3_ZERO)
, m_bLocalXformDirty(false)
, m_animLocalXform(ArnConsts::ARNMAT_IDENTITY)
, m_animLocalXform_Scale(ArnConsts::ARNVEC3_ONE)
, m_animLocalXform_Rot(ArnConsts::ARNQUAT_IDENTITY)
, m_animLocalXform_Trans(ArnConsts::ARNVEC3_ZERO)
, m_bAnimLocalXformDirty(false)
, m_animCtrl(0)
, m_bDoAnim(false)
, m_bAnimSeqEnded(false)
, m_ipo(0)
{
}

ArnXformable::~ArnXformable(void)
{
	//SAFE_RELEASE(m_d3dxAnimCtrl);
	delete m_animCtrl;
}

void
ArnXformable::setIpo( const std::string& ipoName )
{
	if (ipoName.length())
	{
		ArnNode* ipo = getSceneRoot()->getNodeByName(getIpoName());
		if (ipo && ipo->getType() == NDT_RT_IPO)
			setIpo(reinterpret_cast<ArnIpo*>(ipo));
		else
			throw MyError(MEE_RTTI_INCONSISTENCY);
	}
	else
	{
		setIpo(0);
	}
}

void
ArnXformable::advanceTime(float fTime)
{
	m_animCtrl->AdvanceTime( fTime );
}

void
ArnXformable::setIpo( ArnIpo* val )
{
	m_ipo = val;
	/*
	if (m_ipo)
	{
		DWORD cn = m_ipo->getCurveNames();

		ArnVec3 vTrans = getLocalXform_Trans(), vScale = getLocalXform_Scale();
		ArnQuat qRot = getLocalXform_Rot();
		ArnVec3 vRot = ArnQuatToEuler(&qRot);

		if (cn & CN_LocX) vTrans.x = 0;
		if (cn & CN_LocY) vTrans.y = 0;
		if (cn & CN_LocZ) vTrans.z = 0;
		if (cn & CN_ScaleX) vScale.x = 1.0f;
		if (cn & CN_ScaleY) vScale.y = 1.0f;
		if (cn & CN_ScaleZ) vScale.z = 1.0f;
		if (cn & CN_RotX) vRot.x = 0;
		if (cn & CN_RotY) vRot.y = 0;
		if (cn & CN_RotZ) vRot.z = 0;

		qRot = ArnEulerToQuat(&vRot);

		ArnMatrixTransformation(&m_localXformIpo, 0, 0, &vScale, 0, &qRot, &vTrans);
	}
	*/
}

void
ArnXformable::configureAnimCtrl()
{
	ArnIpo* ipo = getIpo();
	if (!ipo)
	{
		//fprintf(stderr, " ** [Node: %s] Animation controller cannot be configured since there is no IPO associated.\n", getName());
		return;
	}
	assert(!m_animCtrl);
	V_VERIFY( ArnCreateAnimationController(
		1, /* MaxNumMatrices */
		1, /* MaxNumAnimationSets */
		1, /* MaxNumTracks */
		10, /* MaxNumEvents */
		&m_animCtrl
		) );

	ArnIpo* globalIpoNode = static_cast<ArnIpo*>(getSceneRoot()->getNodeByName("Global IPOs Node"));
	if (globalIpoNode)
	{
		// Older way (does not use XML file.)
		assert(globalIpoNode->getType() == NDT_RT_IPO);
		ArnIpo* animSet = globalIpoNode->getD3DXAnimSet();
		V_VERIFY(m_animCtrl->RegisterIpo(animSet));
		m_animCtrl->SetTrackAnimationSet(0, 0);
	}
	else
	{
		// Newer way
		m_animCtrl->RegisterIpo(ipo);
		// Need to create simple object-ipo mapping (ArnAction) instance
		// since this is a single object with an animation.
		ArnAction* action = ArnAction::createFrom(this, ipo);
		m_animCtrl->RegisterAnimationSet(action);
		m_animCtrl->SetTrackAnimationSet(0, 0);
	}
	m_animCtrl->SetTrackPosition(0, 0.0f);
	m_animCtrl->SetTrackSpeed(0, 1.0f);
	m_animCtrl->SetTrackWeight(0, 1.0f);
	m_animCtrl->SetTrackEnable(0, TRUE);
	V_VERIFY(m_animCtrl->RegisterAnimationOutput(getIpoName().c_str(), &m_animLocalXform, &m_animLocalXform_Scale, &m_animLocalXform_Rot, &m_animLocalXform_Trans));
	m_animCtrl->AdvanceTime(0); // Initialize animation matrix outputs

	setDoAnim(true); // Start Animation right now.
}

void
ArnXformable::update( double fTime, float fElapsedTime )
{
	if (m_bDoAnim && m_animCtrl)
	{
		m_animCtrl->AdvanceTime(fElapsedTime, 0);
	}

	ArnNode::update(fTime, fElapsedTime);
}

// Explicitly set m_localXform, and therefore, Scale, rotation(quat) and translation values are recalculated.
void
ArnXformable::setLocalXform( const ArnMatrix& localXform )
{
	m_localXform = localXform;
	ArnMatrixDecompose(&m_localXform_Scale, &m_localXform_Rot, &m_localXform_Trans, &m_localXform);
	m_bLocalXformDirty = false;
}

// Recalculate local xform from scale, rotation(quat) and translation.
void
ArnXformable::recalcLocalXform()
{
	m_localXform_Rot.normalize();
	ArnMatrixTransformation(&m_localXform, 0, 0, &m_localXform_Scale, 0, &m_localXform_Rot, &m_localXform_Trans);
	m_bLocalXformDirty = false;
}

void
ArnXformable::recalcAnimLocalXform()
{
	m_animLocalXform_Rot.normalize();
	ArnMatrixTransformation(&m_animLocalXform, 0, 0, &m_animLocalXform_Scale, 0, &m_animLocalXform_Rot, &m_animLocalXform_Trans);
	m_bAnimLocalXformDirty = false;
}

const ArnMatrix&
ArnXformable::getAutoLocalXform() const
{
	if (m_ipo)
	{
		assert(m_bAnimLocalXformDirty == false);
		return m_animLocalXform;
	}
	else
	{
		assert(m_bLocalXformDirty == false);
		return m_localXform;
	}
}

double
ArnXformable::getAnimCtrlTime() const
{
	if ( m_animCtrl )
		return m_animCtrl->GetTime();
	else return -1.0;
}

void
ArnXformable::setAnimCtrlTime( double dTime )
{
	if ( m_animCtrl )
	{
		m_animCtrl->SetTrackPosition( 0, dTime );
		//m_d3dxAnimCtrl->AdvanceTime( dTime, 0 );
	}
	else
	{
		OutputDebugStringA( "Animation controller is not available on the ArnXformable. setAnimCtrlTime ignored\n" );
	}
}

void
ArnXformable::setDoAnim( bool bDoAnim )
{
	m_bDoAnim = bDoAnim;
}

void
ArnXformable::printXformData() const
{
	ArnVec3 eul = ArnQuatToEuler(&m_localXform_Rot);
	printf("  Node name: %s\n", getName());
	printf("    Local scaling        ( %.3f, %.3f, %.3f)\n", m_localXform_Scale.x, m_localXform_Scale.y, m_localXform_Scale.z);
	printf("    Local Rotation Quat  (w%.3f, %.3f, %.3f, %.3f)\n", m_localXform_Rot.w, m_localXform_Rot.x, m_localXform_Rot.y, m_localXform_Rot.z);
	printf("    Local Rotation Euler ( %.3f, %.3f, %.3f)\n", eul.x * 180 / M_PI, eul.y * 180 / M_PI, eul.z * 180 / M_PI);
	printf("    Local translation    ( %.3f, %.3f, %.3f)\n", m_localXform_Trans.x, m_localXform_Trans.y, m_localXform_Trans.z);
}

void
ArnXformable::configureIpo()
{
	if (!m_ipo)
		setIpo(getIpoName());
	configureAnimCtrl();
}

void ArnXformable::addJointData( const ArnJointData& data )
{
	m_jointData.push_back(data);
}

ArnMatrix
ArnXformable::computeWorldXform() const
{
	if	(getParent()
		&&	( getParent()->getType() == NDT_RT_MESH
		||	  getParent()->getType() == NDT_RT_CAMERA
		||	  getParent()->getType() == NDT_RT_LIGHT
		||	  getParent()->getType() == NDT_RT_SKELETON
		||	  getParent()->getType() == NDT_RT_BONE
			)
		)
	{
		return ArnMatrixMultiply(static_cast<ArnXformable*>(getParent())->computeWorldXform(), getAutoLocalXform());
	}
	else
	{
		return getAutoLocalXform();
	}
}
