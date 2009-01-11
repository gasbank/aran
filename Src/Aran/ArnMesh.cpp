#include "AranPCH.h"
#include "ArnMesh.h"
#include "ArnMaterial.h"
#include "ArnFile.h"
#include "VideoMan.h"
#include "ArnSkeleton.h"

ArnMesh::ArnMesh()
: ArnXformable(NDT_RT_MESH), m_d3dxMesh(0), m_bVisible(true), m_bCollide(true)
{
}

ArnMesh::~ArnMesh(void)
{
	SAFE_RELEASE(m_d3dxMesh);
}

ArnNode* ArnMesh::createFrom( const NodeBase* nodeBase )
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

void ArnMesh::buildFrom(const NodeMesh2* nm)
{
	m_data.vertexCount		= nm->m_meshVerticesCount;
	m_data.faceCount		= nm->m_meshFacesCount;
	m_data.materialCount	= nm->m_materialCount;

	if (VideoMan::getSingletonPtr())
	{
		LPD3DXMESH d3dxMesh;
		arn_build_mesh(VideoMan::getSingleton().GetDev(), nm, d3dxMesh);
		setD3DXMesh(d3dxMesh);
	}
}

void ArnMesh::buildFrom(const NodeMesh3* nm)
{
	unsigned int i, j, k;
	m_data.vertexCount		= nm->m_meshVerticesCount;
	m_data.faceCount		= nm->m_meshFacesCount;
	m_data.materialCount	= nm->m_materialCount;
	for (i = 0; i < m_data.materialCount; ++i)
		m_data.matNameList.push_back(nm->m_matNameList[i]);

	setParentName(nm->m_parentName);
	setIpoName(nm->m_ipoName);
	setLocalXform(*nm->m_localXform);
	if (VideoMan::getSingletonPtr())
	{
		LPD3DXMESH d3dxMesh;
		arn_build_mesh(VideoMan::getSingleton().GetDev(), nm, d3dxMesh);
		setD3DXMesh(d3dxMesh);
	}

	if (nm->m_armatureName)
	{
		m_data.armatureName = nm->m_armatureName;
		const unsigned boneCount = nm->m_bones.size();
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

void ArnMesh::interconnect( ArnNode* sceneRoot )
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
	
	if (m_data.armatureName.length())
	{
		m_skeleton = dynamic_cast<ArnSkeleton*>(sceneRoot->getNodeByName(m_data.armatureName));
		assert(m_skeleton);
	}

	ArnNode::interconnect(sceneRoot);
}

const D3DMATERIAL9* ArnMesh::getMaterial( unsigned int i ) const
{
	return &m_materialRefList[i]->getD3DMaterialData();
}
//////////////////////////////////////////////////////////////////////////


HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh2* nm, OUT LPD3DXMESH& mesh)
{
	if (!dev)
		throw MyError(MEE_DEVICE_NOT_READY);

	HRESULT hr = 0;
	ArnVertex* v = 0;
	WORD* ind = 0;
	hr = D3DXCreateMeshFVF(nm->m_meshFacesCount, nm->m_meshVerticesCount, D3DXMESH_MANAGED, ARN_VDD::ARN_VDD_FVF, dev, &mesh);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	mesh->LockVertexBuffer(0, (void**)&v);
	memcpy(v, nm->m_vdd, nm->m_meshVerticesCount * sizeof(ARN_VDD));
	mesh->UnlockVertexBuffer();

	mesh->LockIndexBuffer(0, (void**)&ind);
	unsigned int i;
	for (i = 0; i < nm->m_meshFacesCount * 3; ++i)
	{
		assert((nm->m_triangleIndice[i] & 0xffff0000) == 0);
		ind[i] = (WORD)nm->m_triangleIndice[i];
	}
	mesh->UnlockIndexBuffer();

	DWORD* attrBuf = 0;
	mesh->LockAttributeBuffer(0, &attrBuf);
	memcpy(attrBuf, nm->m_materialRefs, nm->m_meshFacesCount * sizeof(DWORD));
	mesh->UnlockAttributeBuffer();

	return S_OK;
}

HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh3* nm, OUT LPD3DXMESH& mesh)
{
	if (!dev)
		throw MyError(MEE_DEVICE_NOT_READY);

	HRESULT hr = 0;
	ArnVertex* v = 0;
	WORD* ind = 0;
	hr = D3DXCreateMeshFVF(nm->m_meshFacesCount, nm->m_meshVerticesCount, D3DXMESH_MANAGED, ArnVertex::FVF, dev, &mesh);
	if (FAILED(hr))
	{
		DebugBreak();
		return E_FAIL;
	}

	mesh->LockVertexBuffer(0, (void**)&v);
	memcpy(v, nm->m_vertex, nm->m_meshVerticesCount * sizeof(ArnVertex));
	mesh->UnlockVertexBuffer();

	mesh->LockIndexBuffer(0, (void**)&ind);
	memcpy(ind, nm->m_faces, nm->m_meshFacesCount * 3 * sizeof(WORD));
	mesh->UnlockIndexBuffer();

	DWORD* attrBuf = 0;
	mesh->LockAttributeBuffer(0, &attrBuf);
	memcpy(attrBuf, nm->m_attr, nm->m_meshFacesCount * sizeof(DWORD));
	mesh->UnlockAttributeBuffer();

	return S_OK;
}