#include "AranDx9PCH.h"
#include "ArnMeshDx9.h"
#include "AranDx9.h"
#include "VideoManDx9.h"
#include "ArnBinaryChunk.h"

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

static void RegisterVertex( std::vector<unsigned short>& faces,
	std::vector<ArnVertex>& totVerts, const ArnMesh* mesh,
	const int vind, const int faceIdx, const int i)
{
	assert (vind < 0x0000ffff);
	faces.push_back ( totVerts.size() );

	ArnVec3 pos, nor, uv;
	mesh->getVert (&pos, &nor, &uv, vind, false);
	ArnVertex vert;
	vert.x = pos.x;		vert.y = pos.y;		vert.z = pos.z;
	vert.nx = nor.x;	vert.ny = nor.y;	vert.nz = nor.z;
	float u = 0, v = 0;
	if (mesh->getTriquadUvChunk())
	{
		const float *uvVal = reinterpret_cast<const float *>
			(mesh->getTriquadUvChunk()->getRecordAt( faceIdx ));
		u = *(uvVal + 2*i + 0);
		v = *(uvVal + 2*i + 1);
	}
	vert.u = u;
	vert.v = v;
	totVerts.push_back(vert);
}

/*
DirectX 9의 그래픽 API에는 기본적으로 사각면(quad-face)을 그려주는
기능이 없다. 블렌더 측의 익스포터는 그러나 사각면을 포함하고 있으므로
우리는 하나의 사각면을 적절히 삼각면(tri-face 혹은 triangle)으로
변환해야 한다. 변환은 사각면을 이루는 정점 4개를 각각 a, b, c, d라고
했을 때 정점 a, b, c로 이루어진 삼각면과 정점 c, d, a로 이루어진
삼각면을 이용해 사각면 하나를 나타낼 수 있다.

정점 당(per-vertex) 존재하는 정보는 일반적으로 position,
normal, UV texcoord이 있다. 익스포터로 만들어진 모델 파일에는
position과 normal 값은 정점 당 저장되어있으므로 큰 문제가 되지
않지만 UV texcoord는 면 당(per-face) 저장되어 있다. 예를 들어
아래와 같이 두 개의 삼각면으로 이루어진 사각형 모양의 메시가
있다고 하면,

a--b
| /|
|/ |
c--d

정점 개수는 4개이다. 만일 정점 당 position, normal, UV texcoord를
설정한다고 하면 b와 c에서의 UV texcoord도 각각 하나로 정해지게 된다.
아래와 같이 만일 텍스쳐를 입힐 때 삼각형 abc와 삼각형 cdb를
서로 떨어진 영역으로 맵핑했다면,

[v]
^-----------------------
|     a                |
|     |\               |
|     | \              |
|     b--c             |
|             c-----b  |
|              \   /   |
|               \d/    |
|----------------------> [u]

필연적으로 공유 정점 b, c는 서로 다른 UV texcoord값을 가져야 한다.
이러한 이유로 인해 UV coord 값은 면 당 설정되어야 하며, 이는 다시 말해
기존의 공유되는 정점이 복제되어 DirectX 9으로 넘어가야하는 경우도
있다는 뜻이 된다.

아래 코드는 공유 정점이 복제되지 않은 정점 정보와 면 당 UV coord값을
분리해 저장하는 방식을 택한 익스포트 모델 파일을 이용해 DirectX 9에 맞게
가공하여 런타임 데이터를 형성하는 것이다. 기본적으로 메시 하나는
다수의 면 그룹(face group)으로 이루어질 수 있고 각 면 그룹은
삼각면과 사각면을 가질 수 있다. UV texcoord는 아래와 같이 저장되어
있다.

|                 Face group 0                  | ... |         Face group n            |
|-----------------------------------------------| ...
|  Tri-face UV coords   |   Quad-face UV coords | ...
|-----------------------------------------------| ...
| float8 | ... | float8 | float8 | ... | float8 | ...
|-----------------------------------------------|

여기서 주의할 점은 삼각면이든 사각면이든 각 면 당 UV값은 float[8] 형식으로
저장된다는 점이다. 즉 삼각면 UV 값을 나타내는 float[8] 중 뒷부분의 두 float는
무시한다.
*/
ArnMeshDx9* ArnMeshDx9::createFrom (const ArnMesh* mesh)
{	
	ArnMeshDx9 *ret = new ArnMeshDx9 ();
	assert(mesh->getFaceGroupCount() && mesh->getVertGroupCount());
	std::vector <ArnVertex> totVerts;
	std::vector <unsigned short> faces;
	std::vector <DWORD> attr;
	unsigned int fgCount = mesh->getFaceGroupCount ();
	int faceIdx = 0; // tri- and quad-face aware index. Treats as quad face is a single face.
	for (unsigned int fg = 0; fg < fgCount; ++fg)
	{
		unsigned int triCount, quadCount;
		mesh->getFaceCount (triCount, quadCount, fg);
		assert(triCount + quadCount > 0);

		// UV coord 정보는 tri, quad 순서이므로
		// 이 처리 순서가 뒤바뀌면 안됨.
		for (unsigned int t = 0; t < triCount; ++t)
		{
			unsigned int faceIdx;
			unsigned int vind[3];
			mesh->getTriFace (faceIdx, vind, fg, t);
			for (int i = 0; i < 3; ++i)
				RegisterVertex(faces, totVerts, mesh, vind[i], faceIdx, i);
			attr.push_back (fg);
			++faceIdx;
		}

		for (unsigned int t = 0; t < quadCount; ++t)
		{
			unsigned int faceIdx;
			unsigned int vind[4];
			mesh->getQuadFace (faceIdx, vind, fg, t);
			/* A single quad face transformed to
			 * two tri faces.
			 *
			 *       vind[0] vind[1] vind[2] vind[3]
			 * faceA    1      2        3
			 * faceB    3               1      2
			 */
			static const int faceAIdx[3] = { 0, 1, 2 };
			static const int faceBIdx[3] = { 2, 3, 0 };
			/* faceA */
			for (int i = 0; i < 3; ++i)
				RegisterVertex(faces, totVerts, mesh, vind[ faceAIdx[i] ],
					faceIdx, faceAIdx[i]);
			attr.push_back (fg);
			/* faceB */
			for (int i = 0; i < 3; ++i)
				RegisterVertex(faces, totVerts, mesh, vind[ faceBIdx[i] ],
					faceIdx, faceBIdx[i]);
			attr.push_back (fg);

			++faceIdx;
		}
	}
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
	nm3.m_meshVerticesCount = totVerts.size();
	ArnVertex *vertex_mem = new ArnVertex[nm3.m_meshVerticesCount];
	memcpy(vertex_mem, &totVerts[0], sizeof(ArnVertex) * nm3.m_meshVerticesCount);
	nm3.m_vertex = vertex_mem;
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

void
ArnMeshDx9::interconnect( ArnNode* sceneRoot )
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

int
ArnMeshDx9::render( bool bIncludeShadeless ) const
{
	assert (m_d3dxMesh);
	return m_d3dxMesh->DrawSubset (0);
}

void
ArnMeshDx9::cleanup()
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}
