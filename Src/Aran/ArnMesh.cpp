#include "AranPCH.h"
#include "ArnMesh.h"
#include "ArnMaterial.h"
#include "VideoMan.h"
#include "ArnVertexBuffer.h"
#include "ArnBinaryChunk.h"
#include "ArnTexture.h"
#include "ArnMath.h"
#include "ArnRenderableObject.h"

ArnMesh::ArnMesh()
: ArnXformable(NDT_RT_MESH)
, m_bVisible(true)
, m_bCollide(true)
, m_skeleton(0)
, m_arnVb(0)
, m_arnIb(0)
, m_triquadUvChunk(0)
, m_bTwoSided(false)
, m_nodeMesh3(0)
, m_vertexChunk(0)
, m_bBoundingBoxPointsDirty(true)
, m_bRenderBoundingBox(true)
, m_abbt(ABBT_UNKNOWN)
, m_mass(0)
{
	memset(&m_boundingBoxPoints[0], 0, sizeof(m_boundingBoxPoints));
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
	delete m_vertexChunk;
}

ArnMesh*
ArnMesh::createFromVbIb( const ArnVertexBuffer* vb, const ArnIndexBuffer* ib )
{
	ArnMesh* ret = new ArnMesh();
	ret->setVertexBuffer(vb);
	ret->setIndexBuffer(ib);
	//ret->m_renderFunc = &ArnMesh::renderVbIb;
	//ret->m_initRendererObjectFunc = &ArnMesh::initRendererObjectVbIb;
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

void
ArnMesh::render()
{
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
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
	const unsigned int* vert3Ind = reinterpret_cast<const unsigned int*>(triFaceChunk->getConstRawDataPtr()) + (1+3)*triFaceIndex; // [face index][v0 index][v1 index][v2 index]
	faceIdx = vert3Ind[0];
	vind[0] = vert3Ind[1];
	vind[1] = vert3Ind[2];
	vind[2] = vert3Ind[3];
}

void ArnMesh::getQuadFace( unsigned int& faceIdx, unsigned int vind[4], unsigned int faceGroupIdx, unsigned int quadFaceIndex ) const
{
	ArnBinaryChunk* quadFaceChunk = m_faceGroup[faceGroupIdx].quadFaceChunk;
	const unsigned int* vert4Ind = reinterpret_cast<const unsigned int*>(quadFaceChunk->getConstRawDataPtr()) + (1+4)*quadFaceIndex; // [face index][v0 index][v1 index][v2 index][v3 index]
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
	memcpy(&m_boundingBoxPoints[0], bb, sizeof(m_boundingBoxPoints));
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
