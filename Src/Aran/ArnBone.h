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
	/*!
	 * @brief head, tail, roll 값으로 뼈대를 생성
	 *
	 * head, tail, roll 값으로부터 변환행렬을 역으로 계산합니다. 변환행렬이 만들어진 이후에는
	 * 렌더링할 때에는 head, tail값을 이용해 뼈대의 길이만 계산하고, 변환행렬로 회전 행렬을 얻습니다.
	 * 즉, 생성된 이후 head, tail값을 바꾸는 것만으로는 뼈대의 위치가 변경되지 않습니다.
	 */
	static ArnBone*							createFrom(const ArnVec3& head, const ArnVec3& tail, const float roll);
	static ArnBone*							createFrom(const NodeBase* nodeBase);
	static ArnBone*							createFrom(const TiXmlElement* elm);
	void									setFrameData(const MyFrameData* frameData) { m_frameData = frameData; }
	const MyFrameData*						getFrameData() const { return m_frameData; }
	const BoneData&							getBoneData() const { return m_data; }
	/*!
	 * @brief 하위 ArnBone 개수 반환
	 * @param bRecursive \c false 면 바로 속한 자식 ArnBone 개수만을 반환, \c true 면 하위 자식 ArnBone 총 개수 반환
	 * @sa ArnSkeleton::getChildBoneCount
	 */
	unsigned int							getChildBoneCount(bool bRecursive) const;
	/*!
	 * @brief 뼈의 방향
	 *
	 * 뼈의 위치는 뼈의 굵은 부분(head) 위치와 가는 부분(tail)위치로 정의됩니다.
	 * 이 때 뼈의 방향을 나타내는 벡터는 tail - head로 정의하며 이 함수는 그 벡터를 반환합니다.
	 * 결과적으로 head에서 tail를 가리키는 벡터가 반환됩니다.
	 * 이 벡터는 정규화되지 않습니다.
	 */
	ArnVec3									getBoneDirection() const { return ArnVec3Substract(m_tailPos, m_headPos); }

	void									getRotLimit(AxisEnum axis, float& minimum, float& maximum) const;
	void									setRotLimit(AxisEnum axis, float minimum, float maximum);
	void									clearRotLimit(AxisEnum axis);
	ArnBone*								getFirstChildBone() const;
	/*!
	 * @internalonly
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
	float									m_rotLimit[3][2]; // [X, Y, Z axis][min, max]
};
