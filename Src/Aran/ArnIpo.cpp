#include "AranPCH.h"
#include "ArnIpo.h"
#include "Animation.h"
#include "ArnMath.h"
#include "Animation.h"
#include "ArnConsts.h"

ArnIpo::ArnIpo(void)
: ArnNode(NDT_RT_IPO)
, m_curveNames(0)
, m_ipoCount(0)
, m_curveCount(0)
, m_endKeyframe(0)
, m_d3dxAnimSet()
, m_playbackType(ARNPLAY_LOOP)
{
}

ArnIpo::~ArnIpo(void)
{
}

ArnIpo* ArnIpo::createFrom( const NodeBase* nodeBase )
{
	ArnIpo* node = new ArnIpo();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_IPO1:
			node->buildFrom(static_cast<const NodeIpo1*>(nodeBase));
			break;
		case NDT_IPO2:
			node->buildFrom(static_cast<const NodeIpo2*>(nodeBase));
			break;
		default:
			throw MyError(MEE_UNDEFINED_ERROR);
		}
	}
	catch (const MyError& e)
	{
		delete node;
		throw e;
	}
	return node;
}

void ArnIpo::buildFrom( const NodeIpo1* ni )
{
	m_ipoCount = ni->m_ipoCount;
	m_curveCount = 0;
}

void ArnIpo::buildFrom( const NodeIpo2* ni )
{
	setParentName(ni->m_parentName);
	m_ipoCount		= 1;
	m_curveCount	= ni->m_curveCount;
	unsigned int i, j;

	for (i = 0; i < m_curveCount; ++i)
	{
		CurveData cd;
		cd.nameStr = ni->m_curves[i].name;
		cd.name = CurveNameStrToEnum(cd.nameStr.c_str());
		cd.type = ni->m_curves[i].type;
		cd.pointCount = ni->m_curves[i].pointCount;
		for (j = 0; j < cd.pointCount; ++j)
		{
			// TODO
			// Vec[0][0] is smaller than Blender IPO point value by -1.0f
			// Vec[2][0] is larger than Blender IPO point value by +1.0f... Strange...
			const BezTripleData& btd = ni->m_curves[i].points[j];
			if ( (float)((int)btd.vec[1][0]) - btd.vec[1][0] > 0.01f )
			{
				#ifdef WIN32
                                OutputDebugStringA("WARN: Bezier key points X value should be integer\n");
				DebugBreak();
				#else
				std::cerr << _T("WARN: Bezier key points X value should be integer\n") << std::endl;
				#endif
			}
			if ( m_endKeyframe < (int)btd.vec[1][0] ) m_endKeyframe = (int)btd.vec[1][0];

			cd.points.push_back(btd);
		}
		m_curves.push_back(cd);

		m_curveNames |= (DWORD)cd.name;
	}
}

CurveName ArnIpo::CurveNameStrToEnum( const char* name )
{
#define CNCONVERT(x) if(!strcmp(name, #x)) return CN_##x;
	CNCONVERT(LocX);
	CNCONVERT(LocY);
	CNCONVERT(LocZ);
	CNCONVERT(RotX);
	CNCONVERT(RotY);
	CNCONVERT(RotZ);
	CNCONVERT(ScaleX);
	CNCONVERT(ScaleY);
	CNCONVERT(ScaleZ);
	CNCONVERT(QuatW)
	CNCONVERT(QuatX)
	CNCONVERT(QuatY)
	CNCONVERT(QuatZ)
#undef CNCONVERT
	return CN_UNKNOWN;
}

