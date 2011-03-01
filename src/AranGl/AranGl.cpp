/*!
 * @file AranGl.cpp
 * @author Geoyeob Kim
 * @date 2009
 *
 * ARAN Core 자료구조 중 렌더링 가능한 것을 OpenGL을 이용해
 * 렌더링하는 루틴을 모아 놓은 파일입니다.
 */
#include "AranGlPCH.h"
#include "AranGl.h"
#include "ArnMesh.h"
#include "ArnMeshGl.h"
#include "ArnTexture.h"
#include "ArnTextureGl.h"
#include "ArnBone.h"
#include "ArnLight.h"
#include "ArnMaterial.h"
#include "ArnSkinningShaderGl.h"
#include "Node.h"
#include "Tree.h"

static GLUquadric* gs_quadricSphere = 0;
static GLuint gs_glArrowList = 0;
static bool gs_bAranGlInitialized = false;
static ArnSkinningShaderGl* gs_skinningShaderGl = 0;

void
ArnConfigureViewportProjectionMatrixGl(const ArnViewportData* viewportData, const ArnCamera* cam)
{
	glViewport(viewportData->X, viewportData->Y, viewportData->Width, viewportData->Height);
	ArnConfigureProjectionMatrixGl(viewportData, cam);
}

void
ArnConfigureProjectionMatrixGl( const ArnViewportData* viewportData, const ArnCamera* cam )
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float aspect = 0;
		if (viewportData->Height == 0)
			aspect = (float)viewportData->Width;
		else
			aspect = (float)viewportData->Width / viewportData->Height;
	if (cam->isOrtho())
	{
		glOrtho(
			-0.5 * cam->getOrthoScale() * aspect,
			0.5 * cam->getOrthoScale() * aspect,
			-0.5 * cam->getOrthoScale(),
			0.5 * cam->getOrthoScale(),
			cam->getNearClip(),
			cam->getFarClip()
		);
	}
	else
	{
		float fovdeg = ArnToDegree(cam->getFov()); // OpenGL uses degrees as angle unit rather than radians.

		gluPerspective(fovdeg, aspect, cam->getNearClip(), cam->getFarClip());
	}
}


void
ArnConfigureViewMatrixGl(ArnCamera* cam)
{
    const ArnMatrix &localTf = cam->getAutoLocalXform();

	ARN_CAMERA mainCamera;
	mainCamera.eye.x = localTf.m[0][3];
	mainCamera.eye.y = localTf.m[1][3];
	mainCamera.eye.z = localTf.m[2][3];
	mainCamera.at.x = localTf.m[0][3] - localTf.m[0][2];
	mainCamera.at.y = localTf.m[1][3] - localTf.m[1][2];
	mainCamera.at.z = localTf.m[2][3] - localTf.m[2][2];
	mainCamera.up.x = localTf.m[0][1];
	mainCamera.up.y = localTf.m[1][1];
	mainCamera.up.z = localTf.m[2][1];
	mainCamera.nearClip = cam->getNearClip();
	mainCamera.farClip = cam->getFarClip();
	mainCamera.angle = cam->getFov(); // FOV in radian

    // TODO: what?
	cam->getCameraData().pos = mainCamera.eye;
	cam->getCameraData().upVector = mainCamera.up;
	cam->getCameraData().targetPos = mainCamera.at;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		mainCamera.eye.x, mainCamera.eye.y, mainCamera.eye.z,
		mainCamera.at.x, mainCamera.at.y, mainCamera.at.z,
		mainCamera.up.x, mainCamera.up.y, mainCamera.up.z );
}

