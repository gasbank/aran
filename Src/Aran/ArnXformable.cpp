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
, m_ipo(0)
, m_d3dxAnimCtrl(0)
, m_bDoAnim(false)
, m_bAnimSeqEnded(false)
, m_bLocalXformDirty(true)
, m_bAnimLocalXformDirty(true)
, m_localXform_Scale(ArnConsts::ARNVEC3_ONE)
, m_localXform_Trans(ArnConsts::ARNVEC3_ZERO)
, m_localXform_Rot(ArnConsts::ARNQUAT_IDENTITY)
, m_localXform(ArnConsts::ARNMAT_IDENTITY)
, m_animLocalXform_Scale(ArnConsts::ARNVEC3_ONE)
, m_animLocalXform_Trans(ArnConsts::ARNVEC3_ZERO)
, m_animLocalXform_Rot(ArnConsts::ARNQUAT_IDENTITY)
, m_animLocalXform(ArnConsts::ARNMAT_IDENTITY)
, m_localXformIpo(ArnConsts::ARNMAT_IDENTITY)
{
}

ArnXformable::~ArnXformable(void)
{
	//SAFE_RELEASE(m_d3dxAnimCtrl);
	delete m_d3dxAnimCtrl;
}

void
ArnXformable::setIpo( const STRING& ipoName )
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
	m_d3dxAnimCtrl->AdvanceTime( fTime );
}

void
ArnXformable::setIpo( ArnIpo* val )
{
	m_ipo = val;
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
}
ArnMatrix
ArnXformable::getFinalXform()
{
	if (	(getParent()->getType() == NDT_RT_MESH)
		||	(getParent()->getType() == NDT_RT_CAMERA)
		||	(getParent()->getType() == NDT_RT_LIGHT) )
	{
		return ArnMatrixMultiply(static_cast<ArnXformable*>(getParent())->getFinalXform(), getFinalLocalXform());
	}
	else
	{
		return getFinalLocalXform();
	}
}

void
ArnXformable::configureAnimCtrl()
{
	ArnIpo* ipo = getIpo();
	if (!ipo)
	{
		fprintf(stderr, " ** [Node: %s] Animation controller cannot be configured since there is no IPO associated.\n", getName());
		return;
	}
	assert(!m_d3dxAnimCtrl);
	V_VERIFY( ArnCreateAnimationController(
		1, /* MaxNumMatrices */
		1, /* MaxNumAnimationSets */
		1, /* MaxNumTracks */
		10, /* MaxNumEvents */
		&m_d3dxAnimCtrl
		) );

	ArnIpo* globalIpoNode = static_cast<ArnIpo*>(getSceneRoot()->getNodeByName("Global IPOs Node"));
	if (globalIpoNode)
	{
		// Older way (does not use XML file.)
		assert(globalIpoNode->getType() == NDT_RT_IPO);
		ArnIpo* animSet = globalIpoNode->getD3DXAnimSet();
		V_VERIFY(m_d3dxAnimCtrl->RegisterIpo(animSet));
		m_d3dxAnimCtrl->SetTrackAnimationSet(0, 0);
	}
	else
	{
		// Newer way
		m_d3dxAnimCtrl->RegisterIpo(ipo);
		// Need to create simple object-ipo mapping (ArnAction) instance
		// since this is a single object with an animation.
		ArnAction* action = ArnAction::createFrom(this, ipo);
		m_d3dxAnimCtrl->RegisterAnimationSet(action);
		m_d3dxAnimCtrl->SetTrackAnimationSet(0, 0);
	}
	m_d3dxAnimCtrl->SetTrackPosition(0, 0.0f);
	m_d3dxAnimCtrl->SetTrackSpeed(0, 1.0f);
	m_d3dxAnimCtrl->SetTrackWeight(0, 1.0f);
	m_d3dxAnimCtrl->SetTrackEnable(0, TRUE);
	V_VERIFY(m_d3dxAnimCtrl->RegisterAnimationOutput(getIpoName().c_str(), &m_animLocalXform, &m_animLocalXform_Scale, &m_animLocalXform_Rot, &m_animLocalXform_Trans));
	m_d3dxAnimCtrl->AdvanceTime(0); // Initialize animation matrix outputs

	setDoAnim(true); // Start Animation right now.
}

void
ArnXformable::update( double fTime, float fElapsedTime )
{
	if (m_bDoAnim && m_d3dxAnimCtrl)
	{
		//m_d3dxAnimCtrl->AdvanceTime(0.005, 0);
		m_d3dxAnimCtrl->AdvanceTime(fElapsedTime, 0);

		/*
		ARNTRACK_DESC trackDesc;
		m_d3dxAnimCtrl->GetTrackDesc( 0, &trackDesc );
		if (m_ipo && ( (float)m_ipo->getEndKeyframe() / FPS < (float)trackDesc.Position ))
		{
			// Current Ipo ended. Stop the animation
			setDoAnim(false);
			setAnimSeqEnded(true);

			char debugMsg[128];
			//StringCchPrintf(debugMsg, 128, _T("m_ipo End Keyframe = %d, trackDescPosition = %f\n"), m_ipo->getEndKeyframe(), (float)trackDesc.Position);
			sprintf(debugMsg, "m_ipo End Keyframe = %d, trackDescPosition = %f\n", m_ipo->getEndKeyframe(), (float)trackDesc.Position);
			OutputDebugStringA("INFO: Animation stopped since all keyframes passed\n");
			OutputDebugStringA(debugMsg);
		}
		*/
	}

	ArnNode::update(fTime, fElapsedTime);
}

// Explicitly set m_localXform, and therefore, Scale, rotation(quat) and translation values are recalculated.
void
ArnXformable::setLocalXform( const ArnMatrix& localXform )
{
	m_localXform = localXform;
	m_localXformIpo = m_localXform;
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
ArnXformable::getFinalLocalXform() const
{
	// TODO: We need urgent transformation matrix cleanup -_-;

	//m_finalLocalXform = m_animLocalXform * m_localXformIpo;
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
	if ( m_d3dxAnimCtrl )
		return m_d3dxAnimCtrl->GetTime();
	else return -1.0;
}

void
ArnXformable::setAnimCtrlTime( double dTime )
{
	if ( m_d3dxAnimCtrl )
	{
		m_d3dxAnimCtrl->SetTrackPosition( 0, dTime );
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
	printf("Node name: %s\n", getName());
	printf("Local translation (%.3f, %.3f, %.3f)\n", m_localXform_Trans.x, m_localXform_Trans.y, m_localXform_Trans.z);
	ArnVec3 eul = ArnQuatToEuler(&m_localXform_Rot);
	printf("Rotation Quat (w%.3f, %.3f, %.3f, %.3f)\n", m_localXform_Rot.w, m_localXform_Rot.x, m_localXform_Rot.y, m_localXform_Rot.z);
	printf("Rotation Euler (%.3f, %.3f, %.3f)\n", eul.x * 180 / M_PI, eul.y * 180 / M_PI, eul.z * 180 / M_PI);
}

void
ArnXformable::configureIpo()
{
	if (!m_ipo)
		setIpo(getIpoName());
	configureAnimCtrl();
}

