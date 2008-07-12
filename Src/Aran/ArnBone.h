#pragma once
#include "ArnNode.h"

struct NodeBase;
struct NodeBone1;

class ArnBone : public ArnNode
{
public:
	ArnBone(void);
	~ArnBone(void);

	static ArnNode*		createFrom(const NodeBase* nodeBase);
	void				setFrameData(const MyFrameData* frameData) { m_frameData = frameData; }
	const MyFrameData*	getFrameData() const { return m_frameData; }

private:
	void				buildFrom(const NodeBone1* nb);

	BoneData			m_data;
	const MyFrameData*	m_frameData;
};