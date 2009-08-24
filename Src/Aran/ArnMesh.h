/*!
 * @file ArnMesh.h
 * @author Geoyeob Kim
 * @date 2009
 */
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
class ArnRenderableObject;
class TiXmlElement;
typedef std::vector<ArnMaterial*>		MaterialRefList;

// Coordinates should be in the sequence of ---, --+, -++, -+-, +--, +-+, +++, ++-.
typedef boost::array<ArnVec3, 8> Arn8Points;

static const Arn8Points ArnUnitBoundingBox = {{

	ArnVec3(-0.5f, -0.5f, -0.5f),
	ArnVec3(-0.5f, -0.5f,  0.5f),
	ArnVec3(-0.5f,  0.5f,  0.5f),
	ArnVec3(-0.5f,  0.5f, -0.5f),
	ArnVec3( 0.5f, -0.5f, -0.5f),
	ArnVec3( 0.5f, -0.5f,  0.5f),
	ArnVec3( 0.5f,  0.5f,  0.5f),
	ArnVec3( 0.5f,  0.5f, -0.5f) }};

/*!
 * @brief 메시
 */
class ARAN_API ArnMesh : public ArnXformable
{
public:
											~ArnMesh();
	static ArnMesh*							createFromVbIb(const ArnVertexBuffer* vb, const ArnIndexBuffer* ib);
	static ArnMesh*							createFrom(const TiXmlElement* elm, const char* binaryChunkBasePtr);
	inline const MeshData&					getMeshData() const;
	const ArnMaterialData*					getMaterial(unsigned int i) const;
	inline ArnMaterial*						getMaterialNode(unsigned int i) const;
	inline bool								isCollide() const;
	inline void								setCollide(bool val);
	inline const NodeMesh3*					getNodeMesh3() const;
	void									render();

	/*!
	 * @name 면, 면 그룹
	 * 메시는 하나 이상의 면으로 이루어져 있고, 각 면은 하나 이상의 면 그룹에
	 * 속하거나 아무데도 속하지 않을 수 있다. 면은 정점 세 개로 이루어진 삼각형 면과
	 * 정점 네 개로 이루어진 사각형 면 두 종류가 있다. 면 그룹 하나에는
	 * 삼각형 면과 사각형 면이 모두 들어있다.
	 */
	//@{
	unsigned int							getFaceGroupCount() const { return m_faceGroup.size(); }
	unsigned int							getFaceCount(unsigned int& triCount, unsigned int& quadCount, unsigned int faceGroupIdx) const;
	void									getTriFace(unsigned int& faceIdx, unsigned int vind[3], unsigned int faceGroupIdx, unsigned int triFaceIndex) const;
	void									getQuadFace(unsigned int& faceIdx, unsigned int vind[4], unsigned int faceGroupIdx, unsigned int quadFaceIndex) const;
	//@}
	/*!
	 * @name 정점, 정점 그룹
	 * 메시는 셋 이상의 정점으로 이루어져 있고, 각 정점은 하나 이상의 정점 그룹에
	 * 속하거나 아무데도 속하지 않을 수 있다. 특히 정점 그룹 0은 이 메시의 모든 정점이
	 * 속한 고정된 그룹이다.
	 */
	//@{
	unsigned int							getVertGroupCount() const { return m_vertexGroup.size(); }
	unsigned int							getVertCountOfVertGroup(unsigned int vertGroupIdx) const;
	unsigned int							getTotalVertCount() const { return getVertCountOfVertGroup(0); }
	void									getVert(ArnVec3* pos, ArnVec3* nor, ArnVec3* uv, unsigned int vertIdx, bool finalXformed) const;
	//@}
	/*!
	 * @name 물질
	 * 메시는 면 그룹 단위로 물질을 설정할 수 있습니다. 즉 '면 그룹 - 물질 인덱스' 대응이
	 * 메시마다 저장되어 있습니다. 물질 인덱스는 각 메시마다 인덱스가 0으로 시작하는 지역적인
	 * 덱스입니다. (전역적으로 저장되는 물질 정보와는 인덱스가 다름)
	 * 이를 보완하기 위해 '물질 인덱스 - 물질 이름' 대응 정보 역시 가지고 있어서
	 * 최종적으로 '면 그룹 - 물질 인덱스 - 물질 이름'과 같은 대응을 알 수 있게 됩니다.
	 * 물질 이름은 전역적으로 고유해야하며 이를 통해 특정한 면 그룹의 물질 정보를 접근할 수 있습니다.
	 */
	//@{
	int										getMaterialIndexOfFaceGroup(unsigned int i) const { return m_faceGroup[i].mtrlIndex; }
	unsigned int							getMaterialReferenceNameCount() const { return m_mtrlRefNameList.size(); }
	const char*								getMaterialReferenceName(unsigned int i) const { assert(i < m_mtrlRefNameList.size()); return m_mtrlRefNameList[i].c_str(); }
	//@}
	/*!
	 * @name Bounding box
	 */
	//@{
	/*!
	 * @brief bounding box 크기를 가져옴
	 * @param out bounding box의 크기
	 * @param worldSpace true이면 scaling이 적용된 크기, false이면 scaling 적용 되지 않은 크기를 반환
	 */
	void									getBoundingBoxDimension(ArnVec3* out, bool worldSpace) const;
	/*!
	 * @brief bounding box를 정의하는 정점 전부를 가져옴
	 */
	const boost::array<ArnVec3, 8>&			getBoundingBoxPoints() const { return m_boundingBoxPoints; }
	/*!
	 * @brief bounding box를 정의하는 정점 중 하나를 가져옴
	 * @param i bounding box를 이루는 0~7 사이의 인덱스 값
	 * @return bounding box를 정의하는 i번째 정점 좌표
	 * @remarks 로컬 변환이 이루어지기 전 메시의 고유한 bounding box의 정점을 반환합니다.
	 */
	const ArnVec3*							getBoundingBoxPoint(unsigned int i) const { assert(i < 8); return &m_boundingBoxPoints[i]; }
	ArnBoundingBoxType						getBoundingBoxType() const { return m_abbt; }
	/*!
	 * @brief bounding box 렌더링 가능 여부를 반환
	 * @return 현재 bounding box를 정의하는 정점 값이 유효하고, bounding box 렌더링 플래그가 켜져 있는 경우
	 * true를 반환하고 그 외에는 false 반환
	 */
	bool									isOkayToRenderBoundingBox() const { return !m_bBoundingBoxPointsDirty && m_bRenderBoundingBox; }
	//@}
	/*!
	 * @name 양면 메시
	 * 메시가 앞 뒤 culling 없이 모두 렌더링 되어야 하는 경우 양면 메시로 설정해야 합니다.
	 */
	//@{
	void									setTwoSided(bool b) { m_bTwoSided = b; }
	bool									isTwoSided() const { return m_bTwoSided; }
	//@}
	/*!
	 * @name 물리
	 * 물리 시뮬레이션과 관련된 메소드입니다.
	 */
	//@{
	/*!
	 * @brief 이 메시가 물리 시뮬레이터에 따라 움직이는 것이면 \c true, 아니면 \c false 반환
	 */
	bool									isPhyActor() const { return m_bPhyActor; }
	/*!
	 * @brief 이 메시의 물리적 특성 중 하나인 질량을 반환
	 * @remarks 질량이 0이면서 물리 시뮬레이터에 따라 움직이는 것은 일반적인 강체가 아니라
	 * 공간상에 완전히 고정된 물체라고 생각하면 됩니다.
	 * @sa isPhyActor
	 */
	float									getMass() const { return m_mass; }
	//@}