void
ArnConfigureViewMatrixGl2(const ArnCamera *cam, const ArnVec3 *dPos, const ArnQuat *dQuat)
{
    const ArnMatrix &localTf = cam->getAutoLocalXform();

    float eyePos[3];
    if (dPos)
    {
        eyePos[0] = localTf.m[0][3] + dPos->x;
        eyePos[1] = localTf.m[1][3] + dPos->y;
        eyePos[2] = localTf.m[2][3] + dPos->z;
    }
    else
    {
        eyePos[0] = localTf.m[0][3];
        eyePos[1] = localTf.m[1][3];
        eyePos[2] = localTf.m[2][3];
    }

    ArnQuat q;
    ArnMatrix qMat;
    if (dQuat)
        q = (*dQuat) * cam->getLocalXform_Rot();
    else
        q = cam->getLocalXform_Rot();
    q.getRotationMatrix(&qMat);

    ARN_CAMERA mainCamera;
    mainCamera.eye.x = eyePos[0];
    mainCamera.eye.y = eyePos[1];
    mainCamera.eye.z = eyePos[2];
    mainCamera.at.x = eyePos[0] - qMat.m[0][2];
    mainCamera.at.y = eyePos[1] - qMat.m[1][2];
    mainCamera.at.z = eyePos[2] - qMat.m[2][2];
    mainCamera.up.x = qMat.m[0][1];
    mainCamera.up.y = qMat.m[1][1];
    mainCamera.up.z = qMat.m[2][1];
    mainCamera.nearClip = cam->getNearClip();
    mainCamera.farClip = cam->getFarClip();
    mainCamera.angle = cam->getFov(); // FOV in radian

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        mainCamera.eye.x, mainCamera.eye.y, mainCamera.eye.z,
        mainCamera.at.x, mainCamera.at.y, mainCamera.at.z,
        mainCamera.up.x, mainCamera.up.y, mainCamera.up.z );
}

void
ArnConfigureLightGl(GLuint lightId, const ArnLight* light)
{
	assert(lightId < 8);
	if (light)
	{
		DWORD lightType = light->getD3DLightData().Type;
		if (lightType == 1)
		{
			// Point light (e.g. bulb)
			ArnVec4 pos(light->getAutoLocalXform().getColumnVec3(3), 1);
			float a0 = 0.001f;
			float a1 = 0.1f;
			float a2 = 0.00001f;
      float cutoff = 180;
      glLightfv(GL_LIGHT0 + lightId, GL_POSITION, pos.getRawData());
      glLightfv(GL_LIGHT0 + lightId, GL_SPOT_DIRECTION, (const GLfloat*)&ArnConsts::ARNVEC4_ZERO);
			glLightfv(GL_LIGHT0 + lightId, GL_SPOT_CUTOFF, &cutoff);
			
      //glLightfv(GL_LIGHT0 + lightId, GL_CONSTANT_ATTENUATION, &a0);
			//glLightfv(GL_LIGHT0 + lightId, GL_LINEAR_ATTENUATION, &a1);
			//glLightfv(GL_LIGHT0 + lightId, GL_QUADRATIC_ATTENUATION, &a2);

			//pos.printFormatString();
		}
		else if (lightType == 2)
		{
			// Spot light
			ArnVec4 pos(light->getD3DLightData().Position, 1);
			ArnVec4 dir(light->getD3DLightData().Direction, 1);
			float cutoff = 90;
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			{
				//glLoadIdentity();
				glLightfv(GL_LIGHT0 + lightId, GL_POSITION, pos.getRawData());
				glLightfv(GL_LIGHT0 + lightId, GL_SPOT_DIRECTION, dir.getRawData());
			}
			glPopMatrix();
			glLightfv(GL_LIGHT0 + lightId, GL_SPOT_CUTOFF, &cutoff);
		}
		else if (lightType == 3)
		{
			// Directional light (e.g. sunlight)
			ArnVec4 dir(light->getD3DLightData().Direction, 0);
			float cutoff = 180;
			glPushMatrix();
			{
				//glLoadIdentity();
				glLightfv(GL_LIGHT0 + lightId, GL_POSITION, dir.getRawData());
			}
			glPopMatrix();
			glLightfv(GL_LIGHT0 + lightId, GL_SPOT_CUTOFF, &cutoff);
		}
		else
		{
			ARN_THROW_UNEXPECTED_CASE_ERROR
		}

		glLightfv(GL_LIGHT0 + lightId, GL_AMBIENT, (const GLfloat*)&light->getD3DLightData().Ambient);
		glLightfv(GL_LIGHT0 + lightId, GL_DIFFUSE, (const GLfloat*)&light->getD3DLightData().Diffuse);
		glLightfv(GL_LIGHT0 + lightId, GL_SPECULAR, (const GLfloat*)&light->getD3DLightData().Specular);
		//float exponent = 64;
		//glLightfv(GL_LIGHT0 + lightId, GL_SPOT_EXPONENT, (const GLfloat*)&exponent);

		glEnable(GL_LIGHT0 + lightId);
		//light->getD3DLightData().Ambient.printFormatString();
	}
	else
	{
		glDisable(GL_LIGHT0 + lightId);
	}
}

