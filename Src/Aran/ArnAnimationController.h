// 2009 Geoyeob Kim
#pragma once

//----------------------------------------------------------------------------
// D3DXPRIORITY_TYPE:
// ------------------
// This enum defines the type of priority group that a track can be assigned to.
//----------------------------------------------------------------------------
enum ARNPRIORITY_TYPE
{
	ARNPRIORITY_LOW         = 0,           // This track should be blended with all low priority tracks before mixed with the high priority result
	ARNPRIORITY_HIGH        = 1,           // This track should be blended with all high priority tracks before mixed with the low priority result

	ARNPRIORITY_FORCE_DWORD = 0x7fffffff  /* force 32-bit size enum */
};

class ArnIpo;
class ArnAction;

//----------------------------------------------------------------------------
// D3DXTRACK_DESC:
// ---------------
// This structure describes the mixing information of an animation track.
// The mixing information consists of the current position, speed, and blending
// weight for the track.  The Flags field also specifies whether the track is
// low or high priority.  Tracks with the same priority are blended together
// and then the two resulting values are blended using the priority blend factor.
// A track also has an animation set (stored separately) associated with it.
//----------------------------------------------------------------------------
struct ARNTRACK_DESC
{
	ARNPRIORITY_TYPE	Priority;
	float				Weight;
	float				Speed;
	double				Position;
	BOOL				Enable;
	unsigned int		ActionIdx;
};

class ARNANIMATIONOUTPUT
{
public:
										ARNANIMATIONOUTPUT() : name(), mat(), scale(), quat(), trans() {}
	std::string								name;
	ArnMatrix*							mat;
	ArnVec3*							scale;
	ArnQuat*							quat;
	ArnVec3*							trans;
};

class ArnAnimationController;

// Aran library compartment of LPD3DXANIMATIONCONTROLLER
class ARAN_API ArnAnimationController
{
public:
													~ArnAnimationController();
	static ArnAnimationController*					create(UINT MaxNumAnimationOutputs, UINT MaxNumAnimationSets, UINT MaxNumTracks, UINT MaxNumEvents);
	void											Release();
	void											AdvanceTime(double  dTime, void* callBack = 0);
	HRESULT											RegisterIpo(ArnIpo* ipo);
	unsigned int									RegisterAnimationSet(ArnAction* action);
	void											SetAction(unsigned int idx) { m_curActionIdx = idx; }
	void											SetActionToNext();
	void											SetTrackAnimationSet(UINT trackNum, UINT actionIdx);
	void											SetTrackPosition(UINT trackNum, double trackPos);
	void											SetTrackSpeed(UINT trackNum, float trackSpeed);
	void											SetTrackWeight(UINT trackNum, float trackWeight);
	void											SetTrackEnable(UINT trackNum, bool bEnable);
	HRESULT											RegisterAnimationOutput(const char* pName, ArnMatrix* pMatrix, ArnVec3* pScale, ArnQuat* pRotation, ArnVec3* pTranslation);
	HRESULT											GetTrackDesc(UINT trackNum, ARNTRACK_DESC* pDesc);
	double											GetTime() const { return m_dTime; }
	HRESULT											GetAnimationSet(UINT Index, ArnIpo** ppAnimationSet);
	void											ResetTime();
	int												GetNumAnimationSets() const;
	void											UnregisterAnimationSet(ArnIpo* animSet);
	void											update(double fTime, float fElapsedTime);
	unsigned int									getTrackCount() const { return m_tracks.size(); }

private:
													ArnAnimationController();
	double											m_dTime;
	std::vector<ARNANIMATIONOUTPUT>					m_outputs;
	std::vector<ArnIpo*>							m_ipos;
	std::vector<ArnAction*>							m_actions;
	std::vector<ARNTRACK_DESC>						m_tracks;
	std::vector<int>								m_events; // TODO: dummy animation events
	unsigned int									m_outputCount;
	unsigned int									m_actionCount;
	unsigned int									m_eventCount;
	unsigned int									m_curActionIdx;
};

//////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// D3DXCreateAnimationController:
// ------------------------------
// This function creates an animation controller object.
//
// Parameters:
//  MaxNumMatrices
//      Maximum number of matrices that can be animated
//  MaxNumAnimationSets
//      Maximum number of animation sets that can be played
//  MaxNumTracks
//      Maximum number of animation sets that can be blended
//  MaxNumEvents
//      Maximum number of outstanding events that can be scheduled at any given time
//  ppAnimController
//      Returns the animation controller interface
//
//-----------------------------------------------------------------------------
HRESULT ArnCreateAnimationController(UINT MaxNumMatrices,
									 UINT MaxNumAnimationSets,
									 UINT MaxNumTracks,
									 UINT MaxNumEvents,
									 ArnAnimationController** ppAnimController);

HRESULT ArnFrameRegisterNamedMatrices(const ArnFrame* frameRoot, ArnAnimationController* animCtrl);
