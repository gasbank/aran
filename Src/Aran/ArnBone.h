/*!
 * @file Arnbone.h
 * @author Geoyeob Kim
 * @date 2009
 */
#pragma once
#include "ArnXformable.h"

struct NodeBase;
struct NodeBone1;
struct NodeBone2;

/*!
 * @brief ArnSkeleton 구성하는 하나의 뼈를 나타내는 클래스
 */
class ARAN_API ArnBone : public ArnXformable
{
public:
											~ArnBone(void);
	static ArnBone*							createFrom(const NodeBase* nodeBase);
	static ArnBone*							createFrom(const TiXmlElement* elm);
	void									setFrameData(const MyFrameData* frameData) { m_frameData = frameData; }
	const MyFrameData*						getFrameData() const { return m_frameData; }
	const BoneData&							getBoneData() const { return m_data; }
	/*!
	 * @brief 하위 ArnBone 총 개수 반환
	 * @remark 자식뿐만 아니라 이 뼈에 속해있는 모든 하위 ArnBone의 총 개수를 재귀적으로 합산하여 반환합니다.
	 * @sa ArnSkeleton::getChildBoneCount
	 */
	unsigned int							getChildBoneCount() const;
	/*!
	 * @brief 뼈의 방향
	 *
	 * 뼈의 위치는 뼈의 굵은 부분(head) 위치와 가는 부분(tail)위치로 정의됩니다.
	 * 이 때 뼈의 방향을 나타내는 벡터는 tail - head로 정의하며 이 함수는 그 벡터를 반환합니다.
	 * 결과적으로 head에서 tail를 가리키는 벡터가 반환됩니다.
	 */
	ArnVec3									getBoneDirection() const { return ArnVec3Substract(m_tailPos, m_headPos); }
	/*!
	 * @name Internal use only methods
	 *
	 * These methods are exposed in order to make internal linkage between objects or initialization.
	 * Clients should aware that these are not for client-side APIs.
	 */
	//@{
	virtual void							interconnect(ArnNode* sceneRoot) { ArnNode::interconnect(sceneRoot); }
	//@}
private:
											ArnBone(void);
	void									buildFrom(const NodeBone1* nb);
	void									buildFrom(const NodeBone2* nb);
	void									setHeadPos(const ArnVec3& pos) { m_headPos = pos; }
	void									setTailPos(const ArnVec3& pos) { m_tailPos = pos; }
	void									setRoll(float v) { m_roll = v; }
	BoneData								m_data;
	const MyFrameData*						m_frameData;
	ArnVec3									m_headPos;
	ArnVec3									m_tailPos;
	float									m_roll;
};