void
ArnDrawAxesGl(float size)
{
	glPushMatrix();
	glPushAttrib(GL_LINE_BIT | GL_LIGHTING_BIT);
	glLineWidth(2);
	glDisable(GL_LIGHTING);
	{
		glBegin(GL_LINES);
		glColor3f(1,0,0);		glVertex3f(0,0,0);		glVertex3f(size,0,0);
		glColor3f(0,1,0);		glVertex3f(0,0,0);		glVertex3f(0,size,0);
		glColor3f(0,0,1);		glVertex3f(0,0,0);		glVertex3f(0,0,size);
		glEnd();
	}
	glPopAttrib();
	glPopMatrix();
}

GLuint
ArnCreateNormalizationCubeMapGl()
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, tex);

	unsigned char * data = new unsigned char[32*32*3];
	if(!data)
	{
		printf("Unable to allocate memory for texture data for cube map\n");
		return 0;
	}

	//some useful variables
	int size=32;
	float offset=0.5f;
	float halfSize=16.0f;
	ArnVec3 tempVector;
	unsigned char * bytePtr;

	//positive x
	bytePtr=data;
	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tempVector.x = halfSize;
			tempVector.y = -(j+offset-halfSize);
			tempVector.z = -(i+offset-halfSize);
			ArnVec3Normalize(&tempVector, &tempVector);
			tempVector.x = (tempVector.x + 1) / 2;
			tempVector.y = (tempVector.y + 1) / 2;
			tempVector.z = (tempVector.z + 1) / 2;
			bytePtr[0]=(unsigned char)(tempVector.x * 255);
			bytePtr[1]=(unsigned char)(tempVector.y * 255);
			bytePtr[2]=(unsigned char)(tempVector.z * 255);
			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	//negative x
	bytePtr=data;
	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tempVector.x = -halfSize;
			tempVector.y = -(j+offset-halfSize);
			tempVector.z = i+offset-halfSize;
			ArnVec3Normalize(&tempVector, &tempVector);
			tempVector.x = (tempVector.x + 1) / 2;
			tempVector.y = (tempVector.y + 1) / 2;
			tempVector.z = (tempVector.z + 1) / 2;
			bytePtr[0]=(unsigned char)(tempVector.x * 255);
			bytePtr[1]=(unsigned char)(tempVector.y * 255);
			bytePtr[2]=(unsigned char)(tempVector.z * 255);
			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	//positive y
	bytePtr=data;
	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tempVector.x = i+offset-halfSize;
			tempVector.y = halfSize;
			tempVector.z = j+offset-halfSize;
			ArnVec3Normalize(&tempVector, &tempVector);
			tempVector.x = (tempVector.x + 1) / 2;
			tempVector.y = (tempVector.y + 1) / 2;
			tempVector.z = (tempVector.z + 1) / 2;
			bytePtr[0]=(unsigned char)(tempVector.x * 255);
			bytePtr[1]=(unsigned char)(tempVector.y * 255);
			bytePtr[2]=(unsigned char)(tempVector.z * 255);
			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	//negative y
	bytePtr=data;
	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tempVector.x = i+offset-halfSize;
			tempVector.y = -halfSize;
			tempVector.z = -(j+offset-halfSize);
			ArnVec3Normalize(&tempVector, &tempVector);
			tempVector.x = (tempVector.x + 1) / 2;
			tempVector.y = (tempVector.y + 1) / 2;
			tempVector.z = (tempVector.z + 1) / 2;
			bytePtr[0]=(unsigned char)(tempVector.x * 255);
			bytePtr[1]=(unsigned char)(tempVector.y * 255);
			bytePtr[2]=(unsigned char)(tempVector.z * 255);
			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	//positive z
	bytePtr=data;
	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tempVector.x = i+offset-halfSize;
			tempVector.y = -(j+offset-halfSize);
			tempVector.z = halfSize;
			ArnVec3Normalize(&tempVector, &tempVector);
			tempVector.x = (tempVector.x + 1) / 2;
			tempVector.y = (tempVector.y + 1) / 2;
			tempVector.z = (tempVector.z + 1) / 2;
			bytePtr[0]=(unsigned char)(tempVector.x * 255);
			bytePtr[1]=(unsigned char)(tempVector.y * 255);
			bytePtr[2]=(unsigned char)(tempVector.z * 255);
			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	//negative z
	bytePtr=data;
	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tempVector.x = -(i+offset-halfSize);
			tempVector.y = -(j+offset-halfSize);
			tempVector.z = -halfSize;
			ArnVec3Normalize(&tempVector, &tempVector);
			tempVector.x = (tempVector.x + 1) / 2;
			tempVector.y = (tempVector.y + 1) / 2;
			tempVector.z = (tempVector.z + 1) / 2;
			bytePtr[0]=(unsigned char)(tempVector.x * 255);
			bytePtr[1]=(unsigned char)(tempVector.y * 255);
			bytePtr[2]=(unsigned char)(tempVector.z * 255);
			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	delete [] data;

	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, 0);
	return tex;
}

