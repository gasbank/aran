#include "AranDx9PCH.h"
#include "ArnMeshDx9.h"

ArnMeshDx9::ArnMeshDx9(void)
: m_d3dxMesh(0)
{
}

ArnMeshDx9::~ArnMeshDx9(void)
{
	SAFE_DELETE(m_d3dxMesh);
}

ArnMeshDx9*
ArnMeshDx9::createFrom( const NodeBase* nodeBase )
{
	ArnMesh* node = new ArnMesh();
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
}

void ArnMeshDx9::interconnect( ArnNode* sceneRoot )
{
	if (m_data.armatureName.length())
	{
		m_skeleton = dynamic_cast<ArnHierarchy*>(sceneRoot->getNodeByName(m_data.armatureName));
		assert(m_skeleton);
	}
	ArnMesh::interconnect(sceneRoot);
}
void
ArnMesh::buildFrom(const NodeMesh2* nm)
{
	m_data.vertexCount		= nm->m_meshVerticesCount;
	m_data.faceCount		= nm->m_meshFacesCount;
	m_data.materialCount	= nm->m_materialCount;

	if (VideoMan::getSingletonPtr())
	{
		LPD3DXMESH d3dxMesh;
		arn_build_mesh(VideoMan::getSingleton().GetDev(), nm, d3dxMesh);
		m_d3dxMesh = d3dxMesh;
	}
}

void
ArnMesh::buildFrom(const NodeMesh3* nm)
{
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
}