	const ArnVertexBuffer*					getVertexBuffer() const { return m_arnVb; }
	const ArnIndexBuffer*					getIndexBuffer() const { return m_arnIb; }
	const ArnBinaryChunk*					getVertexChunk() const { return m_vertexChunk; }
	const ArnBinaryChunk*					getTriquadUvChunk() const { return m_triquadUvChunk; }
	const ArnBinaryChunk*					getTriFaceChunkOfFaceGroup(unsigned int i) const { return m_faceGroup[i].triFaceChunk; }
	const ArnBinaryChunk*					getQuadFaceChunkOfFaceGroup(unsigned int i) const { return m_faceGroup[i].quadFaceChunk; }

	/*!
	 * @internalonly
	 */
	//@{
	virtual void							interconnect(ArnNode* sceneRoot);
	//@}
private:
											ArnMesh();
	bool									initRendererObjectVbIb();
	bool									initRendererObjectXml();
	void									setVertexBuffer(const ArnVertexBuffer* vb);
	void									setIndexBuffer(const ArnIndexBuffer* ib);
	void									setBoundingBoxPoints(ArnVec3 bb[8]);

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

	bool									m_bCollide;
	ArnHierarchy*							m_skeleton;
	const ArnVertexBuffer*					m_arnVb;
	const ArnIndexBuffer*					m_arnIb;
	std::vector<FaceGroup>					m_faceGroup;
	std::vector<VertexGroup>				m_vertexGroup;
	ArnBinaryChunk*							m_triquadUvChunk;
	ArnBinaryChunk*							m_vertexChunk; // Contains the entire vertices.
	std::vector<std::string>				m_mtrlRefNameList;
	MaterialRefList							m_materialRefList;
	MeshData								m_data;
	std::vector<BoneDataInternal>			m_boneDataInt;
	bool									m_bTwoSided;

	/*!
	 * @name Bounding box coordinates
	 * Coordinates should be in the sequence of ---, --+, -++, -+-, +--, +-+, +++, ++-.
	 */
	//@{
	boost::array<ArnVec3, 8>				m_boundingBoxPoints;
	bool									m_bBoundingBoxPointsDirty;
	bool									m_bRenderBoundingBox;
	//@}

	ArnBoundingBoxType						m_abbt;
	bool									m_bPhyActor;
	float									m_mass;

	// Temporary code
	const NodeMesh3*						m_nodeMesh3;
};

#include "ArnMesh.inl"
