#pragma once
#include "ArnXformable.h"

struct NodeBase;
struct NodeMesh2;
struct NodeMesh3;
class ArnMaterial;
class ArnHierarchy;
class ArnVertexBuffer;
class ArnIndexBuffer;
class ArnBinaryChunk;

class ARAN_API ArnMesh : public ArnXformable
{
public:
	typedef std::vector<ArnMaterial*>		MaterialRefList;
											~ArnMesh();
	static ArnMesh*							createFrom(const NodeBase* nodeBase);
	static ArnMesh*							createFromVbIb(const ArnVertexBuffer* vb, const ArnIndexBuffer* ib);
	static ArnMesh*							createFrom(const DOMElement* elm, char* binaryChunkBasePtr);
	bool									initRendererObject();
	inline const LPD3DXMESH&				getD3DXMesh() const;
	inline void								setD3DXMesh(const LPD3DXMESH d3dxMesh);
	inline const MeshData&					getMeshData() const;
	const ArnMaterialData*					getMaterial(unsigned int i) const;
	inline ArnMaterial*						getMaterialNode(unsigned int i) const;
	inline bool								isVisible() const;
	inline void								setVisible(bool val);
	inline bool								isCollide() const;
	inline void								setCollide(bool val);
	inline const NodeMesh3*					getNodeMesh3() const;
	void									render();
	void									renderVbIb();
	void									renderXml();

	unsigned int							getFaceGroupCount() const { return m_faceGroup.size(); }
	unsigned int							getFaceCount(unsigned int& triCount, unsigned int& quadCount, unsigned int faceGroupIdx) const;
	void									getTriFace(unsigned int& faceIdx, unsigned int vind[3], unsigned int faceGroupIdx, unsigned int triFaceIndex) const;
	void									getQuadFace(unsigned int& faceIdx, unsigned int vind[4], unsigned int faceGroupIdx, unsigned int quadFaceIndex) const;

	unsigned int							getVertGroupCount() const { return m_vertexGroup.size(); }
	unsigned int							getVertCountOfVertGroup(unsigned int vertGroupIdx) const;
	unsigned int							getTotalVertCount() const { return getVertCountOfVertGroup(0); }
	void									getVert(ArnVec3* pos, ArnVec3* nor, ArnVec3* uv, unsigned int vertIdx, bool finalXformed) const;

	void									setTwoSided(bool b) { m_bTwoSided = b; }

	// ********************************* INTERNAL USE ONLY START *********************************
	virtual void							interconnect(ArnNode* sceneRoot);
	// *********************************  INTERNAL USE ONLY END  *********************************
private:
											ArnMesh();

	void									buildFrom(const NodeMesh2* nm);
	void									buildFrom(const NodeMesh3* nm);

	bool									initRendererObjectVbIb();
	bool									initRendererObjectXml();


	struct FaceGroup
	{
		int mtrlIndex;
		ArnBinaryChunk* triFaceChunk;
		ArnBinaryChunk* quadFaceChunk;
	};
	std::vector<FaceGroup>					m_faceGroup;

	struct VertexGroup
	{
		int mtrlIndex;
		ArnBinaryChunk* vertGroupChunk;
	};
	std::vector<VertexGroup>				m_vertexGroup;
	ArnBinaryChunk*							m_vertexChunk; // Contains the entire vertices.


	LPD3DXMESH								m_d3dxMesh;
	LPDIRECT3DVERTEXBUFFER9					m_d3dvb;
	LPDIRECT3DINDEXBUFFER9					m_d3dib;

	std::vector<std::string>				m_mtrlRefNameList;
	MaterialRefList							m_materialRefList;
	MeshData								m_data;

	bool									m_bVisible;
	bool									m_bCollide;

	ArnHierarchy*							m_skeleton;

	struct BoneDataInternal
	{
		STRING nameFixed;
		std::vector<std::pair<DWORD, float> >
			indWeight;
	};
	std::vector<BoneDataInternal>			m_boneDataInt;

	void									setVertexBuffer(const ArnVertexBuffer* vb);
	void									setIndexBuffer(const ArnIndexBuffer* ib);
	const ArnVertexBuffer*					m_arnVb;
	const ArnIndexBuffer*					m_arnIb;
	ArnBinaryChunk*							m_triquadUvChunk;

	GLuint									m_vboId;
	std::vector<GLuint>						m_vboIds;
	GLuint									m_vboUv;
	
	bool									m_bTwoSided;

	void									(ArnMesh::*m_renderFunc)();
	bool									(ArnMesh::*m_initRendererObjectFunc)();

	// Temporary code
	const NodeMesh3*						m_nodeMesh3;
};

#include "ArnMesh.inl"

#ifdef WIN32
HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh2* nm, OUT LPD3DXMESH& mesh);
HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh3* nm, OUT LPD3DXMESH& mesh);
HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh3* nm, OUT LPDIRECT3DVERTEXBUFFER9& d3dvb, OUT LPDIRECT3DINDEXBUFFER9& d3dib);
#else
static inline HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh2* nm, OUT LPD3DXMESH& mesh) { ARN_THROW_NOT_IMPLEMENTED_ERROR }
static inline HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh3* nm, OUT LPD3DXMESH& mesh)  { ARN_THROW_NOT_IMPLEMENTED_ERROR }
static inline HRESULT arn_build_mesh(IN LPDIRECT3DDEVICE9 dev, IN const NodeMesh3* nm, OUT LPDIRECT3DVERTEXBUFFER9& d3dvb, OUT LPDIRECT3DINDEXBUFFER9& d3dib) { ARN_THROW_NOT_IMPLEMENTED_ERROR }
#endif

class VideoMan;

#ifdef WIN32
inline HRESULT ArnCreateMeshFVF(DWORD NumFaces, DWORD NumVertices, DWORD FVF, VideoMan* vman, LPD3DXMESH* ppMesh);
#endif
