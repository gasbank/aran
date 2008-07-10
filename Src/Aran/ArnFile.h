#pragma once

struct NodeBase
{
	NODE_DATA_TYPE m_ndt;
	char* m_nodeName;
	unsigned int m_nodeChunkSize;
};

struct NodeMesh2 : public NodeBase
{
	~NodeMesh2() { delete [] m_mtds; }

	unsigned int m_meshVerticesCount;
	ARN_VDD* m_vdd;
	unsigned int m_meshFacesCount;
	unsigned int* m_triangleIndice;
	unsigned int* m_materialRefs;
	unsigned int m_materialCount;
	ARN_MTD_Data* m_mtds;

	NodeBase* m_nodeAnim1; // TODO: Anim1 node --- should be reorganized!
};

struct NodeAnim1 : public NodeBase
{
	unsigned int m_keyCount;
	RST_DATA* m_rstKeys;
};

struct BoneData
{
	char* m_boneName;
	D3DMATRIX* m_offsetMatrix;
	unsigned int m_infVertexCount; // influencing vertex count
	unsigned int* m_vertexIndices;
	float* m_weights;
	
	NodeBase* m_nodeAnim1; // TODO: Anim1 node --- should be reorganized!
};

struct NodeSkeleton : public NodeBase
{
	~NodeSkeleton() { delete [] m_bones; }
	char* m_associatedMeshName;
	unsigned int m_maxWeightsPerVertex;
	unsigned int m_boneCount;
	BoneData* m_bones;
};

struct MyFrameData
{
	char* m_frameName;
	BOOL m_rootFlag;
	int m_sibling;
	int m_firstChild;
};

struct NodeHierarchy : public NodeBase
{
	~NodeHierarchy() { delete [] m_frames; }
	unsigned int m_frameCount;
	MyFrameData* m_frames;
};

struct NodeLight : public NodeBase
{
	D3DLIGHT9* m_light;
};

struct NodeCamera : public NodeBase
{
	ARN_NDD_CAMERA_CHUNK* m_camera;
};

struct ArnBinaryFile
{
	unsigned int m_fileSize;
	char* m_data;
	unsigned int m_curPos;
};

struct ArnFileData
{
	char* m_fileDescriptor;
	unsigned int m_nodeCount;
	NodeBase** m_nodes;
	char* m_terminalDescriptor;

	ArnBinaryFile m_file;
};



void load_arnfile(const char* fileName, ArnFileData& afd);
void release_arnfile(ArnFileData& afd);

void parse_node(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeMesh2(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeSkeleton(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeHierarchy(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeLight(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeCamera(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeAnim1(ArnBinaryFile& abf, NodeBase*& nodeBase);

//////////////////////////////////////////////////////////////////////////

void			file_load(const char* fileName, ArnBinaryFile& file);
void			file_unload(ArnBinaryFile& file);


char*			file_read_string(ArnBinaryFile& file);
unsigned int	file_read_uint(ArnBinaryFile& file);
unsigned int*	file_read_uint_array(ArnBinaryFile& file, unsigned int count);
int				file_read_int(ArnBinaryFile& file);
float			file_read_float(ArnBinaryFile& file);
BOOL			file_read_BOOL(ArnBinaryFile& file);

ARN_VDD*		file_read_arn_vdd_array(ArnBinaryFile& file, unsigned int count);
ARN_MTD*		file_read_arn_mtd_array(ArnBinaryFile& file, unsigned int count);


template<class T>
T* file_read(ArnBinaryFile& file, unsigned int count = 1)
{
	if (count == 0)
		throw std::runtime_error("Unacceptable parameter");
	T* t = (T*)(file.m_data + file.m_curPos);
	file.m_curPos += sizeof(T) * count;
	return t;
}