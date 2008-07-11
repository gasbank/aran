#pragma once
#include "ArnNode.h"

struct NodeCamera1;
struct NodeCamera2;
struct NodeBase;

class ArnCamera : public ArnNode
{
public:
	ArnCamera();
	~ArnCamera(void);

	static ArnNode*		createFrom(const NodeBase* nodeBase);

private:
	void				buildFrom(const NodeCamera1* nc);
	void				buildFrom(const NodeCamera2* nc);

	ARN_NDD_CAMERA_CHUNK m_cameraData;
};
