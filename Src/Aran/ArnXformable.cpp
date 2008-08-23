#include "AranPCH.h"
#include "ArnXformable.h"
#include "ArnIpo.h"
#include "ArnMath.h"

#define FPS (60)

ArnXformable::ArnXformable(NODE_DATA_TYPE ndt)
: ArnNode(ndt), m_d3dxAnimCtrl(0), m_bDoAnim(false), m_bAnimSeqEnded(false)
{
	m_animLocalXform	= DX_CONSTS::D3DXMAT_IDENTITY;
	m_localXform		= DX_CONSTS::D3DXMAT_IDENTITY;
	m_localXformIpo		= DX_CONSTS::D3DXMAT_IDENTITY;
}

ArnXformable::~ArnXformable(void)
{
	SAFE_RELEASE(m_d3dxAnimCtrl);
}

void ArnXformable::setIpo( const STRING& ipoName )
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

void ArnXformable::setIpo( ArnIpo* val )
{
	m_ipo = val;
	if (m_ipo)
	{
		DWORD cn = m_ipo->getCurveNames();

		D3DXVECTOR3 vTrans = getLocalXform_Trans(), vScale = getLocalXform_Scale();
		D3DXQUATERNION qRot = getLocalXform_Rot();
		D3DXVECTOR3 vRot = ArnMath::QuatToEuler(&qRot);

		if (cn & CN_LocX) vTrans.x = 0;
		if (cn & CN_LocY) vTrans.y = 0;
		if (cn & CN_LocZ) vTrans.z = 0;
		if (cn & CN_ScaleX) vScale.x = 1.0f;
		if (cn & CN_ScaleY) vScale.y = 1.0f;
		if (cn & CN_ScaleZ) vScale.z = 1.0f;
		if (cn & CN_RotX) vRot.x = 0;
		if (cn & CN_RotY) vRot.y = 0;
		if (cn & CN_RotZ) vRot.z = 0;

		qRot = ArnMath::EulerToQuat(&vRot);

		D3DXMatrixTransformation(&m_localXformIpo, 0, 0, &vScale, 0, &qRot, &vTrans);
	}
}
D3DXMATRIX ArnXformable::getFinalXform()
{
	if (	(getParent()->getType() == NDT_RT_MESH)
		||	(getParent()->getType() == NDT_RT_CAMERA)
		||	(getParent()->getType() == NDT_RT_LIGHT) )
	{
		return static_cast<ArnXformable*>(getParent())->getFinalXform() * getFinalLocalXform();
	}
	else
	{
		return getFinalLocalXform();
	}
}

void ArnXformable::configureAnimCtrl()
{
	if (!getIpo())
		return;

	V_VERIFY( D3DXCreateAnimationController(
		1, /* MaxNumMatrices */
		1, /* MaxNumAnimationSets */
		1, /* MaxNumTracks */
		10, /* MaxNumEvents */
		&m_d3dxAnimCtrl
		) );

	ArnIpo* globalIpoNode = static_cast<ArnIpo*>(getSceneRoot()->getNodeByName("Global IPOs Node"));
	assert(globalIpoNode->getType() == NDT_RT_IPO);
	LPD3DXANIMATIONSET animSet = globalIpoNode->getD3DXAnimSet();
	V_VERIFY(m_d3dxAnimCtrl->RegisterAnimationSet(animSet));
	m_d3dxAnimCtrl->SetTrackAnimationSet(0, animSet);
	m_d3dxAnimCtrl->SetTrackPosition(0, 0.0f);
	m_d3dxAnimCtrl->SetTrackSpeed(0, 1.0f);
	m_d3dxAnimCtrl->SetTrackWeight(0, 1.0f);
	m_d3dxAnimCtrl->SetTrackEnable(0, TRUE);
	V_VERIFY(m_d3dxAnimCtrl->RegisterAnimationOutput(getIpoName().c_str(), &m_animLocalXform, 0, 0, 0));
	m_d3dxAnimCtrl->AdvanceTime( 0.0001f, 0 );
}

void ArnXformable::update( double fTime, float fElapsedTime )
{
	if (m_bDoAnim && m_d3dxAnimCtrl)
	{
		m_d3dxAnimCtrl->AdvanceTime(fElapsedTime, 0);
		D3DXTRACK_DESC trackDesc;
		m_d3dxAnimCtrl->GetTrackDesc( 0, &trackDesc );
		if (m_ipo && ( (float)m_ipo->getEndKeyframe() / FPS < (float)trackDesc.Position ))
		{
			// Current Ipo ended. Stop the animation
			setDoAnim(false);
			setAnimSeqEnded(true);
			
			TCHAR debugMsg[128];
			StringCchPrintf(debugMsg, 128, _T("m_ipo End Keyframe = %d, trackDescPosition = %f\n"), m_ipo->getEndKeyframe(), (float)trackDesc.Position);
			OutputDebugString(_T("INFO: Animation stopped since all keyframes passed\n"));
			OutputDebugString(debugMsg);
		}
	}
}

void ArnXformable::setLocalXform( const D3DXMATRIX& localXform )
{
	m_localXform = localXform;
	m_localXformIpo = m_localXform;
	D3DXMatrixDecompose(&m_localXform_Scale, &m_localXform_Rot, &m_localXform_Trans, &m_localXform);
}

const D3DXMATRIX& ArnXformable::getFinalLocalXform()
{
	m_finalLocalXform = m_animLocalXform * m_localXformIpo;
	return m_finalLocalXform;
}

double ArnXformable::getAnimCtrlTime() const
{
	if ( m_d3dxAnimCtrl )
		return m_d3dxAnimCtrl->GetTime();
	else return -1.0;
}

void ArnXformable::setAnimCtrlTime( double dTime )
{
	if ( m_d3dxAnimCtrl )
	{
		m_d3dxAnimCtrl->SetTrackPosition( 0, dTime );
		//m_d3dxAnimCtrl->AdvanceTime( dTime, 0 );
	}
	else
	{
		OutputDebugString( _T("Animation controller is not available on the ArnXformable. setAnimCtrlTime ignored\n" ) );
	}
}

void ArnXformable::setDoAnim( bool bDoAnim )
{
	m_bDoAnim = bDoAnim;
}