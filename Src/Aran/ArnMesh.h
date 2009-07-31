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
	static ArnMesh*							createFromVbIb(const ArnVertexBuffer* vb, const ArnIndexBuffer* ib);
	static ArnMesh*							createFrom(const DOMElement* elm, char* binaryChunkBasePtr);
	bool									initRendererObject();
	inline const MeshData&					getMeshData() const;
	const ArnMaterialData*					getMaterial(unsigned int i) const;
	inline ArnMaterial*						getMaterialNode(unsigned int i) const;
	inline bool								isVisible() const;
	inline void								setVisible(bool val);
	inline bool								isCollide() const;
	inline void								setCollide(bool val);
	inline const NodeMesh3*					getNodeMesh3() const;
	void									render();
	
	unsigned int							getFaceGroupCount() const { return m_faceGroup.size(); }
	unsigned int							getFaceCount(unsigned int& triCount, unsigned int& quadCount, unsigned int faceGroupIdx) const;
	void									getTriFace(unsigned int& faceIdx, unsigned int vind[3], unsigned int faceGroupIdx, unsigned int triFaceIndex) const;
	void									getQuadFace(unsigned int& faceIdx, unsigned int vind[4], unsigned int faceGroupIdx, unsigned int quadFaceIndex) const;

	unsigned int							getVertGroupCount() const { return m_vertexGroup.size(); }
	unsigned int							getVertCountOfVertGroup(unsigned int vertGroupIdx) const;
	unsigned int							getTotalVertCount() const { return getVertCountOfVertGroup(0); }
	void									getVert(ArnVec3* pos, ArnVec3* nor, ArnVec3* uv, unsigned int vertIdx, bool finalXformed) const;

	void									setTwoSided(bool b) { m_bTwoSided = b; }
	void									getBoundingBoxDimension(ArnVec3* out, bool worldSpace) const;

	// ********************************* INTERNAL USE ONLY START *********************************
	virtual void							interconnect(ArnNode* sceneRoot);
	// *********************************  INTERNAL USE ONLY END  *********************************
private:
											ArnMesh();
	bool									initRendererObjectVbIb();
	bool									initRendererObjectXml();
	void									setVertexBuffer(const ArnVertexBuffer* vb);
	void									setIndexBuffer(const ArnIndexBuffer* ib);
	void									setBoundingBoxPoints(ArnVec3 bb[8]);
	virtual void							renderVbIb() = 0;
	virtual void							renderXml() = 0;

	struct FaceGroup
	{
		int					mtrlIndex;
		ArnBinaryChunk*		triFaceChunk;
		ArnBinaryChunk*		quadFaceChunk;
	};
	struct VertexGroup
	{
		int					mtrlIndex;
		ArnBinaryChunk*		vertGroupChunk;
	};
	struct BoneDataInternal
	{
		std::string				nameFixed;
		std::vector<std::pair<DWORD, float> >
							indWeight;
	};

	std::vector<FaceGroup>					m_faceGroup;
	std::vector<VertexGroup>				m_vertexGroup;
	ArnBinaryChunk*							m_vertexChunk; // Contains the entire vertices.
	std::vector<std::string>				m_mtrlRefNameList;
	MaterialRefList							m_materialRefList;
	MeshData								m_data;
	bool									m_bVisible;
	bool									m_bCollide;
	ArnHierarchy*							m_skeleton;
	std::vector<BoneDataInternal>			m_boneDataInt;
	const ArnVertexBuffer*					m_arnVb;
	const ArnIndexBuffer*					m_arnIb;
	ArnBinaryChunk*							m_triquadUvChunk;
	bool									m_bTwoSided;
	
	/*! @name Bounding box coordinates
	Coordinates should be in the sequence of ---, --+, -++, -+-, +--, +-+, +++, ++-.
	*/
	//@{
	ArnVec3									m_boundingBoxPoints[8];
	bool									m_bBoundingBoxPointsDirty;
	bool									m_bRenderBoundingBox;
	//@}

	void									(ArnMesh::*m_renderFunc)();
	bool									(ArnMesh::*m_initRendererObjectFunc)();

	// Temporary code
	const NodeMesh3*						m_nodeMesh3;
};

#include "ArnMesh.inl"
