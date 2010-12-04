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
struct MyFrameData;
struct BoneData;
class ArnSkeleton;

/*!
 * @brief ArnSkeleton 구성하는 하나의 뼈를 나타내는 클래스
 *
 * ArnBone은 뼈의 시작 위치와 뼈가 뻗어나가는 방향, 길이, 뻗어나가는
 * 축에서의 회전으로 정의됩니다. 뼈의 시작 위치를 head, 끝 위치를
 * tail이라고 합니다. 또한, 뼈가 뻗어나가는 방향을 회전축으로 하여 회전된
 * 정도를 roll이라고 합니다.
 *
 * local frame의 원점이 head 위치이며, tail은 head에서 +Y 방향으로
 * 뼈의 길이만큼 이동하면 구할 수 있습니다. 즉, ArnBone 클래스에서는
 * 뼈의 길이와 회전(roll)만 따로 저장하고 있으면 됩니다.
 */
class ARAN_API ArnBone : public ArnXformable
{
public:
											~ArnBone(void);
	virtual const ArnMatrix&				getAutoLocalXform() const;

	/*!
	 * @brief length와 roll(bone space 상) 값을 설정하여 새 뼈 생성
	 */
	static ArnBone*							createFrom(const float length, const float roll);
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
	 * 뼈의 방향은 bone space에서 양의 Y축입니다.
	 */
	ArnVec3									getBoneDirection() const { return getAutoLocalXform().getColumnVec3(1); }
	float									getBoneLength() const { return m_boneLength; }
	/*!
	 * @brief 부모 ArnSkeleton 을 반환
	 * @return 있으면 부모 ArnSkeleton 포인터, 없으면 \c NULL 반환
	 */
	ArnSkeleton*							getParentSkeleton() const;
	void									getRotLimit(AxisEnum axis, float& minimum, float& maximum) const;
	void									setRotLimit(AxisEnum axis, float minimum, float maximum);
	void									clearRotLimit(AxisEnum axis);
	ArnBone*								getFirstChildBone() const;
	void									computeWorldHeadTail(ArnVec3& head, ArnVec3& tail) const;
	/*!
	 * @internalonly
	 */
	//@{
	virtual void							interconnect(ArnNode* sceneRoot);
	//@}
private:
											ArnBone(void);
	void									buildFrom(const NodeBone1* nb);
	void									buildFrom(const NodeBone2* nb);
	BoneData								m_data;
	const MyFrameData*						m_frameData;
	float									m_roll;
	float									m_boneLength;
	float									m_rotLimit[3][2]; // [X, Y, Z axis][min, max]
};