/*!
 * @brief ArnBone 렌더링
 * @sa ArnNodeRenderGl
 *
 * ArnNodeRenderGl 의 서브 루틴으로 호출되는 함수입니다. 자식 노드까지 렌더링하지 않습니다.
 */
static void
ArnBoneRenderGl( const ArnBone* bone )
{
	// No push, pop matrix needed. It is done in the previous call level.
	assert(bone->isLocalXformDirty() == false);

	//ArnVec3 boneDir(bone->getBoneDirection());
	const float boneLength = bone->getBoneLength();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_CULL_FACE);

	// TODO: Bone animation
	ArnMatrix boneMat;
	ArnQuat boneQ = bone->getLocalXform_Rot() * bone->getAnimLocalXform_Rot();
	ArnMatrixTransformation(&boneMat, 0, 0, &bone->getLocalXform_Scale(), 0, &boneQ, &bone->getLocalXform_Trans());

	glMultTransposeMatrixf((const GLfloat*)boneMat.m);
	ArnDrawAxesGl(boneLength/4);
	glPushMatrix();
	{
		glLoadName(bone->getObjectId()); // For screen-space rendering based picking
		glRotatef(-90, 1, 0, 0); // Arrow model has +Z direction but bone has +Y direction. Rotate it to match direction.
		glLineWidth(1);
		glDisable(GL_LIGHTING);
		glColor3f(1, 1, 1);

		glScaled(boneLength/4, boneLength/4, boneLength);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glCallList(gs_glArrowList);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glEnable(GL_LIGHTING);
	}
	glPopMatrix();

	glPushMatrix();
	{
		// Move to tail position.
		glTranslatef(0, boneLength, 0);
		if (bone->getChildBoneCount(true) == 0)
		{
			// Draw an end-effector indicator.
			ArnSetupBasicMaterialGl(&ArnConsts::ARNMTRLDATA_RED);
			ArnRenderSphereGl(boneLength/12);
		}
		else
		{
			// Draw a joint indicator.
			ArnSetupBasicMaterialGl(&ArnConsts::ARNCOLOR_YELLOW);
			ArnRenderSphereGl(boneLength/12);
		}
	}
	glPopMatrix();

	glPopAttrib();
}

