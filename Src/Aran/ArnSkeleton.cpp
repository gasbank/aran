#include "AranPCH.h"
#include "ArnSkeleton.h"
#include "ArnFile.h"
#include "ArnBone.h"
#include "VideoManGl.h"
#include "ArnAction.h"
#include "ArnIpo.h"
#include "ArnAnimationController.h"

ArnSkeleton::ArnSkeleton()
: ArnXformable(NDT_RT_SKELETON)
, m_defaultAction(0)
{
}

ArnSkeleton::~ArnSkeleton(void)
{
}

ArnSkeleton*
ArnSkeleton::createFrom( const NodeBase* nodeBase )
{
	ArnSkeleton* node = new ArnSkeleton();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_SKELETON1:
			node->buildFrom(static_cast<const NodeSkeleton1*>(nodeBase));
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

void
ArnSkeleton::buildFrom( const NodeSkeleton1* ns )
{
	m_data.name					= ns->m_nodeName;
	m_data.associatedMeshName	= ns->m_associatedMeshName;
	m_data.maxWeightsPerVertex	= ns->m_maxWeightsPerVertex;
	m_data.bonesCount			= ns->m_boneCount;

	assert(m_data.name == getName());

	if (strncmp(getName(), "Skeleton-", 9) == 0)
		setParentName(getName() + 9); // implicit parent
	else
		throw MyError(MEE_SKELETON1NODE_CORRUPTED);
}

void
ArnSkeleton::interconnect( ArnNode* sceneRoot )
{
	m_defaultAction = dynamic_cast<ArnAction*>(sceneRoot->getNodeByName(m_actionName));

	foreach(const STRING& actStripName, m_actionStripNames)
		m_actionStrips.push_back(dynamic_cast<ArnAction*>(sceneRoot->getNodeByName(actStripName)));

	ArnNode::interconnect(sceneRoot); // Do interconnect() of children nodes.
}

void
ArnSkeleton::render()
{
	glPushMatrix();
	{
		recalcLocalXform(); // TODO: Is this necessary? -- maybe yes...
		glMultTransposeMatrixf((float*)getLocalXform().m);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		ArnDrawAxesGl(0.5);
		foreach (ArnNode* node, getChildren())
		{
			assert(node->getType() == NDT_RT_BONE);
			ArnBone* bone = (ArnBone*)node;
			bone->render();
		}
	}
	glPopMatrix();
}

void
ArnSkeleton::configureIpos()
{
	if (!m_defaultAction)
		return;
	assert(!getAnimCtrl());
	if (m_actionStrips.size())
	{
		ArnAnimationController* animCtrl = 0;
		V_VERIFY( ArnCreateAnimationController(
			getChildBoneCount(),					/* MaxNumMatrices */
			m_actionStrips.size(),					/* MaxNumAnimationSets */
			m_actionStrips.size(),					/* MaxNumTracks */
			0,										/* MaxNumEvents */
			&animCtrl
			)
			);
		assert(m_defaultAction);
		unsigned int defaultActionIdx = 0;
		for (unsigned int i = 0; i < m_actionStrips.size(); ++i)
		{
			if (m_defaultAction == m_actionStrips[i])
				defaultActionIdx = i;
			animCtrl->RegisterAnimationSet(m_actionStrips[i]);
		}
		animCtrl->SetAction(defaultActionIdx);
		setAnimCtrl(animCtrl);
	}

	// Every bone in the skeleton has an animation controller and its own end keyframe time.
	// Since we need to synchronize the entire skeleton animation with single end keyframe time,
	// find longest IPO and apply its end time to other IPOs.
	typedef std::pair<ArnNode*, ArnIpo*> ObjIpoPair;
	foreach(ArnAction* action, m_actionStrips)
	{
		int maxEndKeyframe = 0;
		foreach(ObjIpoPair p, action->getObjectIpoMap())
			if (maxEndKeyframe < p.second->getEndKeyframe())
				maxEndKeyframe = p.second->getEndKeyframe();
		foreach(ObjIpoPair p, action->getObjectIpoMap())
			p.second->setEndKeyframe(maxEndKeyframe);
	}

	/*
	ArnIpo* ipo = getIpo();
	if (!ipo)
	{
		fprintf(stderr, " ** Animation controller cannot be configured since there is no IPO associated.\n");
		return;
	}

	ArnIpo* globalIpoNode = static_cast<ArnIpo*>(getSceneRoot()->getNodeByName("Global IPOs Node"));
	if (globalIpoNode)
	{
		// Older way (does not use XML file.)
		assert(globalIpoNode->getType() == NDT_RT_IPO);
		ArnIpo* animSet = globalIpoNode->getD3DXAnimSet();
		V_VERIFY(m_d3dxAnimCtrl->RegisterAnimationSet(animSet));
		m_d3dxAnimCtrl->SetTrackAnimationSet(0, animSet);
	}
	else
	{
		// Newer way
		m_d3dxAnimCtrl->RegisterAnimationSet(ipo);
		m_d3dxAnimCtrl->SetTrackAnimationSet(0, ipo);
	}
	m_d3dxAnimCtrl->SetTrackPosition(0, 0.0f);
	m_d3dxAnimCtrl->SetTrackSpeed(0, 1.0f);
	m_d3dxAnimCtrl->SetTrackWeight(0, 1.0f);
	m_d3dxAnimCtrl->SetTrackEnable(0, TRUE);
	V_VERIFY(m_d3dxAnimCtrl->RegisterAnimationOutput(getIpoName().c_str(), &m_animLocalXform, &m_animLocalXform_Scale, &m_animLocalXform_Rot, &m_animLocalXform_Trans));
	m_d3dxAnimCtrl->AdvanceTime( 0.0001 );



	// Every bone in the skeleton has an animation controller and its own end keyframe time.
	// Since we need to synchronize the entire skeleton animation with single end keyframe time,
	// find longest IPO and apply its end time to other IPOs.
	typedef std::pair<ArnNode*, ArnIpo*> ObjIpoPair;
	{
		int maxEndKeyframe = 0;
		foreach(ObjIpoPair p, m_action->getObjectIpoMap())
		{
			assert(p.first->getType() == NDT_RT_BONE);
			ArnBone* bone = static_cast<ArnBone*>(p.first);
			ArnIpo* ipo = p.second;
			bone->setIpo(ipo);
			bone->configureIpo();
			if (maxEndKeyframe < ipo->getEndKeyframe())
				maxEndKeyframe = ipo->getEndKeyframe();
		}
		foreach(ObjIpoPair p, m_action->getObjectIpoMap())
			p.second->setEndKeyframe(maxEndKeyframe);
	}

	foreach(ArnAction* action, m_actionStrips)
	{
		int maxEndKeyframe = 0;
		foreach(ObjIpoPair p, action->getObjectIpoMap())
		{
			assert(p.first->getType() == NDT_RT_BONE);
			ArnBone* bone = static_cast<ArnBone*>(p.first);
			ArnIpo* ipo = p.second;
			bone->setIpo(ipo);
			bone->configureIpo();
			if (maxEndKeyframe < ipo->getEndKeyframe())
				maxEndKeyframe = ipo->getEndKeyframe();
		}
		foreach(ObjIpoPair p, action->getObjectIpoMap())
			p.second->setEndKeyframe(maxEndKeyframe);
	}
	*/
}

unsigned int
ArnSkeleton::getChildBoneCount() const
{
	unsigned int ret = 0;
	foreach(ArnNode* node, getChildren())
	{
		if (node->getType() == NDT_RT_BONE)
		{
			ret += static_cast<ArnBone*>(node)->getChildBoneCount();
		}
	}
	return ret;
}

void ArnSkeleton::update( double fTime, float fElapsedTime )
{
	if (getAnimCtrl())
		getAnimCtrl()->update(fTime, fElapsedTime);
}

void ArnSkeleton::setActionToNext()
{
	if (getAnimCtrl())
		getAnimCtrl()->SetActionToNext();
}
