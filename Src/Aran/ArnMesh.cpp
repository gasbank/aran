#include "AranPCH.h"
#include "ArnMesh.h"
#include "ArnMaterial.h"
#include "VideoMan.h"
#include "ArnVertexBuffer.h"
#include "ArnBinaryChunk.h"
#include "ArnTexture.h"
#include "ArnMath.h"

static float lmodel_twoside[] = {GL_TRUE};
static float lmodel_oneside[] = {GL_FALSE};

ArnMesh::ArnMesh()
: ArnXformable(NDT_RT_MESH)
, m_bVisible(true)
, m_bCollide(true)
, m_skeleton(0)
, m_arnVb(0)
, m_arnIb(0)
, m_triquadUvChunk(0)
, m_bTwoSided(false)
, m_vboId(0)
, m_vboUv(0)
, m_renderFunc(0)
, m_initRendererObjectFunc(0)
, m_nodeMesh3(0)
, m_vertexChunk(0)
, m_bBoundingBoxPointsDirty(true)
, m_bRenderBoundingBox(true)
{
	memset(m_boundingBoxPoints, 0, sizeof(ArnVec3) * 8);
}

ArnMesh::~ArnMesh(void)
{
	foreach (const FaceGroup& fg, m_faceGroup)
	{
		delete fg.triFaceChunk;
		delete fg.quadFaceChunk;
	}
	foreach (const VertexGroup& vg, m_vertexGroup)
	{
		delete vg.vertGroupChunk;
	}
	delete m_triquadUvChunk;
}

ArnMesh*
ArnMesh::createFromVbIb( const ArnVertexBuffer* vb, const ArnIndexBuffer* ib )
{
	ArnMesh* ret = new ArnMesh();
	ret->setVertexBuffer(vb);
	ret->setIndexBuffer(ib);
	ret->m_renderFunc = &ArnMesh::renderVbIb;
	ret->m_initRendererObjectFunc = &ArnMesh::initRendererObjectVbIb;
	return ret;
}

void
ArnMesh::interconnect( ArnNode* sceneRoot )
{
	unsigned int i;
	for (i = 0; i < m_data.matNameList.size(); ++i)
	{
		ArnNode* matNode = sceneRoot->getNodeByName(m_data.matNameList[i]);
		if (matNode && matNode->getType() == NDT_RT_MATERIAL)
		{
			ArnMaterial* mat = reinterpret_cast<ArnMaterial*>(matNode);
			mat->loadTexture();
			m_materialRefList.push_back(reinterpret_cast<ArnMaterial*>(matNode));
		}
		else
			throw MyError(MEE_RTTI_INCONSISTENCY);
	}
	setIpo(getIpoName());
	configureAnimCtrl();


	ArnNode::interconnect(sceneRoot);
}

const ArnMaterialData*
ArnMesh::getMaterial( unsigned int i ) const
{
	return &m_materialRefList[i]->getD3DMaterialData();
}

void
ArnMesh::setVertexBuffer( const ArnVertexBuffer* vb )
{
	if (m_arnVb)
	{
		// TODO: Deallocate existing vertex buffer
		abort();
	}
	else
	{
		m_arnVb = vb;

		//glGenBuffersARB()
	}

}

void
ArnMesh::setIndexBuffer( const ArnIndexBuffer* ib )
{
	m_arnIb = ib;
}

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

bool
ArnMesh::initRendererObject()
{
	bool ret = false;
	if (m_initRendererObjectFunc)
	{
		ret = (this->*m_initRendererObjectFunc)();
	}
	return ret;
}

bool
ArnMesh::initRendererObjectVbIb()
{
	if (m_arnVb)
	{
		glGenBuffersARB(1, &m_vboId);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vboId);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_arnVb->getDataSize(), 0, GL_STATIC_DRAW_ARB);
		glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, m_arnVb->getDataSize(), m_arnVb->getData());
		return true;
	}
	else
	{
		return false;
	}
}

