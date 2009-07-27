#include "AranPCH.h"
#include "ArnAnimationController.h"
#include "ArnIpo.h"
#include "Animation.h"
#include "ArnMath.h"
#include "ArnAction.h"
#include "ArnBone.h"
#include "ArnConsts.h"

ArnAnimationController::ArnAnimationController()
: m_dTime(0)
, m_outputs()
, m_ipos()
, m_actions()
, m_tracks()
, m_events()
, m_outputCount(0)
, m_actionCount(0)
, m_eventCount(0)
, m_curActionIdx(0)
{
}

ArnAnimationController::~ArnAnimationController()
{
	/*
	foreach(ArnIpo* aas, m_animSets)
	{
		delete aas;
	}
	*/
}

ArnAnimationController*
ArnAnimationController::create( UINT MaxNumAnimationOutputs, UINT MaxNumAnimationSets, UINT MaxNumTracks, UINT MaxNumEvents )
{
	ArnAnimationController* ret = new ArnAnimationController();
	ret->m_outputs.resize(MaxNumAnimationOutputs);
	ret->m_actions.resize(MaxNumAnimationSets);
	ret->m_tracks.resize(MaxNumTracks);
	ret->m_events.resize(MaxNumEvents);
	return ret;
}

unsigned int
ArnAnimationController::RegisterAnimationSet( ArnAction* action )
{
	if (m_actionCount < m_actions.size())
	{
		m_actions[m_actionCount] = action;
		++m_actionCount;
		return m_actionCount-1;
	}
	else
	{
		fprintf(stderr, " ** Animation set number exceeds on this controller.\n");
		return 0;
	}
}

HRESULT
ArnAnimationController::RegisterIpo( ArnIpo* ipo )
{
	m_ipos.push_back(ipo);
	return S_OK;
}

void ArnAnimationController::SetTrackAnimationSet( UINT trackNum, UINT actionIdx )
{
	if (trackNum < m_tracks.size())
	{
		assert(actionIdx < m_actionCount);
		m_tracks[trackNum].ActionIdx = actionIdx;
	}
	else
	{
		fprintf(stderr, " ** Track index out of range.\n");
	}
}

void
ArnAnimationController::SetTrackPosition( UINT trackNum, double trackPos )
{
	if (trackNum < m_tracks.size())
		m_tracks[trackNum].Position = trackPos;
	else
		ARN_THROW_UNEXPECTED_CASE_ERROR
}

void
ArnAnimationController::SetTrackSpeed( UINT trackNum, float trackSpeed )
{
	if (trackNum < m_tracks.size())
		m_tracks[trackNum].Speed = trackSpeed;
	else
		ARN_THROW_UNEXPECTED_CASE_ERROR
}

void
ArnAnimationController::SetTrackWeight( UINT trackNum, float trackWeight )
{
	if (trackNum < m_tracks.size())
		m_tracks[trackNum].Weight = trackWeight;
	else
		ARN_THROW_UNEXPECTED_CASE_ERROR
}

void
ArnAnimationController::SetTrackEnable( UINT trackNum, bool bEnable )
{
	if (trackNum < m_tracks.size())
		m_tracks[trackNum].Enable = bEnable;
	else
		ARN_THROW_UNEXPECTED_CASE_ERROR
}

HRESULT
ArnAnimationController::RegisterAnimationOutput( const char* pName, ArnMatrix* pMatrix, ArnVec3* pScale, ArnQuat* pRotation, ArnVec3* pTranslation )
{
	assert(pScale || pRotation || pTranslation == 0);
	if (m_outputCount < m_outputs.size())
	{
		m_outputs[m_outputCount].name	= pName;
		m_outputs[m_outputCount].mat	= pMatrix;
		m_outputs[m_outputCount].scale	= pScale;
		m_outputs[m_outputCount].quat	= pRotation;
		m_outputs[m_outputCount].trans	= pTranslation;
		++m_outputCount;
		return S_OK;
	}
	else
	{
		fprintf(stderr, " ** Animation set number exceeds on this controller.\n");
		return E_FAIL;
	}
}

void
ArnAnimationController::Release()
{
	// TODO: Is there anything to release?
}

