#include "AranPCH.h"
#include "ArnIpo.h"
#include "ArnFile.h"
#include "Animation.h"
#include "ArnMath.h"

ArnIpo::ArnIpo(void)
: ArnNode(NDT_RT_IPO), m_d3dxAnimSet(0), m_curveNames(0)
{
}

ArnIpo::~ArnIpo(void)
{
	SAFE_RELEASE(m_d3dxAnimSet);
}

ArnNode* ArnIpo::createFrom( const NodeBase* nodeBase )
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
			cd.points.push_back(ni->m_curves[i].points[j]);
		}
		m_curves.push_back(cd);

		m_curveNames |= (DWORD)cd.name;
	}
}

void ArnIpo::interconnect( ArnNode* sceneRoot )
{
	unsigned int i, j;

	if (m_curveCount == 0) // If this is IPO container node ...
	{
		if (getNodeCount()) // ... and there are some IPOs there
			D3DXCreateKeyframedAnimationSet(getName(), FPS /* ticks per second */, D3DXPLAY_LOOP, getIpoCount() /* num Animations seq */, 0, 0, &m_d3dxAnimSet);
	}
	else
	{
		ArnIpo* ipoContainer = static_cast<ArnIpo*>(getParent());
		assert(ipoContainer->getType() == NDT_RT_IPO);
		LPD3DXKEYFRAMEDANIMATIONSET d3dxAnimSet = ipoContainer->getD3DXAnimSet();
		assert(d3dxAnimSet);
		
		const unsigned int keyCount = 121;
		const unsigned int sampleCount = keyCount * FPS;
		std::vector<D3DXKEY_VECTOR3> scaleKeys(sampleCount);
		std::vector<D3DXKEY_VECTOR3> transKeys(sampleCount);
		std::vector<D3DXKEY_QUATERNION> rotKeys(sampleCount);
		for (i = 0; i < sampleCount; ++i)
		{
			float keyTime = (float)i/FPS;
			rotKeys[i].Time = keyTime;
			rotKeys[i].Value = DX_CONSTS::D3DXQUAT_IDENTITY;
			scaleKeys[i].Time = keyTime;
			scaleKeys[i].Value = DX_CONSTS::D3DXVEC3_ONE;
			transKeys[i].Time = keyTime;
			transKeys[i].Value = DX_CONSTS::D3DXVEC3_ZERO;

			float eulX = 0.0f, eulY = 0.0f, eulZ = 0.0f; // Euler rotation values
			
			for (j = 0; j < getCurveCount(); ++j)
			{
				const CurveData& cd = getCurveData(j);
				float val = Animation::EvalCurveInterp(&cd, keyTime);
				switch (cd.name)
				{
				case CN_LocX:	transKeys[i].Value.x = val;		break;
				case CN_LocY:	transKeys[i].Value.y = val;		break;
				case CN_LocZ:	transKeys[i].Value.z = val;		break;
				case CN_ScaleX: scaleKeys[i].Value.x = val;		break;
				case CN_ScaleY: scaleKeys[i].Value.y = val;		break;
				case CN_ScaleZ: scaleKeys[i].Value.z = val;		break;
				case CN_RotX:	eulX = D3DXToRadian(val);		break;
				case CN_RotY:	eulY = D3DXToRadian(val);		break;
				case CN_RotZ:	eulZ = D3DXToRadian(val);		break;
				default:		throw MyError(MEE_UNSUPPORTED_CURVENAME);
				}
			}
			D3DXQUATERNION quatX, quatY, quatZ;
			D3DXVECTOR3 vEulX(eulX, 0, 0);
			D3DXVECTOR3 vEulY(0, eulY, 0);
			D3DXVECTOR3 vEulZ(0, 0, eulZ);
			quatX = ArnMath::EulerToQuat(&vEulX);
			quatY = ArnMath::EulerToQuat(&vEulY);
			quatZ = ArnMath::EulerToQuat(&vEulZ);
			rotKeys[i].Value = quatZ * quatY * quatX;
		}
		DWORD animIdx;
		V_VERIFY(d3dxAnimSet->RegisterAnimationSRTKeys(getName(), sampleCount, sampleCount, sampleCount, &scaleKeys[0], &rotKeys[0], &transKeys[0], &animIdx));
		STRING debugMsg;
		debugMsg = " - ";
		debugMsg += getName();
		debugMsg += " Registered.\n";
		//OutputDebugStringA(debugMsg.c_str());
	}

	ArnNode::interconnect(sceneRoot);
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
#undef CNCONVERT
	return CN_UNKNOWN;
}