void ArnIpo::interconnect( ArnNode* sceneRoot )
{
	unsigned int i, j;
	ArnNode* ipoNode = getParent();
	ArnIpo* ipoContainer = dynamic_cast<ArnIpo*>(ipoNode);

	if (m_curveCount == 0) // If this is IPO container node ...
	{
		if (getNodeCount()) // ... and there are some IPOs there
		{
			ArnCreateKeyframedAnimationSet(
				getName(),
				FPS /* ticks per second */,
				ARNPLAY_ONCE,
				getIpoCount() /* num Animations seq */,
				0,
				0,
				&m_d3dxAnimSet
				);
		}
	}
	else if (ipoContainer)
	{
		ArnIpo* d3dxAnimSet = ipoContainer->getD3DXAnimSet();
		assert(d3dxAnimSet);
		assert( m_endKeyframe );

		//const unsigned int keyCount = 151;
		//const unsigned int sampleCount = keyCount * FPS;
		const unsigned int sampleCount = m_endKeyframe + 1; //(unsigned int)((m_endKeyframe+1.0f) * FPS);
		std::vector<ARNKEY_VECTOR3> scaleKeys(sampleCount);
		std::vector<ARNKEY_VECTOR3> transKeys(sampleCount);
		std::vector<ARNKEY_QUATERNION> rotKeys(sampleCount);
		for (i = 0; i < sampleCount; ++i)
		{
			//float keyTime = (float)i/FPS;
			rotKeys[i].Time = (float)i;
			rotKeys[i].Value = ArnConsts::ARNQUAT_IDENTITY;
			scaleKeys[i].Time = (float)i;
			scaleKeys[i].Value = ArnConsts::ARNVEC3_ONE;
			transKeys[i].Time = (float)i;
			transKeys[i].Value = ArnConsts::ARNVEC3_ZERO;

			float eulX = 0.0f, eulY = 0.0f, eulZ = 0.0f; // Euler rotation values

			for (j = 0; j < getCurveCount(); ++j)
			{
				const CurveData& cd = getCurveData(j);
				float val = Animation::EvalCurveInterp(&cd, (float)i);
				switch (cd.name)
				{
				case CN_LocX:	transKeys[i].Value.x = val;		break;
				case CN_LocY:	transKeys[i].Value.y = val;		break;
				case CN_LocZ:	transKeys[i].Value.z = val;		break;
				case CN_ScaleX: scaleKeys[i].Value.x = val;		break;
				case CN_ScaleY: scaleKeys[i].Value.y = val;		break;
				case CN_ScaleZ: scaleKeys[i].Value.z = val;		break;
				case CN_RotX:	eulX = (float)ArnToRadian(val);		break;
				case CN_RotY:	eulY = (float)ArnToRadian(val);		break;
				case CN_RotZ:	eulZ = (float)ArnToRadian(val);		break;

				case CN_QuatW:
				case CN_QuatX:
				case CN_QuatY:
				case CN_QuatZ:
					ARN_THROW_NOT_IMPLEMENTED_ERROR
					break;

				default:		throw MyError(MEE_UNSUPPORTED_CURVENAME);
				}
			}
			ArnQuat quatX, quatY, quatZ;
			ArnVec3 vEulX = CreateArnVec3(eulX, 0, 0);
			ArnVec3 vEulY = CreateArnVec3(0, eulY, 0);
			ArnVec3 vEulZ = CreateArnVec3(0, 0, eulZ);
			quatX = ArnEulerToQuat(&vEulX);
			quatY = ArnEulerToQuat(&vEulY);
			quatZ = ArnEulerToQuat(&vEulZ);
			rotKeys[i].Value = quatZ * quatY * quatX;
		}
		DWORD animIdx;
		V_VERIFY(d3dxAnimSet->RegisterAnimationSRTKeys(getName(), sampleCount, sampleCount, sampleCount, &scaleKeys[0], &rotKeys[0], &transKeys[0], &animIdx));
		std::string debugMsg;
		debugMsg = " - ";
		debugMsg += getName();
		debugMsg += " Registered.\n";
		//OutputDebugStringA(debugMsg.c_str());
	}
	else
	{
		// Scene graph loaded by XML (ARN30 format)
	}

	ArnNode::interconnect(sceneRoot);
}

