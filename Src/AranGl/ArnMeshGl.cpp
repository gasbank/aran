#include "AranGlPCH.h"
#include "ArnMeshGl.h"
#include "ArnVertexBuffer.h"
#include "ArnBinaryChunk.h"
#include "ArnMaterial.h"
#include "AranGl.h"
#include "ArnSkinningShaderGl.h"

static float lmodel_twoside[] = {GL_TRUE};
static float lmodel_oneside[] = {GL_FALSE};

void checkGlError()
{
	GLenum errCode;
	const GLubyte* errString;
	if ((errCode = glGetError()) != GL_NO_ERROR)
	{
		errString = gluErrorString(errCode);
		fprintf(stderr, "OpenGL Error: %s\n", errString);
	}
}

ArnMeshGl::ArnMeshGl(void)
: m_vboId(0)
, m_vboUv(0)
, m_target(0)
{
}

ArnMeshGl::~ArnMeshGl(void)
{
}

bool
ArnMeshGl::initRendererObjectVbIb()
{
	assert(m_target);
	assert(m_target->getVertexBuffer());
	glGenBuffersARB(1, &m_vboId);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vboId);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_target->getVertexBuffer()->getDataSize(), 0, GL_STATIC_DRAW_ARB);
	glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, m_target->getVertexBuffer()->getDataSize(), m_target->getVertexBuffer()->getData());
	return true;
}

struct ArnxVertexChunkEntry
{
	float x, y, z;			// vertex coordinates
	float nx, ny, nz;		// vertex normal coordinates
};

struct ArnxVboEntry
{
	// minimal fields
	ArnxVertexChunkEntry coords;
	// optional fields
	float u, v;				// UV texture coordinates
	int numInf;				// the number of bone influences, 0..4
	int m0, m1, m2, m3;		// bone matrix index. 0..
	float w0, w1, w2, w3;	// bone influences. 0.0f..1.0f
};

struct ArnxTriFaceChunkEntry
{
	int faceId;
	int v0, v1, v2;
};

struct ArnxQuadFaceChunkEntry
{
	int faceId;
	int v0, v1, v2, v3;
};

struct ArnxBoneRecord
{
	float numInf[4];			// Should be 'int numInf' but shader compatability
	float influences[4];
	float m[4];					// Should be 'int m[4]' but shader compatability
};

