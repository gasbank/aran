#include "AranPCH.h"
#include "ArnXformable.h"
#include "ArnIpo.h"

ArnXformable::ArnXformable(NODE_DATA_TYPE ndt)
: ArnNode(ndt), m_d3dxAnimCtrl(0)
{
	m_animLocalXform = DX_CONSTS::D3DXMAT_IDENTITY;
	m_localXform = DX_CONSTS::D3DXMAT_IDENTITY;
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
	m_d3dxAnimCtrl->SetTrackSpeed(0, 1.0f/10);
	m_d3dxAnimCtrl->SetTrackWeight(0, 1.0f);
	m_d3dxAnimCtrl->SetTrackEnable(0, TRUE);
	V_VERIFY(m_d3dxAnimCtrl->RegisterAnimationOutput(getIpoName().c_str(), &m_animLocalXform, 0, 0, 0));
}

void ArnXformable::update( double fTime, float fElapsedTime )
{
	if (m_d3dxAnimCtrl)
		m_d3dxAnimCtrl->AdvanceTime(fElapsedTime, 0);
}