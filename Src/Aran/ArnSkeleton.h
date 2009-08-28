/*!
 * @file ArnSkeleton.h
 * @author Geoyeob Kim
 * @date 2009
 */
#pragma once
#include "ArnXformable.h"

struct NodeBase;
struct NodeSkeleton1;
struct NodeHierarchy2;
class ArnAction;
class ArnBone;

/*!
 * @brief 뼈대 구조를 정의하는 클래스 (블랜더의 Armature)
 */
class ARAN_API ArnSkeleton : public ArnXformable
{
public:
										~ArnSkeleton(void);
	static ArnSkeleton*					createFrom(const TiXmlElement* elm);
	static ArnSkeleton*					createFromEmpty();
	void								render();
	void								configureIpos();
	void								setDefaultActionName(const char* name) { m_actionName = name; }
	const std::string&					getActionName() const { return m_actionName; }
	/*!
	 * @brief ArnBone 총 개수 반환
	 * @param bRecursive \c false 면 바로 속한 자식 ArnBone 개수만을 반환, \c true 면 하위 자식 ArnBone 총 개수 반환
	 * @sa ArnBone::getChildBoneCount
	 */
	unsigned int						getChildBoneCount(bool bRecursive) const;
	void								setActionToNext();
	/*!
	 * @name Internal use only methods
	 * These methods are exposed in order to make internal linkage between objects.
	 * You should aware that these are not for client-side APIs.
	 */
	//@{
	virtual void						interconnect(ArnNode* sceneRoot);
	//@}
protected:
	virtual void						update(double fTime, float fElapsedTime);
private:
										ArnSkeleton(void);
	std::string							m_actionName;
	ArnAction*							m_defaultAction;
	std::vector<std::string>			m_actionStripNames;
	std::vector<ArnAction*>				m_actionStrips;
};

/*!
 * @brief ArnSkeleton에 속하는 ArnBone의 head와 tail 위치의 global coordinates를 반환
 * @param head ArnBone의 global head position
 * @param tail ArnBone의 global tail position
 * @param skel ArnBone이 속해있는 ArnSkeleton
 * @param bone head 위치를 알고 싶은 ArnBone
 * @remarks ArnBone이 제공된 ArnSkeleton에 속하는지 확인하지는 않습니다.
 */
ARAN_API void ArnGetGlobalBonePosition(ArnVec3* head, ArnVec3* tail, const ArnSkeleton* skel, const ArnBone* bone);