void
ArnSetupBasicMaterialGl(const ArnMaterialData* mtrlData)
{
	// TODO: Material and shininess settings on OpenGL context
	glMaterialfv(GL_FRONT, GL_AMBIENT, (const GLfloat*)&mtrlData->Ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, (const GLfloat*)&mtrlData->Diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, (const GLfloat*)&mtrlData->Specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, (const GLfloat*)&mtrlData->Emissive);
	const static float shininess = 5.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
}

void
ArnSetupBasicMaterialGl(const ArnColorValue4f* color)
{
	GLfloat light_ambient[4], light_diffuse[4], light_specular[4];
	light_ambient[0] = color->r*0.3f;
	light_ambient[1] = color->g*0.3f;
	light_ambient[2] = color->b*0.3f;
	light_ambient[3] = color->a;
	light_diffuse[0] = color->r*0.7f;
	light_diffuse[1] = color->g*0.7f;
	light_diffuse[2] = color->b*0.7f;
	light_diffuse[3] = color->a;
	light_specular[0] = color->r*0.2f;
	light_specular[1] = color->g*0.2f;
	light_specular[2] = color->b*0.2f;
	light_specular[3] = color->a;
	glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, light_ambient);
	glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, light_diffuse);
	glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, light_specular);
	glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 5.0f);
}

