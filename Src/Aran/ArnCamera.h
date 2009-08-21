/*!
 * @author Geoyeob Kim
 * @date 2009
 *
 * Camera
 */
#pragma once
#include "ArnXformable.h"

struct NodeCamera1;
struct NodeCamera2;
struct NodeBase;

/*!
 * @brief 카메라 클래스
 */
class ARAN_API ArnCamera : public ArnXformable
{
public:
	virtual								~ArnCamera(void);
	static ArnCamera*					createFrom(const NodeBase* nodeBase);
	static ArnCamera*					createFrom(const char* name, const ArnQuat& rot, const ArnVec3& trans, float fov);
	static ArnCamera*					createFrom(const char* name, const ArnVec3& eye, const ArnVec3& target, const ArnVec3& up, float fov);
	static ArnCamera*					createFrom(const TiXmlElement* elm);
	/*!
	 * @deprecated
	 */
	ARN_NDD_CAMERA_CHUNK&				getCameraData() { return m_cameraData; }
	/*!
	 * @name 카메라 기본 특성
	 *
	 * near, far 클립, FOV(field of view), 우향/상향/LookAt 벡터, ortho/persp 속성을 설정하거나
	 * 가져올 수 있습니다.
	 */
	//@{
	float								getFarClip() const { return m_cameraData.farClip; }
	void								setFarClip(float v) { m_cameraData.farClip = v; }
	float								getNearClip() const { return m_cameraData.nearClip; }
	void								setNearClip(float v) { m_cameraData.nearClip = v; }
	float								getFov() const { return m_cameraData.fov; }
	void								setFov(float fov) { m_cameraData.fov = fov; }
	ArnVec3								getRightVec() const;
	ArnVec3								getUpVec() const;
	ArnVec3								getLookVec() const;
	void								setOrtho(bool v) { m_cameraData.bOrtho = v; }
	bool								isOrtho() const { return m_cameraData.bOrtho; }
	/*!
	 * @brief Orthographic projection의 영역 배율을 가져옴
	 */
	float								getOrthoScale() const { return m_cameraData.scale; }
	/*!
	 * @brief Orthographic projection의 영역 배율을 설정
	 */
	void								setOrthoScale(float v) { m_cameraData.scale = v; }
	//@}
	void								printCameraOrientation() const;
	/*!
	 * @internalonly
	 */
	//@{
	virtual void						interconnect(ArnNode* sceneRoot);
	//@}
private:
										ArnCamera();
	void								buildFrom(const NodeCamera1* nc);
	void								buildFrom(const NodeCamera2* nc);

	ARN_NDD_CAMERA_CHUNK				m_cameraData;
};
