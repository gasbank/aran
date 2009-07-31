#pragma once
#include "ArnXformable.h"

struct NodeBase;
struct NodeBone1;
struct NodeBone2;

class ArnBone : public ArnXformable
{
public:
											~ArnBone(void);

	static ArnBone*							createFrom(const NodeBase* nodeBase);
	static ArnBone*							createFrom(const DOMElement* elm);
	void									setFrameData(const MyFrameData* frameData) { m_frameData = frameData; }
	const MyFrameData*						getFrameData() const { return m_frameData; }
	const BoneData&							getBoneData() const { return m_data; }
	unsigned int							getChildBoneCount() const;
	ArnVec3									getBoneDirection() const { return ArnVec3Substract(m_tailPos, m_headPos); }
	// *** INTERNAL USE ONLY START ***
	virtual void							interconnect(ArnNode* sceneRoot) { ArnNode::interconnect(sceneRoot); }
	// *** INTERNAL USE ONLY END ***
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
