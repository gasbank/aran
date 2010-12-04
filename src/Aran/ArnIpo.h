#pragma once
#include "ArnNode.h"

enum ARNPLAYBACK_TYPE
{
	ARNPLAY_LOOP          = 0,
	ARNPLAY_ONCE          = 1,
	ARNPLAY_PINGPONG      = 2,

	ARNPLAY_FORCE_DWORD   = 0x7fffffff /* force 32-bit size enum */
};

struct ARNKEY_CALLBACK
{
	float Time;
	void* pCallbackData;
};

struct NodeBase;
struct NodeIpo1;
struct NodeIpo2;

class ArnIpo;

class ARAN_API ArnIpo : public ArnNode
{
public:
											~ArnIpo(void);
	static ArnIpo*							createFrom(const NodeBase* nodeBase);
	static ArnIpo*							createFrom(const TiXmlElement* elm, const char* binaryChunkBasePtr);
	static ArnIpo*							create(const char* name, unsigned int curveCount);
	unsigned int							getIpoCount() const { return m_ipoCount; }
	unsigned int							getCurveCount() const { return m_curveCount; }
	const CurveData&						getCurveData(unsigned int idx) const { return m_curves[idx]; }
	const std::vector<CurveData>&			getCurves() const { return m_curves; }
	DWORD									getCurveNames() const { return m_curveNames; }
	int										getEndKeyframe() const { return m_endKeyframe; }
	ArnIpo*									getD3DXAnimSet() const { return m_d3dxAnimSet; }
	static CurveName						CurveNameStrToEnum(const char* name);
	void									setEndKeyframe(int endKeyframe) { m_endKeyframe = endKeyframe; }
	ARNPLAYBACK_TYPE						getPlaybackType() const { return m_playbackType; }

	// ****************** Aran library compartment of LPD3DXKEYFRAMEDANIMATIONSET START ******************
	void									Release() { /* For compatiability issue. Do nothing here. */ }
	HRESULT									RegisterAnimationSRTKeys(const char* pName,							// Animation name
																	UINT NumScaleKeys,							// Number of scale keys
																	UINT NumRotationKeys,						// Number of rotation keys
																	UINT NumTranslationKeys,					// Number of translation keys
																	CONST ARNKEY_VECTOR3* pScaleKeys,			// Array of scale keys
																	CONST ARNKEY_QUATERNION* pRotationKeys,		// Array of rotation keys
																	CONST ARNKEY_VECTOR3* pTranslationKeys,		// Array of translation keys
																	DWORD *pAnimationIndex						// Returns the animation index
																	);
	double									GetPeriodicPosition(double time) const
	{
		UNREFERENCED_PARAMETER(time);
		ARN_THROW_NOT_IMPLEMENTED_ERROR
	}
	HRESULT									GetSRT(double time, UINT unused, ArnVec3*, ArnQuat*, ArnVec3*)
	{
		UNREFERENCED_PARAMETER(time);
		UNREFERENCED_PARAMETER(unused);
		ARN_THROW_NOT_IMPLEMENTED_ERROR
	}
	// ******************  Aran library compartment of LPD3DXKEYFRAMEDANIMATIONSET END  ******************

	// *** INTERNAL USE ONLY START ***
	virtual void							interconnect(ArnNode* sceneRoot);
	// *** INTERNAL USE ONLY END ***
private:
											ArnIpo(void);
	void									buildFrom(const NodeIpo1* ni);
	void									buildFrom(const NodeIpo2* ni);

	DWORD									m_curveNames;
	unsigned int							m_ipoCount;
	unsigned int							m_curveCount;
	std::vector<CurveData>					m_curves;
	int										m_endKeyframe;
	ArnIpo*									m_d3dxAnimSet;
	ARNPLAYBACK_TYPE						m_playbackType;
};

//////////////////////////////////////////////////////////////////////////

HRESULT ARAN_API ArnCreateKeyframedAnimationSet( const char* pName,
									   double TicksPerSecond,
									   ARNPLAYBACK_TYPE Playback,
									   UINT NumAnimations,
									   UINT NumCallbackKeys,
									   CONST ARNKEY_CALLBACK* pCallbackKeys,
									   ArnIpo** ppAnimationSet );
