#include "AranPCH.h"
#include "ArnBone.h"
#include "ArnFile.h"
#include "ArnMath.h"
#include "VideoManGl.h"

ArnBone::ArnBone(void)
: ArnXformable(NDT_RT_BONE)
, m_frameData()
, m_roll(0)
{
}

ArnBone::~ArnBone(void)
{
}

ArnBone*
ArnBone::createFrom( const NodeBase* nodeBase )
{
	ArnBone* node = new ArnBone();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_BONE1:
			node->buildFrom(static_cast<const NodeBone1*>(nodeBase));
			break;
		case NDT_BONE2:
			node->buildFrom(static_cast<const NodeBone2*>(nodeBase));
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

void
ArnBone::buildFrom( const NodeBone1* nb )
{
	m_data.nameFixed		= nb->m_nodeName;
	m_data.offsetMatrix		= *nb->m_offsetMatrix;
	m_data.infVertexCount	= nb->m_infVertexCount;
	unsigned int i;
	m_data.indices.resize(m_data.infVertexCount, 0);
	m_data.weights.resize(m_data.infVertexCount, 0);
	for (i = 0; i < m_data.infVertexCount; ++i)
	{
		m_data.indices[i] = nb->m_vertexIndices[i];
		m_data.weights[i] = nb->m_weights[i];
	}
}

void
ArnBone::buildFrom( const NodeBone2* nb )
{
	m_data.nameFixed		= nb->m_nodeName;
	m_data.offsetMatrix		= *nb->m_offsetMatrix;

	/*m_data.infVertexCount	= nb->m_infVertCount;
	unsigned int i;
	m_data.indices.resize(m_data.infVertexCount, 0);
	m_data.weights.resize(m_data.infVertexCount, 0);
	for (i = 0; i < m_data.infVertexCount; ++i)
	{
		m_data.indices[i] = nb->m_indWeightArray[i].ind;
		m_data.weights[i] = nb->m_indWeightArray[i].weight;
	}*/

	// If bone name is the same as its armature, this bone is root.
	setParentName( nb->m_parentBoneName );
}

void ArnBone::render()
{
	recalcLocalXform(); // TODO: Is this necessary? -- maybe yes...
	ArnVec3 boneDir = ArnVec3Substract(m_tailPos, m_headPos);
	float boneLength = ArnVec3GetLength(boneDir);
	glPushMatrix();
	{
		ArnQuat q = getLocalXform_Rot() * getAnimLocalXform_Rot();
		ArnMatrix matRot;
		q.getRotationMatrix(&matRot);
		glMultTransposeMatrixf((const GLfloat*)matRot.m);
		glPushMatrix();
		{
			glLoadName(getObjectId()); // For screenspace rendering based picking
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

unsigned int
ArnBone::getChildBoneCount() const
{
	unsigned int ret = 1;
	foreach(const ArnNode* node, getChildren())
	{
		if (node->getType() == NDT_RT_BONE)
		{
			ret += static_cast<const ArnBone*>(node)->getChildBoneCount();
		}
	}
	return ret;
}