bool
ArnMesh::initRendererObjectXml()
{
	if (m_faceGroup.size() && m_vertexGroup.size())
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

		const ArnBinaryChunk* vertexChunk = m_vertexChunk;
		const int vertexRecordSize = vertexChunk->getRecordSize(); // typically coords(float3) + normal(float3) = 24 bytes
		int uvRecordSize = 0;
		if (m_triquadUvChunk)
			uvRecordSize += sizeof(float) * 2; // Extend vertex size to include 2D UV coordinates.
		int vboEntrySize = vertexRecordSize + uvRecordSize;

		const size_t faceGroupCount = getFaceGroupCount();
		m_vboIds.resize(faceGroupCount);
		glGenBuffersARB(faceGroupCount, &m_vboIds[0]);
		checkGlError();
		int totalTriFaceCount = 0;
		int totalQuadFaceCount = 0;
		for (size_t i = 0; i < faceGroupCount; ++i)
		{
			ArnBinaryChunk* triFaceChunk = m_faceGroup[i].triFaceChunk;
			ArnBinaryChunk* quadFaceChunk = m_faceGroup[i].quadFaceChunk;
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
				int* vert3Ind = (int*)triFaceChunk->getRawDataPtr(); // [face index][v0 index][v1 index][v2 index]
				for (int j = 0; j < triFaceCount; ++j)
				{
					for (int k = 0; k < 3; ++k)
					{
						const char* record = vertexChunk->getRecordAt( *(vert3Ind + j*(1+3) + k + 1) ); // Skip face index by adding 1 to vert3Ind ptr.
						memcpy(&buf[bufOffset], record, vertexRecordSize);
						bufOffset += vertexRecordSize;
						if (m_triquadUvChunk)
						{
							assert(uvRecordSize);
							const char* quadrupleUvCoordsRecord = m_triquadUvChunk->getRecordAt( *(vert3Ind + j*(1+3)) );
							quadrupleUvCoordsRecord += uvRecordSize * k;
							memcpy(&buf[bufOffset], quadrupleUvCoordsRecord, uvRecordSize);
							bufOffset += uvRecordSize;
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
				int* vert4Ind = (int*)quadFaceChunk->getRawDataPtr();  // [face index][v0 index][v1 index][v2 index][v3 index]
				for (int j = 0; j < quadFaceCount; ++j)
				{
					for (int k = 0; k < 4; ++k)
					{
						const char* record = vertexChunk->getRecordAt( *(vert4Ind + j*(1+4) + k + 1) ); // Skip face index by adding 1 to vert3Ind ptr.
						memcpy(&buf[bufOffset], record, vertexRecordSize);
						bufOffset += vertexRecordSize;
						if (m_triquadUvChunk)
						{
							assert(uvRecordSize);
							const char* quadrupleUvCoordsRecord = m_triquadUvChunk->getRecordAt( *(vert4Ind + j*(1+4)) );
							quadrupleUvCoordsRecord += uvRecordSize * k;
							memcpy(&buf[bufOffset], quadrupleUvCoordsRecord, uvRecordSize);
							bufOffset += uvRecordSize;
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
ArnMesh::render()
{
	if (m_renderFunc)
	{
		(this->*m_renderFunc)();
	}
}

void
ArnMesh::renderVbIb()
{
	assert(m_vboId && m_arnVb);

	// bind VBOs with IDs and set the buffer offsets of the bound VBOs
	// When buffer object is bound with its ID, all pointers in gl*Pointer()
	// are treated as offset instead of real pointer.
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vboId);

	// enable vertex arrays
	glEnableClientState(GL_VERTEX_ARRAY);

	// before draw, specify vertex and index arrays with their offsets
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glPushMatrix();
	recalcLocalXform(); // TODO: Is this necessary? -- maybe yes...
#ifdef WIN32
	glMultMatrixf((float*)getLocalXform().m);
#else
	glMultMatrixf((float*)getLocalXform().transpose().m);
#endif
	glDrawArrays(GL_TRIANGLES, 0, m_arnVb->getCount());
	glPopMatrix();
	glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays

	// it is good idea to release VBOs with ID 0 after use.
	// Once bound with 0, all pointers in gl*Pointer() behave as real
	// pointer, so, normal vertex array operations are re-activated
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

void
ArnMesh::renderXml()
{
	assert(m_faceGroup.size() && m_vertexGroup.size());

	/*

	'm_vboIds[i]' buffer structure
	-----------------------------------------------------------------
	| Vertex coordinates | Vertex normals | UV coordinates (if any) |
	--------------------------------------+--------------------------

	|<----- vertexRecordSize ------------>|<---- uvRecordSize ----->|

	|<----- vboEntrySize (if no UV) ----->|

	|<------------------ vboEntrySize (if UV) --------------------->|

	*/

	//const ArnBinaryChunk* vertexChunk = m_vertexGroup[0].vertexChunk;
	const ArnBinaryChunk* vertexChunk = m_vertexChunk;
	const int vertexRecordSize = vertexChunk->getRecordSize(); // typically coords(float3) + normal(float3) = 24 bytes
	int uvRecordSize = 0;
	if (m_triquadUvChunk)
		uvRecordSize += sizeof(float) * 2; // Extend vertex size to include 2D UV coordinates.
	int vboEntrySize = vertexRecordSize + uvRecordSize;

	const size_t faceGroupCount = m_faceGroup.size();
	for (size_t i = 0; i < faceGroupCount; ++i)
	{
		if (m_faceGroup[i].mtrlIndex < (int)m_mtrlRefNameList.size())
		{
			ArnNode* sceneRoot = getSceneRoot();
			ArnNode* mtrlNode = sceneRoot->getNodeByName(m_mtrlRefNameList[ m_faceGroup[i].mtrlIndex ]);
			ArnMaterial* mtrl = dynamic_cast<ArnMaterial*>(mtrlNode);
			assert(mtrl);
			ArnSetupMaterialGl(mtrl);
		}
		const ArnBinaryChunk* triFaceChunk = m_faceGroup[i].triFaceChunk;
		const ArnBinaryChunk* quadFaceChunk = m_faceGroup[i].quadFaceChunk;
		int triFaceCount = triFaceChunk->getRecordCount();
		int quadFaceCount = quadFaceChunk->getRecordCount();
		int triFaceVertSize = triFaceCount * 3 * vboEntrySize;
		//int quadFaceVertSize = quadFaceCount * 3 * vboEntrySize;

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vboIds[i]);

		if (m_triquadUvChunk)
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);

		glPushMatrix();
		glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT | GL_LINE_BIT);
		{
			recalcLocalXform(); // TODO: Is this necessary? -- maybe yes...
			glMultTransposeMatrixf((float*)getFinalLocalXform().m);
			if (m_bTwoSided)
			{
				glDisable(GL_CULL_FACE);
				//glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);
			}
			else
			{
				glEnable(GL_CULL_FACE);
				//glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_oneside);
			}
			glLoadName(getObjectId()); // For screenspace rendering based picking
			// 1. Draw tri faces
			if (triFaceCount)
			{
				if (m_triquadUvChunk)
					glTexCoordPointer(2, GL_FLOAT, vboEntrySize, (void*)(sizeof(float)*6));
				glNormalPointer(GL_FLOAT, vboEntrySize, (void*)(sizeof(float)*3));
				glVertexPointer(3, GL_FLOAT, vboEntrySize, 0);
				glDrawArrays(GL_TRIANGLES, 0, triFaceCount * 3);
			}

			// 2. Draw quad faces
			if (quadFaceCount)
			{
				if (m_triquadUvChunk)
					glTexCoordPointer(2, GL_FLOAT, vboEntrySize, (void*)(sizeof(float)*6 + triFaceVertSize));
				glNormalPointer(GL_FLOAT, vboEntrySize, (void*)((sizeof(float)*3 + triFaceVertSize)));
				glVertexPointer(3, GL_FLOAT, vboEntrySize, (void*)(0 + triFaceVertSize));
				glDrawArrays(GL_QUADS, 0, quadFaceCount * 4);
			}

			// Bounding Box
			if (!m_bBoundingBoxPointsDirty && m_bRenderBoundingBox)
			{
				glDisable(GL_LIGHTING);
				glDisable(GL_CULL_FACE);
				glLineWidth(0.5f);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(1.0f, 1.0f, 1.0f);

				glBegin(GL_QUADS);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[0]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[1]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[2]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[3]);
				
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[0]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[4]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[5]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[1]);

				glVertex3fv((GLfloat*)&m_boundingBoxPoints[0]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[3]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[7]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[4]);

				glVertex3fv((GLfloat*)&m_boundingBoxPoints[7]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[6]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[5]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[4]);

				glVertex3fv((GLfloat*)&m_boundingBoxPoints[7]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[3]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[2]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[6]);

				glVertex3fv((GLfloat*)&m_boundingBoxPoints[6]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[2]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[1]);
				glVertex3fv((GLfloat*)&m_boundingBoxPoints[5]);				
				glEnd();
			}
		}
		glPopAttrib();
		glPopMatrix();

		if (m_triquadUvChunk)
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}

}

unsigned int
ArnMesh::getFaceCount( unsigned int& triCount, unsigned int& quadCount, unsigned int faceGroupIdx ) const
{
	ArnBinaryChunk* triFaceChunk = m_faceGroup[faceGroupIdx].triFaceChunk;
	ArnBinaryChunk* quadFaceChunk = m_faceGroup[faceGroupIdx].quadFaceChunk;
	triCount = triFaceChunk->getRecordCount();
	quadCount = quadFaceChunk->getRecordCount();
	return (triCount + quadCount);
}

void ArnMesh::getTriFace( unsigned int& faceIdx, unsigned int vind[3], unsigned int faceGroupIdx, unsigned int triFaceIndex ) const
{
	ArnBinaryChunk* triFaceChunk = m_faceGroup[faceGroupIdx].triFaceChunk;
	const unsigned int* vert3Ind = reinterpret_cast<const unsigned int*>(triFaceChunk->getRawDataPtr()) + (1+3)*triFaceIndex; // [face index][v0 index][v1 index][v2 index]
	faceIdx = vert3Ind[0];
	vind[0] = vert3Ind[1];
	vind[1] = vert3Ind[2];
	vind[2] = vert3Ind[3];
}

void ArnMesh::getQuadFace( unsigned int& faceIdx, unsigned int vind[4], unsigned int faceGroupIdx, unsigned int quadFaceIndex ) const
{
	ArnBinaryChunk* quadFaceChunk = m_faceGroup[faceGroupIdx].quadFaceChunk;
	const unsigned int* vert4Ind = reinterpret_cast<const unsigned int*>(quadFaceChunk->getRawDataPtr()) + (1+4)*quadFaceIndex; // [face index][v0 index][v1 index][v2 index][v3 index]
	faceIdx = vert4Ind[0];
	vind[0] = vert4Ind[1];
	vind[1] = vert4Ind[2];
	vind[2] = vert4Ind[3];
	vind[3] = vert4Ind[4];
}

void ArnMesh::getVert( ArnVec3* pos, ArnVec3* nor, ArnVec3* uv, unsigned int vertIdx, bool finalXformed ) const
{
	assert(pos || nor);
	struct PosNor { ArnVec3 pos; ArnVec3 nor; };
	const PosNor* posnor = reinterpret_cast<const PosNor*>(m_vertexChunk->getRecordAt(vertIdx));
	if (pos)
	{
		if (finalXformed)
			ArnVec3TransformCoord(pos, &posnor->pos, &getFinalLocalXform());
		else
			*pos = posnor->pos;
	}
	if (nor)
	{
		if (finalXformed)
			ArnVec3TransformNormal(nor, &posnor->nor, &getFinalLocalXform());
		else
			*nor = posnor->nor;
	}
}

unsigned int ArnMesh::getVertCountOfVertGroup( unsigned int vertGroupIdx ) const
{
	if (vertGroupIdx == 0)
	{
		return m_vertexChunk->getRecordCount();
	}
	else
	{
		ARN_THROW_NOT_IMPLEMENTED_ERROR
	}
}

void ArnMesh::setBoundingBoxPoints( ArnVec3 bb[8] )
{
	memcpy(m_boundingBoxPoints, bb, sizeof(ArnVec3) * 8 );
	m_bBoundingBoxPointsDirty = false;
}

void ArnMesh::getBoundingBoxDimension( ArnVec3* out, bool worldSpace ) const
{
	assert(!m_bBoundingBoxPointsDirty);
	ArnVec3DimensionFromBounds(out, m_boundingBoxPoints);
	if (worldSpace)
	{
		// TODO: Animated scaling does not effect the bounding box dimension.
		//       (It does effect the rendering of bounding box, though.)
		out->x *= getLocalXform_Scale().x;
		out->y *= getLocalXform_Scale().y;
		out->z *= getLocalXform_Scale().z;
	}
}