void
ArnAnimationController::AdvanceTime( double dTime, void* callBack /*= 0*/ )
{
	if (dTime == 0)
		return;
	m_dTime += dTime;
	float totalWeight = 0;
	for (unsigned int i = 0; i < m_tracks.size(); ++i)
	{
		if (m_tracks[i].Enable)
			totalWeight += m_tracks[i].Weight;
	}
	for (unsigned int i = 0; i < m_tracks.size(); ++i)
	{
		ARNTRACK_DESC& track = m_tracks[i];
		if (!track.Enable)
			continue;
		//printf("%.3f\n", (float)(m_dTime - track.Position));
		assert(track.ActionIdx < m_actionCount);
		//float normalizedWeight = track.Weight / totalWeight;
		typedef std::pair<ArnNode*, ArnIpo*> ObjIpoPair;
		foreach (ObjIpoPair p, m_actions[track.ActionIdx]->getObjectIpoMap())
		{
			ArnIpo* ipo = p.second;
			assert(ipo);
			ArnVec3 transKey = CreateArnVec3(0,0,0);
			ArnVec3 scaleKey = CreateArnVec3(1,1,1);
			ArnVec3 rotKey = CreateArnVec3(0,0,0);
			ArnQuat quat = CreateArnQuat(0, 0, 0, 0);
			foreach (const CurveData& cd, ipo->getCurves())
			{
				float val = Animation::EvalCurveInterp(&cd, (float)(m_dTime - track.Position)*FPS);
				switch (cd.name)
				{
				case CN_LocX:	transKey.x = val;				break;
				case CN_LocY:	transKey.y = val;				break;
				case CN_LocZ:	transKey.z = val;				break;
				case CN_ScaleX: scaleKey.x = val;				break;
				case CN_ScaleY: scaleKey.y = val;				break;
				case CN_ScaleZ: scaleKey.z = val;				break;
				case CN_RotX:	rotKey.x = ArnToRadian(val);	break;
				case CN_RotY:	rotKey.y = ArnToRadian(val);	break;
				case CN_RotZ:	rotKey.z = ArnToRadian(val);	break;
				case CN_QuatW:	quat.w = val;					break;
				case CN_QuatX:	quat.x = val;					break;
				case CN_QuatY:	quat.y = val;					break;
				case CN_QuatZ:	quat.z = val;					break;
				default:		throw MyError(MEE_UNSUPPORTED_CURVENAME);
				}
			}
			if (quat != ArnQuat(0, 0, 0, 0))
			{
				assert((rotKey.x || rotKey.y || rotKey.z) == 0);
			}
			else
			{
				quat = ArnEulerToQuat(&rotKey);
			}
			// TODO: Quaternion should be normalized before a making composed transformation matrix.
			//       Any way to make this routine faster?
			quat /= quat.getLength();
			//ArnMatrix ipoResultMat;
			//ArnMatrixTransformation(&ipoResultMat, 0, 0, &scaleKey, 0, &quat, &transKey);
			static_cast<ArnBone*>(p.first)->setAnimLocalXform_Rot(quat);
			assert(p.first->getType() == NDT_RT_BONE);

			if ((m_dTime > (track.Position + (float)ipo->getEndKeyframe()/FPS)) && ipo->getPlaybackType() == ARNPLAY_LOOP)
			{
				// Reset the time if the end of animation reached.
				track.Position += (float)ipo->getEndKeyframe()/FPS;
			}
		}
	}
}

HRESULT
ArnAnimationController::GetTrackDesc( UINT trackNum, ARNTRACK_DESC* pDesc )
{
	memcpy(pDesc, &m_tracks[trackNum], sizeof(ARNTRACK_DESC));
	return S_OK;
}

HRESULT
ArnAnimationController::GetAnimationSet( UINT Index, ArnIpo** ppAnimationSet )
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

void
ArnAnimationController::ResetTime()
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

int
ArnAnimationController::GetNumAnimationSets() const
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

void
ArnAnimationController::UnregisterAnimationSet( ArnIpo* animSet )
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

void ArnAnimationController::update( double fTime, float fElapsedTime )
{
	AdvanceTime(fElapsedTime, 0);
}

void ArnAnimationController::SetActionToNext()
{
	++m_curActionIdx;
	if (m_curActionIdx >= m_actionCount)
		m_curActionIdx = 0;
}

//////////////////////////////////////////////////////////////////////////

HRESULT ArnCreateAnimationController( UINT MaxNumMatrices, UINT MaxNumAnimationSets, UINT MaxNumTracks, UINT MaxNumEvents, ArnAnimationController** ppAnimController )
{
	*ppAnimController = ArnAnimationController::create(MaxNumMatrices, MaxNumAnimationSets, MaxNumTracks, MaxNumEvents);
	return S_OK;
}

HRESULT ArnFrameRegisterNamedMatrices( const ArnFrame* frameRoot, ArnAnimationController* animCtrl )
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