bool
ArnMeshGl::initRendererObjectXml()
{
	if (m_target->getFaceGroupCount() && m_target->getVertGroupCount())
	{
		/*

		'm_vboIds[i]' buffer structure
		-----------------------------------------------------------------
		| Vertex coordinates | Vertex normals | UV coordinates (if any) |
		--------------------------------------+--------------------------

		|<----- vertexRecordSize ------------>|<---- uvRecordSize ----->|

		|<----- vboEntrySize (if no UV) ----->|

		|<------------------ vboEntrySize (if UV) --------------------->|

		*/

		const ArnBinaryChunk* vertexChunk = m_target->getVertexChunk();
		const ArnBinaryChunk* triquadUvChunk = m_target->getTriquadUvChunk();
		const int vertexRecordSize = vertexChunk->getRecordSize(); // typically coords(float3) + normal(float3) = 24 bytes
		int uvRecordSize = 0;
		int boneRecordSize = 0;
		if (triquadUvChunk)
			uvRecordSize += sizeof(float) * 2; // Extend vertex size to include 2D UV coordinates.
		if (m_target->getVertGroupCount() > 1) // TODO: How to determine skinned meshes?
			boneRecordSize += sizeof(ArnxBoneRecord);
		int vboEntrySize = vertexRecordSize + uvRecordSize + boneRecordSize;

		const size_t faceGroupCount = m_target->getFaceGroupCount();
		m_vboIds.resize(faceGroupCount);
		glGenBuffersARB(faceGroupCount, &m_vboIds[0]);
		checkGlError();
		int totalTriFaceCount = 0;
		int totalQuadFaceCount = 0;
		for (size_t i = 0; i < faceGroupCount; ++i)
		{
			const ArnBinaryChunk* triFaceChunk = m_target->getTriFaceChunkOfFaceGroup(i);
			const ArnBinaryChunk* quadFaceChunk = m_target->getQuadFaceChunkOfFaceGroup(i);
			int triFaceCount = triFaceChunk->getRecordCount();
			int quadFaceCount = quadFaceChunk->getRecordCount();
			int triFaceVertSize = triFaceCount * 3 * vboEntrySize;
			int quadFaceVertSize = quadFaceCount * 4 * vboEntrySize;

			glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vboIds[i]);
			glBufferDataARB(GL_ARRAY_BUFFER_ARB, triFaceVertSize + quadFaceVertSize, 0, GL_STATIC_DRAW_ARB);

			// 1. tri face coordinates/normal/uv(if any)
			if (triFaceCount)
			{
				char* buf = new char[triFaceVertSize];
				int bufOffset = 0;
				const int* vert3Ind = reinterpret_cast<const int*>(triFaceChunk->getConstRawDataPtr()); // [face index][v0 index][v1 index][v2 index]
				for (int j = 0; j < triFaceCount; ++j)
				{
					for (int k = 0; k < 3; ++k)
					{
						const int* faceId = vert3Ind + j*(1+3);
						const int* vertexId = faceId + 1 + k; // Skip face index by adding 1 to vert3Ind ptr.
						const char* record = vertexChunk->getRecordAt(*vertexId);
						memcpy(&buf[bufOffset], record, vertexRecordSize);
						bufOffset += vertexRecordSize;
						if (triquadUvChunk)
						{
							assert(uvRecordSize);
							const char* quadrupleUvCoordsRecord = triquadUvChunk->getRecordAt(*faceId);
							quadrupleUvCoordsRecord += uvRecordSize * k;
							memcpy(&buf[bufOffset], quadrupleUvCoordsRecord, uvRecordSize);
							bufOffset += uvRecordSize;
						}
						if (boneRecordSize)
						{
							ArnxBoneRecord abr;
							int numInf;
							int m[4];
							m_target->computeBoneDataOfVert(*vertexId, &numInf, abr.influences, m);
							abr.numInf[0] = static_cast<float>(numInf);
							abr.numInf[1] = 0;
							abr.numInf[2] = 0;
							abr.numInf[3] = 0;
							abr.m[0] = static_cast<float>(m[0]);
							abr.m[1] = static_cast<float>(m[1]);
							abr.m[2] = static_cast<float>(m[2]);
							abr.m[3] = static_cast<float>(m[3]);
							memcpy(&buf[bufOffset], &abr, boneRecordSize);
							bufOffset += boneRecordSize;
						}
					}
				}
				assert(bufOffset == triFaceVertSize);
				glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, triFaceVertSize, buf);
				checkGlError();
				delete [] buf;
			}

			// 2. quad face coordinates/normal/uv(if any)
			if (quadFaceCount)
			{
				char* buf = new char[quadFaceVertSize];
				int bufOffset = 0;
				const int* vert4Ind = reinterpret_cast<const int*>(quadFaceChunk->getConstRawDataPtr());  // [face index][v0 index][v1 index][v2 index][v3 index]
				for (int j = 0; j < quadFaceCount; ++j)
				{
					for (int k = 0; k < 4; ++k)
					{
						const int* faceId = vert4Ind + j*(1+4);
						const int* vertexId = faceId + 1 + k; // Skip face index by adding 1 to vert4Ind ptr.
						const char* record = vertexChunk->getRecordAt(*vertexId);
						memcpy(&buf[bufOffset], record, vertexRecordSize);
						bufOffset += vertexRecordSize;
						if (triquadUvChunk)
						{
							assert(uvRecordSize);
							const char* quadrupleUvCoordsRecord = triquadUvChunk->getRecordAt(*faceId);
							quadrupleUvCoordsRecord += uvRecordSize * k;
							memcpy(&buf[bufOffset], quadrupleUvCoordsRecord, uvRecordSize);
							bufOffset += uvRecordSize;
						}
						if (boneRecordSize)
						{
							ArnxBoneRecord abr;
							int numInf;
							int m[4];
							m_target->computeBoneDataOfVert(*vertexId, &numInf, abr.influences, m);
							abr.numInf[0] = static_cast<float>(numInf);
							abr.numInf[1] = 0;
							abr.numInf[2] = 0;
							abr.numInf[3] = 0;
							abr.m[0] = static_cast<float>(m[0]);
							abr.m[1] = static_cast<float>(m[1]);
							abr.m[2] = static_cast<float>(m[2]);
							abr.m[3] = static_cast<float>(m[3]);
							memcpy(&buf[bufOffset], &abr, boneRecordSize);
							bufOffset += boneRecordSize;
							/*
							printf("%.1f %.1f %.1f %.1f --- %.1f %.1f %.1f %.1f --- %.1f %.1f %.1f %.1f\n",
								abr.numInf[0], abr.numInf[1], abr.numInf[2], abr.numInf[3],
								abr.influences[0], abr.influences[1], abr.influences[2], abr.influences[3],
								abr.m[0], abr.m[1], abr.m[2], abr.m[3] );
							*/
						}
					}
				}
				assert(bufOffset == quadFaceVertSize);
				glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, triFaceVertSize, quadFaceVertSize, buf);
				checkGlError();
				delete [] buf;
			}

			totalTriFaceCount += triFaceCount;
			totalQuadFaceCount += quadFaceCount;
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		}
		return true;
	}
	else
	{
		return false;
	}
}


