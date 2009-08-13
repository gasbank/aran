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
, m_ikSolver(0)
{
}

ArnSkeleton::~ArnSkeleton(void)
{
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
}

unsigned int
ArnSkeleton::getChildBoneCount() const
{
	unsigned int ret = 0;
	foreach(ArnNode* node, getChildren())
	{
		if (node->getType() == NDT_RT_BONE)
		{
			ret += 1 + static_cast<ArnBone*>(node)->getChildBoneCount();
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

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ArnGetGlobalBonePosition(ArnVec3* head, ArnVec3* tail, const ArnSkeleton* skel, const ArnBone* bone)
{
	assert(head && tail && skel && bone);

	std::list<const ArnBone*> parentList;
	{
		const ArnBone* parent = dynamic_cast<const ArnBone*>(bone->getParent());
		while (parent)
		{
			parentList.push_front(parent);
			parent = dynamic_cast<const ArnBone*>(parent->getParent());
		}
	}

	// 먼저 skeleton 자체의 위치와 회전을 설정
	ArnMatrix matRot;
	*head = skel->getLocalXform_Trans();
	ArnQuat qSkel = skel->getLocalXform_Rot();
	qSkel.getRotationMatrix(&matRot);

	foreach (const ArnBone* p, parentList)
	{
		ArnVec3 boneDir = p->getBoneDirection();
		ArnVec4 boneDirXformed;
		ArnVec3Transform(&boneDirXformed, &boneDir, &matRot);
		head->x += boneDirXformed.x / boneDirXformed.w;
		head->y += boneDirXformed.y / boneDirXformed.w;
		head->z += boneDirXformed.z / boneDirXformed.w;

		ArnQuat q = p->getLocalXform_Rot();
		ArnMatrix matRot2;
		q.getRotationMatrix(&matRot2);
		matRot = matRot * matRot2;
	}

	// Tail position goes one step further.
	ArnVec3 boneDir = bone->getBoneDirection();
	ArnVec4 boneDirXformed;
	ArnVec3Transform(&boneDirXformed, &boneDir, &matRot);
	tail->x = head->x + boneDirXformed.x / boneDirXformed.w;
	tail->y = head->y + boneDirXformed.y / boneDirXformed.w;
	tail->z = head->z + boneDirXformed.z / boneDirXformed.w;
}