void
ArnSetupMaterialGl(const ArnMaterial* mtrl)
{
	ArnSetupBasicMaterialGl(&mtrl->getD3DMaterialData());

	// TODO: Handling of multiple textures on a single material is missing.
	if (mtrl->getTextureCount())
	{
		const ArnTexture* tex = mtrl->getFirstTexture();
		const ArnRenderableObject* renderable = tex->getRenderableObject();
		if (renderable) {
		  // 'Rendering texture' has meaning of 'binding texture'
		  renderable->render(false);
    } else {
      // We do not have the proper texture image in this case.
    }
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// TODO: Shadeless material
	if (mtrl->isShadeless())
	{
		ArnSetupBasicMaterialGl(&ArnConsts::ARNMTRLDATA_WHITE);
	}
}

/*!
 * @brief ArnSkeleton 렌더링
 * @sa ArnNodeRenderGl
 *
 * ArnNodeRenderGl 의 서브 루틴으로 호출되는 함수입니다. 자식 노드까지 렌더링하지 않습니다.
 */
static void
ArnSkeletonRenderGl( const ArnSkeleton* skel )
{
	// No push, pop matrix needed. It is done in the previous call level.
	assert(skel->isLocalXformDirty() == false);
	glMultTransposeMatrixf((float*)skel->getLocalXform().m);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	ArnDrawAxesGl(0.75);
	ArnSetupBasicMaterialGl(&ArnConsts::ARNCOLOR_BLUE);
	ArnRenderSphereGl(0.1);
}

static void
InitializeArnTextureRenderableObjectGl( INOUT ArnTexture* tex )
{
  ArnNode *gltex = ArnTextureGl::createFrom(tex);
  if (gltex)
	  tex->attachChild( gltex );
}

static void
InitializeArnMeshRenderableObjectGl( INOUT ArnMesh* mesh )
{
	mesh->attachChild( ArnMeshGl::createFrom(mesh) );
}

static void
InitializeArnMaterialRenderableObjectGl( const ArnMaterial* mtrl )
{
	unsigned int texCount = mtrl->getTextureCount();
	for (unsigned int i = 0; i < texCount; ++i)
	{
		ArnTexture* tex = mtrl->getD3DTexture(i);
		tex->init();
		InitializeArnTextureRenderableObjectGl(tex);
	}
}

void
ArnInitializeRenderableObjectsGl( ArnNode* node )
{
	if (node->getType() == NDT_RT_MESH)
	{
		ArnMesh* mesh = static_cast<ArnMesh*>(node);
		InitializeArnMeshRenderableObjectGl(mesh);
	}
	else if (node->getType() == NDT_RT_MATERIAL)
	{
		ArnMaterial* mtrl = static_cast<ArnMaterial*>(node);
		InitializeArnMaterialRenderableObjectGl(mtrl);
	}
	else if (node->getType() == NDT_RT_SKELETON)
	{
		ArnSkeleton* skel = static_cast<ArnSkeleton*>(node);
		skel->configureIpos();
	}

	foreach (ArnNode* child, node->getChildren())
	{
		ArnInitializeRenderableObjectsGl(child);
	}
}

/*!
 * @brief ArnMesh 렌더링
 * @sa ArnNodeRenderGl
 *
 * ArnNodeRenderGl 의 서브 루틴으로 호출되는 함수입니다. 자식 노드까지 렌더링하지 않습니다.
 */
static void
ArnMeshRenderGl( const ArnMesh* mesh, bool bIncludeShadeless )
{
	// No push, pop matrix needed. It is done in the previous call level.
	const ArnRenderableObject* renderable = mesh->getRenderableObject();
	assert(renderable);
	renderable->render(bIncludeShadeless);
}

static void ArnNodeRenderGl( const ArnNode* node, bool bIncludeShadeless )
{
	glPushMatrix(); // ArnNode-level push

	bool bVisible = true;
	switch (node->getType())
	{
	case NDT_RT_MESH:
		{
			const ArnMesh* mesh = reinterpret_cast<const ArnMesh*>(node);
			if (mesh->isVisible())
			{
				ArnMeshRenderGl(mesh, bIncludeShadeless);
			}
			else
			{
				bVisible = false;
			}
		}
		break;
	case NDT_RT_SKELETON:
		{
			const ArnSkeleton* skel = reinterpret_cast<const ArnSkeleton*>(node);
			if (skel->isVisible())
			{
				ArnSkeletonRenderGl(skel);
			}
			else
			{
				bVisible = false;
			}
		}
		break;
	case NDT_RT_BONE:
		{
			const ArnBone* bone = reinterpret_cast<const ArnBone*>(node);
			if (bone->isVisible())
			{
				ArnBoneRenderGl(bone);
			}
			else
			{
				bVisible = false;
			}
		}
		break;
	default:
		// Not a renderable object.
		break;
	}

	if (bVisible)
	{
		foreach (const ArnNode* child, node->getChildren())
		{
			ArnNodeRenderGl(child, bIncludeShadeless);
		}
	}

	glPopMatrix(); // ArnNode-level pop
}

void
ArnSceneGraphRenderGl( const ArnSceneGraph* sg, bool bIncludeShadeless )
{
	foreach (const ArnNode* node, sg->getChildren())
	{
		ArnNodeRenderGl(node, bIncludeShadeless);
	}
}

void
ArnRenderSphereGl(double radius, unsigned int slices, unsigned int stacks)
{
	// Did you initialize AranGl library by calling ArnInitializeGl() before use this function?
	assert(gs_quadricSphere);
	gluSphere(gs_quadricSphere, radius, slices, stacks);
}

void
ArnRenderBoundingBox(const boost::array<ArnVec3, 8>& bb)
{
	glBegin(GL_QUADS);
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[0]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[1]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[2]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[3]));

	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[0]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[4]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[5]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[1]));

	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[0]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[3]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[7]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[4]));

	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[7]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[6]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[5]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[4]));

	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[7]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[3]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[2]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[6]));

	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[6]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[2]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[1]));
	glVertex3fv(reinterpret_cast<const GLfloat*>(&bb[5]));
	glEnd();
}

void
setTransformGl (const double pos[3], const double R[12])
{
	GLdouble matrix[16];
	matrix[0]	= R[0];
	matrix[1]	= R[4];
	matrix[2]	= R[8];
	matrix[3]	= 0;
	matrix[4]	= R[1];
	matrix[5]	= R[5];
	matrix[6]	= R[9];
	matrix[7]	= 0;
	matrix[8]	= R[2];
	matrix[9]	= R[6];
	matrix[10]	= R[10];
	matrix[11]	= 0;
	matrix[12]	= pos[0];
	matrix[13]	= pos[1];
	matrix[14]	= pos[2];
	matrix[15]	= 1;
	glMultMatrixd (matrix);
}

