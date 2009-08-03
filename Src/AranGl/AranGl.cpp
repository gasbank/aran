// AranGl.cpp : Defines the exported functions for the DLL application.
//

#include "AranGlPCH.h"
#include "AranGl.h"
#include "ArnMesh.h"
#include "ArnMeshGl.h"
#include "ArnTexture.h"
#include "ArnTextureGl.h"
#include "ArnBone.h"
#include "ArnLight.h"
#include "ArnMaterial.h"

void ArnConfigureViewportProjectionMatrixGl(const ArnViewportData* viewportData, const ArnCamera* cam)
{
	glViewport(viewportData->X, viewportData->Y, viewportData->Width, viewportData->Height);
	ArnConfigureProjectionMatrixGl(viewportData, cam);
}

void ArnConfigureProjectionMatrixGl( const ArnViewportData* viewportData, const ArnCamera* cam )
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float fovdeg = ArnToDegree(cam->getFov()); // OpenGL uses degrees as angle unit rather than radians.
	float aspect = 0;
	if (viewportData->Height == 0)
		aspect = (float)viewportData->Width;
	else
		aspect = (float)viewportData->Width / viewportData->Height;
	gluPerspective(fovdeg, aspect, cam->getNearClip(), cam->getFarClip());
}


void ArnConfigureViewMatrixGl(ArnCamera* cam)
{
	//ArnMatrix localTf = cam->getLocalXform();
	ArnMatrix localTf = cam->getFinalLocalXform();

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

void ArnConfigureLightGl(GLuint lightId, const ArnLight* light)
{
	assert(lightId < 8);
	if (light)
	{
		DWORD lightType = light->getD3DLightData().Type;
		if (lightType == 1)
		{
			// Point light (e.g. bulb)
			ArnVec4 pos(light->getFinalLocalXform().getColumnVec3(3), 1);
			float a0 = 0.001f;
			float a1 = 0.1f;
			float a2 = 0.00001f;
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			{
				//glLoadIdentity();
				glLightfv(GL_LIGHT0 + lightId, GL_POSITION, pos.getRawData());
				glLightfv(GL_LIGHT0 + lightId, GL_SPOT_DIRECTION, (const GLfloat*)&ArnConsts::ARNVEC4_ZERO);
			}
			glPopMatrix();
			glLightfv(GL_LIGHT0 + lightId, GL_CONSTANT_ATTENUATION, &a0);
			glLightfv(GL_LIGHT0 + lightId, GL_LINEAR_ATTENUATION, &a1);
			glLightfv(GL_LIGHT0 + lightId, GL_QUADRATIC_ATTENUATION, &a2);

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
			ArnVec4 dir(light->getD3DLightData().Direction, 1);
			float cutoff = 180;
			glPushMatrix();
			{
				//glLoadIdentity();
				glLightfv(GL_LIGHT0 + lightId, GL_SPOT_DIRECTION, dir.getRawData());
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

void ArnDrawAxesGl(float size)
{
	glPushMatrix();
	glPushAttrib(GL_LINE_BIT);
	{
		glLineWidth(2);
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glColor3f(1,0,0);		glVertex3f(0,0,0);		glVertex3f(size,0,0);
		glColor3f(0,1,0);		glVertex3f(0,0,0);		glVertex3f(0,size,0);
		glColor3f(0,0,1);		glVertex3f(0,0,0);		glVertex3f(0,0,size);
		glEnd();
		glEnable(GL_LIGHTING);
	}
	glPopAttrib();
	glPopMatrix();
}

GLuint ArnCreateNormalizationCubeMapGl()
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

static void ArnBoneRenderGl( const ArnBone* bone )
{
	//bone->recalcLocalXform(); // TODO: Is this necessary? -- maybe yes...
	assert(bone->isLocalXformDirty() == false);
	
	ArnVec3 boneDir(bone->getBoneDirection());
	float boneLength = ArnVec3GetLength(boneDir);
	glPushMatrix();
	{
		ArnQuat q = bone->getLocalXform_Rot() * bone->getAnimLocalXform_Rot();
		ArnMatrix matRot;
		q.getRotationMatrix(&matRot);
		glMultTransposeMatrixf((const GLfloat*)matRot.m);
		glPushMatrix();
		{
			glLoadName(bone->getObjectId()); // For screen-space rendering based picking
			glRotatef(-90, 1, 0, 0);
			glLineWidth(1);
			glDisable(GL_LIGHTING);
			glColor3f(1, 1, 1);

			glScaled(0.25, 0.25, boneLength);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glEnable(GL_LIGHTING);
		}
		glPopMatrix();
		glTranslatef(0, boneLength, 0);
		ArnDrawAxesGl(0.25);

		foreach (const ArnNode* node, bone->getChildren())
		{
			assert(node->getType() == NDT_RT_BONE);
			const ArnBone* bone = reinterpret_cast<const ArnBone*>(node);
			ArnBoneRenderGl(bone);
		}
	}
	glPopMatrix();
}

void ArnSetupMaterialGl(const ArnMaterial* mtrl)
{
	// TODO: Material (ambient? specular?)

	glMaterialfv(GL_FRONT, GL_AMBIENT, (const GLfloat*)&mtrl->getD3DMaterialData().Ambient);
	//glMaterialfv(GL_FRONT, GL_AMBIENT, (const GLfloat*)&POINT4FLOAT::ZERO);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, (const GLfloat*)&mtrl->getD3DMaterialData().Diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, (const GLfloat*)&mtrl->getD3DMaterialData().Specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, (const GLfloat*)&mtrl->getD3DMaterialData().Emissive);

	/*
	glMaterialfv(GL_FRONT, GL_AMBIENT, (const GLfloat*)&POINT4FLOAT::ZERO);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, (const GLfloat*)&mtrl->getD3DMaterialData().Diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, (const GLfloat*)&POINT4FLOAT::ZERO);
	*/

	// TODO: Shininess...
	float shininess = 100;
	glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);

	// TODO: Handling of multiple textures on a single material is missing.
	if (mtrl->getTextureCount())
	{
		const ArnTexture* tex = mtrl->getFirstTexture();
		const ArnRenderableObject* renderable = tex->getRenderableObject();
		assert(renderable);
		renderable->render();
		// 'Rendering texture' has meaning of 'binding texture'
		//
		// glBindTexture(GL_TEXTURE_2D, tex->getTextureId());
		// GLenum err = glGetError( );
		// assert(err == 0);
		//
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	if (mtrl->isShadeless())
		glDisable(GL_LIGHTING);
	else
		glEnable(GL_LIGHTING);
}

static void ArnSkeletonRenderGl( const ArnSkeleton* skel )
{
	glPushMatrix();
	{
		assert(skel->isLocalXformDirty() == false);
		//skel->recalcLocalXform(); // TODO: Is this necessary? -- maybe yes...

		glMultTransposeMatrixf((float*)skel->getLocalXform().m);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		ArnDrawAxesGl(0.5);
		foreach (const ArnNode* node, skel->getChildren())
		{
			assert(node->getType() == NDT_RT_BONE);
			const ArnBone* bone = reinterpret_cast<const ArnBone*>(node);
			ArnBoneRenderGl(bone);
		}
	}
	glPopMatrix();
}

static void InitializeArnTextureRenderableObjectGl( INOUT ArnTexture* tex )
{
	tex->attachChild( ArnTextureGl::createFrom(tex) );
}

static void InitializeArnMeshRenderableObjectGl( INOUT ArnMesh* mesh )
{
	mesh->attachChild( ArnMeshGl::createFrom(mesh) );
}

static void InitializeArnMaterialRenderableObjectGl( const ArnMaterial* mtrl )
{
	unsigned int texCount = mtrl->getTextureCount();
	for (unsigned int i = 0; i < texCount; ++i)
	{
		ArnTexture* tex = mtrl->getD3DTexture(i);
		tex->init();
		InitializeArnTextureRenderableObjectGl(tex);
	}
}

void ArnInitializeRenderableObjectsGl( ArnSceneGraph* sg )
{
	foreach (ArnNode* node, sg->getChildren())
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
	}
}

void ArnSceneGraphRenderGl( const ArnSceneGraph* sg )
{
	foreach (const ArnNode* node, sg->getChildren())
	{
		if (node->getType() == NDT_RT_MESH)
		{
			const ArnMesh* mesh = reinterpret_cast<const ArnMesh*>(node);
			ArnMeshRenderGl(mesh);
		}
		else if (node->getType() == NDT_RT_SKELETON)
		{
			const ArnSkeleton* skel = reinterpret_cast<const ArnSkeleton*>(node);
			ArnSkeletonRenderGl(skel);
		}
	}
}

static void ArnMeshRenderGl( const ArnMesh* mesh )
{
	const ArnRenderableObject* renderable = mesh->getRenderableObject();
	assert(renderable);
	renderable->render();
}