void
ArnMeshGl::renderVbIb() const
{
	assert(m_vboId && m_target->getVertexBuffer());

	// bind VBOs with IDs and set the buffer offsets of the bound VBOs
	// When buffer object is bound with its ID, all pointers in gl*Pointer()
	// are treated as offset instead of real pointer.
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vboId);

	// enable vertex arrays
	glEnableClientState(GL_VERTEX_ARRAY);

	// before draw, specify vertex and index arrays with their offsets
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glPushMatrix();
	assert(m_target->isLocalXformDirty() == false);
	glMultMatrixf((float*)m_target->getLocalXform().transpose().m);
	glDrawArrays(GL_TRIANGLES, 0, m_target->getVertexBuffer()->getCount());
	glPopMatrix();
	glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays

	// it is good idea to release VBOs with ID 0 after use.
	// Once bound with 0, all pointers in gl*Pointer() behave as real
	// pointer, so, normal vertex array operations are re-activated
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

void
ArnMeshGl::renderXml(bool bIncludeShadeless) const
{
	assert(m_target);
	assert(m_target->getFaceGroupCount() && m_target->getVertGroupCount());

	/*

	'm_vboIds[i]' buffer structure
	-----------------------------------------------------------------
	| Vertex coordinates | Vertex normals | UV coordinates (if any) |
	--------------------------------------+--------------------------

	|<----- vertexRecordSize ------------>|<---- uvRecordSize ----->|

	|<----- vboEntrySize (if no UV) ----->|

	|<------------------ vboEntrySize (if UV) --------------------->|

	*/
	const ArnBinaryChunk* vertexChunk = m_target->getVertexChunk();
	const ArnBinaryChunk* triquadUvChunk = m_target->getTriquadUvChunk();
	const int vertexRecordSize = vertexChunk->getRecordSize(); // typically coords(float3) + normal(float3) = 24 bytes
	int uvRecordSize = 0;
	int boneRecordSize = 0;
	if (triquadUvChunk)
		uvRecordSize += sizeof(float) * 2; // Extend vertex size to include 2D UV coordinates.
	if (m_target->getVertGroupCount() > 1)
		boneRecordSize += sizeof(ArnxBoneRecord);
	int vboEntrySize = vertexRecordSize + uvRecordSize + boneRecordSize;

	if (boneRecordSize)
	{
		glUseProgramObjectARB(ArnSkinningShaderGl::getSingleton().getProgramObj());

		// TODO: How to locate the parent skeleton of a skinned mesh?
		ArnNode* skelParent = getParent();
		while (skelParent && skelParent->getType() != NDT_RT_SKELETON)
		{
			skelParent = skelParent->getParent();
		}
		assert(skelParent && skelParent->getType() == NDT_RT_SKELETON);
		ArnSkinningShaderGl::getSingleton().setShaderConstants(reinterpret_cast<ArnSkeleton*>(skelParent), m_target->getVertGroup(), m_target);
	}

	const size_t faceGroupCount = m_target->getFaceGroupCount();
	const int mtrlRefNameCount = static_cast<const int>(m_target->getMaterialReferenceNameCount());
	for (size_t i = 0; i < faceGroupCount; ++i)
	{
		int mtrlIndex = m_target->getMaterialIndexOfFaceGroup(i);
		if (mtrlIndex < mtrlRefNameCount)
		{
			const ArnNode* sceneRoot = m_target->getConstSceneRoot();
			const ArnNode* mtrlNode = sceneRoot->getConstNodeByName( m_target->getMaterialReferenceName(mtrlIndex) );
			const ArnMaterial* mtrl = dynamic_cast<const ArnMaterial*>(mtrlNode);
			assert(mtrl);
			if (mtrl->isShadeless() && !bIncludeShadeless)
			{
				continue;
			}
			ArnSetupMaterialGl(mtrl);
		}
		const ArnBinaryChunk* triFaceChunk = m_target->getTriFaceChunkOfFaceGroup(i);
		const ArnBinaryChunk* quadFaceChunk = m_target->getQuadFaceChunkOfFaceGroup(i);
		int triFaceCount = triFaceChunk->getRecordCount();
		int quadFaceCount = quadFaceChunk->getRecordCount();
		int triFaceVertSize = triFaceCount * 3 * vboEntrySize;
		//int quadFaceVertSize = quadFaceCount * 3 * vboEntrySize;

		// Skinning shader attributes placeholders setup
		const GLuint matIndAttr = ArnSkinningShaderGl::getSingleton().getMatrixIndicesAttrId();
		const GLuint weightsAttr = ArnSkinningShaderGl::getSingleton().getWeightsAttrId();
		const GLuint numBonesAttr = ArnSkinningShaderGl::getSingleton().getNumBonesAttrId();

		glBindBuffer(GL_ARRAY_BUFFER_ARB, m_vboIds[i]);

		if (boneRecordSize)
		{
			glEnableVertexAttribArrayARB(matIndAttr);
			glEnableVertexAttribArrayARB(weightsAttr);
			glEnableVertexAttribArrayARB(numBonesAttr);
		}
		if (triquadUvChunk)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);

		// No push, pop matrix needed. It is done in the previous call level.
		//glPushMatrix();

        glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT | GL_LINE_BIT | GL_COLOR_BUFFER_BIT);
		{
			assert(m_target->isLocalXformDirty() == false);

			glMultTransposeMatrixf((float*)m_target->getAutoLocalXform().m);

			/*
			if (m_target->isTwoSided())
			{
				glDisable(GL_CULL_FACE);
				//glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);
			}
			else
			{
				glEnable(GL_CULL_FACE);
				//glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_oneside);
			}
			*/
			glLoadName(getObjectId()); // For screenspace rendering based picking


			// 1. Draw tri faces
			if (triFaceCount)
			{
				if (boneRecordSize)
				{
					glVertexAttribPointer(matIndAttr, 4, GL_FLOAT, GL_FALSE, vboEntrySize, (void*)(vertexRecordSize + uvRecordSize + sizeof(float)*8));
					glVertexAttribPointer(weightsAttr, 4, GL_FLOAT, GL_FALSE, vboEntrySize, (void*)(vertexRecordSize + uvRecordSize + sizeof(float)*4));
					glVertexAttribPointer(numBonesAttr, 4, GL_FLOAT, GL_FALSE, vboEntrySize, (void*)(vertexRecordSize + uvRecordSize));
				}
				if (triquadUvChunk)
				{
					glTexCoordPointer(2, GL_FLOAT, vboEntrySize, (void*)(vertexRecordSize));
				}
				glNormalPointer(GL_FLOAT, vboEntrySize, (void*)(sizeof(float)*3));
				glVertexPointer(3, GL_FLOAT, vboEntrySize, 0);
				glDrawArrays(GL_TRIANGLES, 0, triFaceCount * 3);
			}

			// 2. Draw quad faces
			if (quadFaceCount)
			{
				if (boneRecordSize)
				{
					glVertexAttribPointer(matIndAttr, 4, GL_FLOAT, GL_FALSE, vboEntrySize, (void*)(vertexRecordSize + uvRecordSize + sizeof(float)*8 + triFaceVertSize));
					glVertexAttribPointer(weightsAttr, 4, GL_FLOAT, GL_FALSE, vboEntrySize, (void*)(vertexRecordSize + uvRecordSize + sizeof(float)*4 + triFaceVertSize));
					glVertexAttribPointer(numBonesAttr, 4, GL_FLOAT, GL_FALSE, vboEntrySize, (void*)(vertexRecordSize + uvRecordSize + triFaceVertSize));
				}
				if (triquadUvChunk)
				{
					glTexCoordPointer(2, GL_FLOAT, vboEntrySize, (void*)(vertexRecordSize + triFaceVertSize));
				}
				glNormalPointer(GL_FLOAT, vboEntrySize, (void*)((sizeof(float)*3 + triFaceVertSize)));
				glVertexPointer(3, GL_FLOAT, vboEntrySize, (void*)(0 + triFaceVertSize));
				glDrawArrays(GL_QUADS, 0, quadFaceCount * 4);
			}
			// Render anchor point if there is a rigid body joint constraint.
			foreach (const ArnJointData& ajd, m_target->getJointData())
			{
				glLoadName(0);
				glPushMatrix();
				glTranslated(ajd.pivot.x, ajd.pivot.y, ajd.pivot.z);
				ArnRenderSphereGl(0.01);
				glPopMatrix();
			}
			// Bounding Box
			if (m_target->isOkayToRenderBoundingBox())
			{
				glLoadName(0); // Bounding box portion is not selectable.
				glDisable(GL_LIGHTING);
				glDisable(GL_CULL_FACE);
				glLineWidth(0.5f);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(1.0f, 1.0f, 1.0f);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBindTexture(GL_TEXTURE_2D, 0);
				ArnRenderBoundingBox(m_target->getBoundingBoxPoints());
			}
		}
		glPopAttrib();

		//glPopMatrix();

		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		if (triquadUvChunk)
		{
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		if (boneRecordSize)
		{
			glDisableVertexAttribArrayARB(matIndAttr);
			glDisableVertexAttribArrayARB(weightsAttr);
			glDisableVertexAttribArrayARB(numBonesAttr);
		}

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}

	if (boneRecordSize)
		glUseProgramObjectARB(0);
}

int ArnMeshGl::render(bool bIncludeShadeless) const
{
	renderXml(bIncludeShadeless);
	return 0;
}

void ArnMeshGl::cleanup()
{
}

ArnMeshGl* ArnMeshGl::createFrom( const ArnMesh* mesh )
{
	ArnMeshGl* ret = new ArnMeshGl();
	ret->m_target = mesh;
	if (mesh->getVertexBuffer())
	{
		assert(mesh->getVertexChunk() == 0); // Should not have both property between VB/IB and XML loaded data.
		ret->initRendererObjectVbIb();
	}
	else if (mesh->getVertexChunk())
	{
		assert(mesh->getVertexBuffer() == 0); // Should not have both property between VB/IB and XML loaded data.
		ret->initRendererObjectXml();
	}
	else
	{
		delete ret;
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}
	ret->setInitialized(true);
	ret->setRendererType(RENDERER_GL);
	return ret;
}