void
ArnRenderGeneralBodyGl()
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

int
ArnInitializeGl()
{
	if (gs_bAranGlInitialized == false)
	{
		assert(!gs_quadricSphere);
		gs_quadricSphere = gluNewQuadric();
		assert(gs_quadricSphere);

		gs_glArrowList = glGenLists(1);
		glNewList(gs_glArrowList, GL_COMPILE);
		glBegin(GL_QUADS);
		glVertex3d(0.5, 0.5, 0);
		glVertex3d(-0.5, 0.5, 0);
		glVertex3d(-0.5, -0.5, 0);
		glVertex3d(0.5, -0.5, 0);
		glEnd();
		glBegin(GL_TRIANGLES);
		glVertex3d(0.5, 0.5, 0);
		glVertex3d(-0.5, 0.5, 0);
		glVertex3d(0, 0, 1);
		glVertex3d(-0.5, 0.5, 0);
		glVertex3d(-0.5, -0.5, 0);
		glVertex3d(0, 0, 1);
		glVertex3d(-0.5, -0.5, 0);
		glVertex3d(0.5, -0.5, 0);
		glVertex3d(0, 0, 1);
		glVertex3d(0.5, -0.5, 0);
		glVertex3d(0.5, 0.5, 0);
		glVertex3d(0, 0, 1);
		glEnd();
		glEndList();

		gs_skinningShaderGl = new ArnSkinningShaderGl();
		gs_skinningShaderGl->initShader();

		gs_bAranGlInitialized = true;
		return 0;
	}
	else
	{
		// Already initialized.
		return -1;
	}
}

int
ArnCleanupGl()
{
	if (gs_bAranGlInitialized)
	{
		assert(gs_quadricSphere);
		gluDeleteQuadric(gs_quadricSphere);
		gs_quadricSphere = 0;

		glDeleteLists(gs_glArrowList, 1);
		gs_glArrowList = 0;

		delete gs_skinningShaderGl;
		gs_skinningShaderGl = 0;
		gs_bAranGlInitialized = false;
		return 0;
	}
	else
	{
		// Not initialized, but cleanup called.
		return -1;
	}
}


// Draw the box from the origin to point r.
static void
NodeDrawBox(const Node& node, const bool bDrawJointIndicator)
{
	const VectorR3& r = node.getRelativePosition();
	const double length = r.Norm();
	glLoadName(node.getObjectId());
	glDisable(GL_CULL_FACE);
	glPushMatrix();
	{
		if (bDrawJointIndicator)
		{
			glPushMatrix();
			{
				glTranslated(r.x, r.y, r.z);
				if (!node.getRealParent())
				{
					// Draw a root node indicator.
					ArnSetupBasicMaterialGl(&ArnConsts::ARNCOLOR_BLUE);
					ArnRenderSphereGl(length/4);
				}
				else if (node.getRealParent() && node.getLeftNode())
				{
					// Draw a joint indicator.
					ArnSetupBasicMaterialGl(&ArnConsts::ARNCOLOR_YELLOW);
					ArnRenderSphereGl(length/4);
				}
				else if (node.getRealParent() && !node.getLeftNode())
				{
					// Draw an end-effector indicator.
					ArnSetupBasicMaterialGl(&ArnConsts::ARNCOLOR_RED);
					ArnRenderSphereGl(length/4);
				}
				else
				{
					ARN_THROW_UNEXPECTED_CASE_ERROR
				}
			}
			glPopMatrix();
		}

		if ( r.z!=0.0 || r.x!=0.0 )
		{
			double alpha = atan2(r.z, r.x);
			glRotated(ArnToDegree(alpha), 0.0, -1.0, 0.0);
		}

		if ( r.y!=0.0 )
		{
			double beta = atan2(r.y, sqrt(r.x*r.x+r.z*r.z));
			glRotated( ArnToDegree(beta), 0.0, 0.0, 1.0 );
		}

		glPushMatrix();
		{
			glRotatef(90, 0, 1, 0);
			glLineWidth(1);
			glDisable(GL_LIGHTING);
			glColor3f(1, 1, 1);

			//glScaled(0.15, 0.15, length);
			glScaled(length/4, length/4, length);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glCallList(gs_glArrowList);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_LIGHTING);
		}
		glPopMatrix();
	}
	glPopMatrix();
	glEnable(GL_CULL_FACE);
}

