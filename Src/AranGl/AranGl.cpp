// AranGl.cpp : Defines the exported functions for the DLL application.
//

#include "AranGlPCH.h"
#include "AranGl.h"

void ArnBoneRenderGl( const ArnBone* bone )
{
	bone->recalcLocalXform(); // TODO: Is this necessary? -- maybe yes...
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

		foreach (ArnNode* node, getChildren())
		{
			assert(node->getType() == NDT_RT_BONE);
			ArnBone* bone = (ArnBone*)node;
			bone->render();
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

	if (mtrl->getTextureCount())
	{
		const ArnTexture* tex = mtrl->getFirstTexture();
		glBindTexture(GL_TEXTURE_2D, tex->getTextureId());
		GLenum err = glGetError( );
		assert(err == 0);
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
