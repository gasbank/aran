#include "StdAfx.h"
#include "Animation.h"

Animation::Animation( unsigned int totalCurveCount )
: m_totalCurveCount(totalCurveCount)
{
	m_curves = new AnimCurve[m_totalCurveCount];
	memset(m_curveFlags, 0, sizeof(m_curveFlags));
}
Animation::~Animation(void)
{
	delete m_curves;
}

unsigned int Animation::registerCurve( CurveName curveName, unsigned int pointCount, ArnCurve::CurveType curveType )
{
	if (m_curveCount < m_totalCurveCount && m_curveFlags[curveName] == false)
	{
		AnimCurve* curve = &m_curves[m_curveCount];
		curve->curveName = curveName;
		curve->pointCount = pointCount;
		curve->curveType = curveType;
		curve->point2Array = 0;
		++m_curveCount;
		return m_curveCount;
	}
	else
	{
		return 0;
	}
}

bool Animation::setCurvePoint( CurveName curveName, POINT2FLOAT* point2Array )
{
	AnimCurve* curve = findCurve(curveName);
	if (curve)
	{
		curve->point2Array = point2Array;
		return true;
	}
	else
	{
		return false;
	}
}

Animation::AnimCurve* Animation::findCurve( CurveName curveName )
{
	unsigned int i;
	for (i = 0; i < m_totalCurveCount; ++i)
	{
		if (m_curves[i].curveName == curveName)
			return &m_curves[i];
	}
	return 0;
}