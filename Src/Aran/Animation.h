#pragma once

class Animation
{
public:
	Animation(unsigned int totalCurveCount);
	~Animation(void);

	enum CurveName
	{
		CN_LOCX, CN_LOCY, CN_LOCZ,
		CN_ROTX, CN_ROTY, CN_ROTZ,
		CN_SCLX, CN_SCLY, CN_SCLZ,
		CN_SIZE,
	};
	unsigned int registerCurve(CurveName curveName, unsigned int pointCount, ArnCurve::CurveType curveType);

	bool setCurvePoint(CurveName curveName, POINT2FLOAT* point2Array);

private:
	struct AnimCurve
	{
		CurveName curveName;
		unsigned int pointCount;
		ArnCurve::CurveType curveType;
		POINT2FLOAT* point2Array;
	};

	AnimCurve* findCurve(CurveName curveName);

	unsigned int m_totalCurveCount;
	unsigned int m_curveCount;
	AnimCurve* m_curves;
	bool m_curveFlags[CN_SIZE];
};
