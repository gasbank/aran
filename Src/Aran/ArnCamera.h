#pragma once
#include "ArnXformable.h"

struct NodeCamera1;
struct NodeCamera2;
struct NodeBase;

class ARAN_API ArnCamera : public ArnXformable
{
public:
	virtual								~ArnCamera(void);
	static ArnCamera*					createFrom(const NodeBase* nodeBase);
	static ArnCamera*					createFrom(const char* name, const ArnQuat& rot, const ArnVec3& trans, float fov);
	static ArnCamera*					createFrom(const char* name, const ArnVec3& eye, const ArnVec3& target, const ArnVec3& up, float fov);
	static ArnCamera*					createFrom(xercesc::DOMElement* elm);
	ARN_NDD_CAMERA_CHUNK&				getCameraData() { return m_cameraData; }
	float								getFarClip() const { return m_cameraData.farClip; }
	float								getNearClip() const { return m_cameraData.nearClip; }
	float								getFov() const { return m_cameraData.fov; }
	void								setFov(float fov) { m_cameraData.fov = fov; }
	void								setNearClip(float v) { m_cameraData.nearClip = v; }
	void								setFarClip(float v) { m_cameraData.farClip = v; }
	void								printCameraOrientation() const;
	// *** INTERNAL USE ONLY START ***
	virtual void						interconnect(ArnNode* sceneRoot);
	// *** INTERNAL USE ONLY END ***
private:
										ArnCamera();
	void								buildFrom(const NodeCamera1* nc);
	void								buildFrom(const NodeCamera2* nc);

	ARN_NDD_CAMERA_CHUNK				m_cameraData;
};

/////////////////////////////////////////////////////////////////////////////

struct ARN_CAMERA
{
	// eye: Position of camera
	// at: Look-at vector
	// up: Up-vector

	ArnVec3 eye, at, up;
	float farClip, nearClip;
	float angle; // in radian
};