HRESULT
ArnIpo::RegisterAnimationSRTKeys(const char* pName,							// Animation name
								 UINT NumScaleKeys,							// Number of scale keys
								 UINT NumRotationKeys,						// Number of rotation keys
								 UINT NumTranslationKeys,					// Number of translation keys
								 CONST ARNKEY_VECTOR3* pScaleKeys,			// Array of scale keys
								 CONST ARNKEY_QUATERNION* pRotationKeys,	// Array of rotation keys
								 CONST ARNKEY_VECTOR3* pTranslationKeys,	// Array of translation keys
								 DWORD *pAnimationIndex						// Returns the animation index
								)
{
	// TODO: Registration of IPO animation
	CurveData sxCurve, syCurve, szCurve;
	CurveData rxCurve, ryCurve, rzCurve, rwCurve;
	CurveData txCurve, tyCurve, tzCurve;

	sxCurve.nameStr = "ScaleX";
	sxCurve.name = CN_ScaleX;
	sxCurve.pointCount = NumScaleKeys;
	sxCurve.type = IPO_LIN; // TODO: Curve type?
	syCurve.nameStr = "ScaleY";
	syCurve.name = CN_ScaleY;
	syCurve.pointCount = NumScaleKeys;
	syCurve.type = IPO_LIN; // TODO: Curve type?
	szCurve.nameStr = "ScaleZ";
	szCurve.name = CN_ScaleZ;
	szCurve.pointCount = NumScaleKeys;
	szCurve.type = IPO_LIN; // TODO: Curve type?
	for (UINT i = 0; i < NumScaleKeys; ++i)
	{
		BezTripleData d;
		d.vec[0][0] = 0;
		d.vec[0][1] = 0;
		d.vec[2][0] = 0;
		d.vec[2][1] = 0;

		d.vec[1][0] = pScaleKeys->Time;
		d.vec[1][1] = pScaleKeys->Value.x;
		sxCurve.points.push_back(d);
		d.vec[1][0] = pScaleKeys->Time;
		d.vec[1][1] = pScaleKeys->Value.y;
		syCurve.points.push_back(d);
		d.vec[1][0] = pScaleKeys->Time;
		d.vec[1][1] = pScaleKeys->Value.z;
		szCurve.points.push_back(d);
	}

	rxCurve.nameStr = "QuatX";
	rxCurve.name = CN_QuatX;
	rxCurve.pointCount = NumRotationKeys;
	rxCurve.type = IPO_LIN; // TODO: Curve type?
	ryCurve.nameStr = "QuatY";
	ryCurve.name = CN_QuatY;
	ryCurve.pointCount = NumRotationKeys;
	ryCurve.type = IPO_LIN; // TODO: Curve type?
	rzCurve.nameStr = "QuatZ";
	rzCurve.name = CN_QuatZ;
	rzCurve.pointCount = NumRotationKeys;
	rzCurve.type = IPO_LIN; // TODO: Curve type?
	rwCurve.nameStr = "QuatW";
	rwCurve.name = CN_QuatW;
	rwCurve.pointCount = NumRotationKeys;
	rwCurve.type = IPO_LIN; // TODO: Curve type?
	for (UINT i = 0; i < NumRotationKeys; ++i)
	{
		BezTripleData d;
		d.vec[0][0] = 0;
		d.vec[0][1] = 0;
		d.vec[2][0] = 0;
		d.vec[2][1] = 0;

		d.vec[1][0] = pRotationKeys->Time;
		d.vec[1][1] = pRotationKeys->Value.x;
		rxCurve.points.push_back(d);
		d.vec[1][0] = pRotationKeys->Time;
		d.vec[1][1] = pRotationKeys->Value.y;
		ryCurve.points.push_back(d);
		d.vec[1][0] = pRotationKeys->Time;
		d.vec[1][1] = pRotationKeys->Value.z;
		rzCurve.points.push_back(d);
		d.vec[1][0] = pRotationKeys->Time;
		d.vec[1][1] = pRotationKeys->Value.w;
		rwCurve.points.push_back(d);
	}

	txCurve.nameStr = "LocX";
	txCurve.name = CN_ScaleX;
	txCurve.pointCount = NumTranslationKeys;
	txCurve.type = IPO_LIN; // TODO: Curve type?
	tyCurve.nameStr = "LocX";
	tyCurve.name = CN_ScaleY;
	tyCurve.pointCount = NumTranslationKeys;
	tyCurve.type = IPO_LIN; // TODO: Curve type?
	tzCurve.nameStr = "LocX";
	tzCurve.name = CN_ScaleZ;
	tzCurve.pointCount = NumTranslationKeys;
	tzCurve.type = IPO_LIN; // TODO: Curve type?
	for (UINT i = 0; i < NumTranslationKeys; ++i)
	{
		BezTripleData d;
		d.vec[0][0] = 0;
		d.vec[0][1] = 0;
		d.vec[2][0] = 0;
		d.vec[2][1] = 0;

		d.vec[1][0] = pTranslationKeys->Time;
		d.vec[1][1] = pTranslationKeys->Value.x;
		txCurve.points.push_back(d);
		d.vec[1][0] = pTranslationKeys->Time;
		d.vec[1][1] = pTranslationKeys->Value.y;
		tyCurve.points.push_back(d);
		d.vec[1][0] = pTranslationKeys->Time;
		d.vec[1][1] = pTranslationKeys->Value.z;
		tzCurve.points.push_back(d);
	}

	m_curves.push_back(sxCurve);
	m_curves.push_back(syCurve);
	m_curves.push_back(szCurve);
	m_curves.push_back(rxCurve);
	m_curves.push_back(ryCurve);
	m_curves.push_back(rzCurve);
	m_curves.push_back(rwCurve);
	m_curves.push_back(txCurve);
	m_curves.push_back(tyCurve);
	m_curves.push_back(tzCurve);

	// TODO: Animation index is invalid.
	*pAnimationIndex = 0;
	return S_OK;
}

ArnIpo* ArnIpo::create( const char* name, unsigned int curveCount )
{
	ArnIpo* ret = new ArnIpo();
	ret->setName(name);
	ret->m_curveCount = curveCount;
	return ret;
}
//////////////////////////////////////////////////////////////////////////

HRESULT ArnCreateKeyframedAnimationSet( const char* pName, double TicksPerSecond, ARNPLAYBACK_TYPE Playback, UINT NumAnimations, UINT NumCallbackKeys, CONST ARNKEY_CALLBACK* pCallbackKeys, ArnIpo** ppAnimationSet )
{
	assert(*ppAnimationSet == 0);
	assert(NumAnimations >= 1);
	assert(NumCallbackKeys == 0);
	assert(pCallbackKeys == 0);
	*ppAnimationSet = ArnIpo::create(pName, NumAnimations);
	//D3DXCreateKeyframedAnimationSet(pName, TicksPerSecond, Playback, NumAnimations, NumCallbackKeys, pCallbackKeys, ppAnimationSet);
	return S_OK;
}
