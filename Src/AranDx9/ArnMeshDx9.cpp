#include "AranDx9PCH.h"
#include "ArnMeshDx9.h"
#include "AranDx9.h"
#include "VideoManDx9.h"

ArnMeshDx9::ArnMeshDx9(void)
: m_d3dxMesh(0)
{
}

ArnMeshDx9::~ArnMeshDx9(void)
{
	SAFE_RELEASE(m_d3dxMesh);
}

ArnMeshDx9*
ArnMeshDx9::createFrom( const NodeBase* nodeBase )
{
	/*
	ArnMeshDx9* node = new ArnMeshDx9();
	node->setName(nodeBase->m_nodeName);
	try
	{
		switch (nodeBase->m_ndt)
		{
		case NDT_MESH2:
			node->buildFrom(static_cast<const NodeMesh2*>(nodeBase));
			break;
		case NDT_MESH3:
			node->buildFrom(static_cast<const NodeMesh3*>(nodeBase));
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
	*/
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
}

ArnMeshDx9 *ArnMeshDx9::createFrom (const ArnMesh* mesh)
{
	ArnMeshDx9 *ret = new ArnMeshDx9 ();
	/*
	 * Should set the following members of NodeMesh3:
	 *
	 *   - m_armatureName to NULL
	 *   - m_meshFacesCount
	 *   - m_meshVerticesCount
	 *   - m_vertex
	 *   - m_faces
	 *   - m_attr
	 */
	NodeMesh3 nm3;
	nm3.m_armatureName = 0;

	nm3.m_meshVerticesCount = mesh->getVertCountOfVertGroup (0);
	ArnVertex *vertex_mem = new ArnVertex[nm3.m_meshVerticesCount];
	for (unsigned int v = 0; v < nm3.m_meshVerticesCount; ++v)
	{
		ArnVec3 pos, nor, uv;
		mesh->getVert (&pos, &nor, &uv, v, false);
		ArnVertex vert;
		vert.x = pos.x;		vert.y = pos.y;		vert.z = pos.z;
		vert.nx = nor.x;	vert.ny = nor.y;	vert.nz = nor.z;
		vert.u = uv.x;		vert.v = uv.y;
		vertex_mem[v] = vert;
	}
	nm3.m_vertex = vertex_mem;

	std::vector <unsigned short> faces;
	std::vector <DWORD> attr;
	unsigned int fgCount = mesh->getFaceGroupCount ();
	for (unsigned int fg = 0; fg < fgCount; ++fg)
	{
		unsigned int triCount, quadCount;
		mesh->getFaceCount (triCount, quadCount, fg);
		assert (quadCount == 0); // Not supporting quad faces yet.

		for (unsigned int t = 0; t < triCount; ++t)
		{
			unsigned int faceIdx;
			unsigned int vind[3];
			mesh->getTriFace (faceIdx, vind, fg, t);
			for (int i = 0; i < 3; ++i)
			{
				assert (vind[i] < 0x0000ffff);
				faces.push_back (vind[i]);
			}
			attr.push_back (fg);
		}
	}
	unsigned short *faces_mem = new unsigned short[faces.size ()];
	memcpy (faces_mem, &faces[0], sizeof (unsigned short) * faces.size () );
	nm3.m_faces = faces_mem;
	assert (faces.size () % 3 == 0);
	nm3.m_meshFacesCount = faces.size () / 3;
	assert (nm3.m_meshFacesCount == attr.size ());
	DWORD *attr_mem = new DWORD[nm3.m_meshFacesCount];
	memcpy (attr_mem, &attr[0], sizeof (DWORD) * attr.size ());
	nm3.m_attr = attr_mem;
	
	if (arn_build_mesh (GetVideoManagerDx9 ().GetDev (), &nm3, ret->m_d3dxMesh) < 0)
	{
		std::cerr << "arn_build_mesh() returns error. Mesh creation failed on " << mesh->getName() << "." << std::endl;
		delete ret;
		return 0;
	}
	else
	{
		return ret;
	}
}

void ArnMeshDx9::interconnect( ArnNode* sceneRoot )
{
	/*
	if (m_data.armatureName.length())
	{
		m_skeleton = dynamic_cast<ArnHierarchy*>(sceneRoot->getNodeByName(m_data.armatureName));
		assert(m_skeleton);
	}
	ArnMesh::interconnect(sceneRoot);
	*/
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
}
void
ArnMeshDx9::buildFrom(const NodeMesh2* nm)
{
	/*
	m_data.vertexCount		= nm->m_meshVerticesCount;
	m_data.faceCount		= nm->m_meshFacesCount;
	m_data.materialCount	= nm->m_materialCount;

	if (VideoMan::getSingletonPtr())
	{
		LPD3DXMESH d3dxMesh;
		arn_build_mesh(VideoMan::getSingleton().GetDev(), nm, d3dxMesh);
		m_d3dxMesh = d3dxMesh;
	}
	*/
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
}

void
ArnMeshDx9::buildFrom(const NodeMesh3* nm)
{
	/*
	unsigned int i, j, k;
	m_data.vertexCount		= nm->m_meshVerticesCount;
	m_data.faceCount		= nm->m_meshFacesCount;
	m_data.materialCount	= nm->m_materialCount;
	for (i = 0; i < m_data.materialCount; ++i)
		m_data.matNameList.push_back(nm->m_matNameList[i]);

	m_nodeMesh3 = nm;

	setParentName(nm->m_parentName);
	setIpoName(nm->m_ipoName);
	setLocalXform(*nm->m_localXform);
	if (VideoMan::getSingletonPtr())
	{
		if (nm->m_armatureName)
		{
			arn_build_mesh(VideoMan::getSingleton().GetDev(), nm, m_d3dvb, m_d3dib);
		}
		else
		{
			LPD3DXMESH d3dxMesh;
			arn_build_mesh(VideoMan::getSingleton().GetDev(), nm, d3dxMesh);
			m_d3dxMesh = d3dxMesh;
		}
	}

	if (nm->m_armatureName)
	{
		m_data.armatureName = nm->m_armatureName;

		const unsigned boneCount = nm->m_bones.size();
		const unsigned idxMapCount = nm->m_boneMatIdxMap.size();
		assert(boneCount == idxMapCount);
		for (j = 0; j < idxMapCount; ++j)
			m_data.boneMatIdxMap.push_back(nm->m_boneMatIdxMap[j]);

		// Data copied to 'm_boneDataInt' are not used(redundant) basically.
		// Bone weights per vertex are included in vertex declaration.
		m_boneDataInt.resize(boneCount);
		for (j = 0; j < boneCount; ++j)
		{
			m_boneDataInt[j].nameFixed = nm->m_bones[j].boneName;
			assert(nm->m_bones[j].indWeightCount);
			m_boneDataInt[j].indWeight.resize(nm->m_bones[j].indWeightCount);
			for (k = 0; k < nm->m_bones[j].indWeightCount; ++k)
			{
				m_boneDataInt[j].indWeight[k].first = nm->m_bones[j].indWeight[k].ind;
				m_boneDataInt[j].indWeight[k].second = nm->m_bones[j].indWeight[k].weight;
			}
		}
	}
	*/
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
}

int ArnMeshDx9::render( bool bIncludeShadeless ) const
{
	assert (m_d3dxMesh);
	return m_d3dxMesh->DrawSubset (0);
}

void ArnMeshDx9::cleanup()
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}