void
NodeDrawNode(const Node& node, bool isRoot, const bool bDrawJointIndicator, const bool bDrawRotationAxis, const bool bDrawRootNodeIndicator)
{
	if (!isRoot)
	{
		NodeDrawBox(node, bDrawJointIndicator);
	}
	else
	{
		// Draw a root node indicator.
		if (bDrawRootNodeIndicator)
		{
			glPushMatrix();
			glTranslated(node.getAttach().x, node.getAttach().y, node.getAttach().z);
			ArnSetupBasicMaterialGl(&ArnConsts::ARNCOLOR_BLUE);
			ArnRenderSphereGl(0.1);
			glPopMatrix();
		}
	}

	// Draw rotation axis
	if (bDrawRotationAxis)
	{
		const double rotAxisLen = 0.5;
		glDisable(GL_LIGHTING);
		glColor3f(1.0f, 1.0f, 0.0f);
		glLineWidth(2.0);
		glBegin(GL_LINES);
		{
			VectorR3 temp = node.getRelativePosition();
			temp.AddScaled(node.getRotationAxis(), rotAxisLen * node.getSize());
			glVertex3d( temp.x, temp.y, temp.z );
			temp.AddScaled(node.getRotationAxis(),-2.0*rotAxisLen*node.getSize());
			glVertex3d( temp.x, temp.y, temp.z );
		}
		glEnd();
		glLineWidth(1.0);
		glEnable(GL_LIGHTING);
	}

	const VectorR3& r = node.getRelativePosition();
	const VectorR3& v = node.getRotationAxis();
	glTranslated(r.x, r.y, r.z);
	glRotated(ArnToDegree(node.getJointAngle()), v.x, v.y, v.z);
}

static void
TreeDrawTree(const Tree& tree, const Node* node, const bool bDrawJointIndicator, const bool bDrawRotationAxis, const bool bDrawRootNodeIndicator)
{
	if (node)
	{
		glPushMatrix();
		{
			NodeDrawNode(*node, tree.GetRoot() == node, bDrawJointIndicator, bDrawRotationAxis, bDrawRootNodeIndicator); // Recursively draw node and update ModelView matrix
			if (node->getLeftNode())
			{
				TreeDrawTree(tree, node->getLeftNode(), bDrawJointIndicator, bDrawRotationAxis, bDrawRootNodeIndicator); // Draw tree of children recursively
			}
		}
		glPopMatrix();

		if (node->getRightNode())
		{
			TreeDrawTree(tree, node->getRightNode(), bDrawJointIndicator, bDrawRotationAxis, bDrawRootNodeIndicator); // Draw right siblings recursively
		}
	}
}

static void
DrawEndeffectorTarget(const Node* node)
{
	if (node)
	{
		if (node->isEndeffector())
		{
			// End-effector target indicator.
			const VectorR3& targetPos = node->getTarget();
			glPushMatrix();
			glTranslated(targetPos.x, targetPos.y, targetPos.z);
			ArnSetupBasicMaterialGl(&ArnConsts::ARNCOLOR_GREEN);
			ArnRenderSphereGl(0.1);
			glPopMatrix();
		}
		DrawEndeffectorTarget(node->getLeftNode());
		DrawEndeffectorTarget(node->getRightNode());
	}
}

void
TreeDraw(const Tree& tree, const bool bDrawJointIndicator, const bool bDrawEndeffectorIndicator, const bool bDrawRotationAxis, const bool bDrawRootNodeIndicator)
{
	TreeDrawTree(tree, tree.GetRoot(), bDrawJointIndicator, bDrawRotationAxis, bDrawRootNodeIndicator);
	if (bDrawEndeffectorIndicator)
		DrawEndeffectorTarget(tree.GetRoot());
}