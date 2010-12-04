#include "AranPCH.h"
#include "ArnSkeleton.h"
#include "ArnBone.h"
#include "ArnAction.h"
#include "ArnIpo.h"
#include "ArnAnimationController.h"
#include "ArnMath.h"

ArnSkeleton::ArnSkeleton()
: ArnXformable(NDT_RT_SKELETON)
, m_defaultAction(0)
{
}

ArnSkeleton::~ArnSkeleton(void)
{
}

ArnSkeleton* ArnSkeleton::createFromEmpty()
{
	ArnSkeleton* ret = new ArnSkeleton();
	return ret;
}

void
ArnSkeleton::interconnect( ArnNode* sceneRoot )
{
	m_defaultAction = dynamic_cast<ArnAction*>(sceneRoot->getNodeByName(m_actionName));

	foreach(const std::string& actStripName, m_actionStripNames)
		m_actionStrips.push_back(dynamic_cast<ArnAction*>(sceneRoot->getNodeByName(actStripName)));

	ArnNode::interconnect(sceneRoot); // Do interconnect() of children nodes.
}

void
ArnSkeleton::render()
{
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
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
			getChildBoneCount(true),					/* MaxNumMatrices */
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

      animCtrl->SetTrackAnimationSet(i, i);
      animCtrl->SetTrackWeight(i, 1.0);
      animCtrl->SetTrackSpeed(i, 1.0);
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
}

unsigned int
ArnSkeleton::getChildBoneCount(bool bRecursive) const
{
	unsigned int ret = 0;
	foreach(ArnNode* node, getChildren())
	{
		if (node->getType() == NDT_RT_BONE)
		{
			ret += 1 + (bRecursive ? static_cast<const ArnBone*>(node)->getChildBoneCount(true) : 0);
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
