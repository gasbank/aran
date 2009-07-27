#pragma once

class ArnSkinInfo;


class Animation
{
public:
										Animation(unsigned int totalCurveCount);
										~Animation(void);
	unsigned int						registerCurve(CurveName curveName, unsigned int pointCount, CurveType curveType);
	bool								setCurvePoint(CurveName curveName, POINT2FLOAT* point2Array);

	// originally written by Blender;
	// file: blendersvn/blender/source/blender/blenkernel/intern/ipo.c
	// function: float eval_icu(IpoCurve *icu, float ipotime)
	static float						EvalCurveInterp(const CurveData* icu, float ipotime);

private:
	struct AnimCurve
	{
		CurveName curveName;
		unsigned int pointCount;
		CurveType curveType;
		POINT2FLOAT* point2Array;
	};
	AnimCurve*							findCurve(CurveName curveName);
	unsigned int						m_totalCurveCount;
	unsigned int						m_curveCount;
	AnimCurve*							m_curves;
	bool								m_curveFlags[CN_SIZE];
};

//////////////////////////////////////////////////////////////////////////

// Creates a skin info object based on the number of vertices, number of bones, and a FVF describing the vertex layout of the target vertices
// The bone names and initial bone transforms are not filled in the skin info object by this method.
HRESULT ArnCreateSkinInfoFVF( DWORD NumVertices, DWORD FVF, DWORD NumBones, ArnSkinInfo** ppSkinInfo);
