#include "AranPCH.h"
#include "ArnCamera.h"
#include "ArnFile.h"
#include "ArnConsts.h"
#include "ArnMath.h"

ArnCamera::ArnCamera()
: ArnXformable(NDT_RT_CAMERA)
{
	m_cameraData.farClip = 1000.0f;
	m_cameraData.nearClip = 1.0f;
	m_cameraData.fov = float(ARN_PI / 4);
	m_cameraData.pos = ArnConsts::D3DXVEC3_ZERO;
	m_cameraData.rot = ArnConsts::D3DXQUAT_IDENTITY;
	m_cameraData.lookAtVector = ArnVec3(0, 0, -1);
	m_cameraData.targetPos = ArnConsts::D3DXVEC3_ZERO;
	m_cameraData.upVector = ArnConsts::D3DXVEC3_Y;
}

ArnCamera::~ArnCamera(void)
{
	//std::cerr << "ArnCamera dtor() called.\n";
}

ArnCamera*
ArnCamera::createFrom( const NodeBase* nodeBase )
{
	ArnCamera* node = new ArnCamera();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_CAMERA1:
			node->buildFrom(static_cast<const NodeCamera1*>(nodeBase));
			break;
		case NDT_CAMERA2:
			node->buildFrom(static_cast<const NodeCamera2*>(nodeBase));
			break;
		default:
			throw MyError(MEE_UNDEFINED_ERROR);
		}
	}
	catch (const MyError& e)
	{
		delete node;
		throw e;
	}
	return node;
}

ArnCamera*
ArnCamera::createFrom( const char* name, const ArnQuat& rot, const ArnVec3& trans, float fov )
{
	ArnCamera* node = new ArnCamera();
	node->setName(name);
	node->setLocalXform_Scale(ArnConsts::D3DXVEC3_ONE);
	node->setLocalXform_Rot(rot);
	node->setLocalXform_Trans(trans);
	node->setFov(fov);
	return node;
}

ArnCamera*
ArnCamera::createFrom( const char* name, const ArnVec3& eye, const ArnVec3& target, const ArnVec3& up, float fov, cml::Handedness handedness )
{
	ArnCamera* node = new ArnCamera();
	node->setName(name);
	node->setLocalXform_Scale(ArnConsts::D3DXVEC3_ONE);

	ArnVec3 lookDir = target - eye;
	lookDir /= ArnVec3Length(&lookDir);
	ArnVec3 right = ArnVec3GetCrossProduct(up, ArnVec3(-lookDir.x, -lookDir.y, -lookDir.z));
	right /= ArnVec3Length(&right);
	ArnMatrix mat;

	// Right vector (1st column)
	mat.m[0][0] = right.x;
	mat.m[1][0] = right.y;
	mat.m[2][0] = right.z;
	mat.m[3][0] = 0.0f;
	
	// Up vector (2nd column)
	mat.m[0][1] = up.x;
	mat.m[1][1] = up.y;
	mat.m[2][1] = up.z;
	mat.m[3][1] = 0.0f;

	// negative(-) look direction vector (3rd column)
	mat.m[0][2] = -lookDir.x;
	mat.m[1][2] = -lookDir.y;
	mat.m[2][2] = -lookDir.z;
	mat.m[3][2] = 0.0f;
	
	// Translation vector (4th column)
	mat.m[0][3] = eye.x;
	mat.m[1][3] = eye.y;
	mat.m[2][3] = eye.z;
	mat.m[3][3] = 1.0f;
	
	node->setLocalXform(mat);
	node->setFov(fov);
	return node;
}

void
ArnCamera::buildFrom( const NodeCamera1* nc )
{
	m_cameraData = *nc->m_camera;
}

void
ArnCamera::buildFrom( const NodeCamera2* nc )
{
	setParentName(nc->m_parentName);
	setLocalXform(*nc->m_localXform);
	setIpoName(nc->m_ipoName);
	m_cameraData.nearClip		= nc->m_clipStart;
	m_cameraData.farClip		= nc->m_clipEnd;
	m_cameraData.lookAtVector	= ArnVec3(0, 0, -1);
	m_cameraData.pos			= ArnConsts::D3DXVEC3_ZERO;
	m_cameraData.rot			= ArnConsts::D3DXQUAT_IDENTITY;
	m_cameraData.targetPos		= ArnConsts::D3DXVEC3_ZERO;
	m_cameraData.upVector		= ArnConsts::D3DXVEC3_Y;
}

void
ArnCamera::interconnect( ArnNode* sceneRoot )
{
	setIpo(getIpoName());
	configureAnimCtrl();

	ArnNode::interconnect(sceneRoot);
}

void
ArnCamera::printCameraOrientation() const
{
	//
	// We use the camera orientation model of blender.
	// If localTf == {identity matrix},
	// we have   righ    vector == positive x-axis
	//           up      vector == positive y-axis
	//           lookdir vector == negative z-axis
	//
	const ArnMatrix& localTf = getLocalXform();
	std::string str;
	char buf[256];
	sprintf(buf, "   Camera Object [%s] Orientation :\n", getName());
	str.append(buf);
	sprintf(buf, "(x: right   ) 1st Column: %.3f, %.3f, %.3f\n", localTf.m[0][0], localTf.m[1][0], localTf.m[2][0]);
	str.append(buf);
	sprintf(buf, "(y: up      ) 2nd Column: %.3f, %.3f, %.3f\n", localTf.m[0][1], localTf.m[1][1], localTf.m[2][1]);
	str.append(buf);
	sprintf(buf, "(z: -lookdir) 3rd Column: %.3f, %.3f, %.3f\n", localTf.m[0][2], localTf.m[1][2], localTf.m[2][2]);
	str.append(buf);
	sprintf(buf, "(     trans ) 4th Column: %.3f, %.3f, %.3f\n", localTf.m[0][3], localTf.m[1][3], localTf.m[2][3]);
	str.append(buf);
	std::cout << str;
}
