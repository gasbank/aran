#include "AranPCH.h"
#include "ArnIpo.h"
#include "ArnFile.h"

ArnIpo::ArnIpo(void)
: ArnNode(NDT_RT_IPO)
{
}

ArnIpo::~ArnIpo(void)
{
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
		cd.name = ni->m_curves[i].name;
		cd.type = ni->m_curves[i].type;
		cd.pointCount = ni->m_curves[i].pointCount;
		for (j = 0; j < cd.pointCount; ++j)
		{
			cd.points.push_back(ni->m_curves[i].points[j]);
		}
		m_curves.push_back(cd);
	}
}