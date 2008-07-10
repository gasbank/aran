#include "StdAfx.h"
#include "ArnFile.h"

enum LoadErrorEnum
{
	LEE_FILEDESCRIPTOR_CORRUPTED,
	LEE_TERMINALSTRING_CORRUPTED,
	LEE_UNSUPPORTED_NODE,
	LEE_UNDEFINED_ERROR,
	LEE_SKELETON_BONES_ERROR,
	LEE_HIERARCHY_FRAMES_ERROR,
	LEE_BOOL_DATA_PARSE,
};

class MyError
{
public:
	MyError(LoadErrorEnum lee)
	{
		switch (lee)
		{
		case LEE_FILEDESCRIPTOR_CORRUPTED:	m_str = "File descriptor in ARN file is corrupted";
		case LEE_TERMINALSTRING_CORRUPTED:	m_str = "Terminal string in ARN file is corrupted";
		case LEE_UNDEFINED_ERROR:			m_str = "Undefined error occurred!";
		case LEE_UNSUPPORTED_NODE:			m_str = "Unsupported or not implemented node detected; skip this node...";
		case LEE_SKELETON_BONES_ERROR:		m_str = "Skeleton node's bone count error";
		case LEE_HIERARCHY_FRAMES_ERROR:	m_str = "Hierarchy node's MyFrame count error";
		case LEE_BOOL_DATA_PARSE:			m_str = "Error while parsing BOOL data type from ARN file";
		default:							m_str = "<Should not see me!>";
		}
	}
private:
	char* m_str;
};


void load_arnfile( const char* fileName, ArnFileData& afd )
{
	afd.m_nodes = 0;

	file_load(fileName, afd.m_file);

	afd.m_fileDescriptor = file_read_string(afd.m_file);
	if (!(afd.m_fileDescriptor[0] == 'A'
		&& afd.m_fileDescriptor[1] == 'R'
		&& afd.m_fileDescriptor[2] == 'N'))
		throw MyError(LEE_FILEDESCRIPTOR_CORRUPTED);

	afd.m_nodeCount = file_read_uint(afd.m_file);

	unsigned int i;
	afd.m_nodes = new NodeBase*[afd.m_nodeCount];
	for (i = 0; i < afd.m_nodeCount; ++i)
	{
		parse_node(afd.m_file, afd.m_nodes[i]);
	}
	
	afd.m_terminalDescriptor = file_read_string(afd.m_file);
	if (strcmp(afd.m_terminalDescriptor, "TERM") != 0)
		throw MyError(LEE_TERMINALSTRING_CORRUPTED);
}

void parse_node( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	NODE_DATA_TYPE ndt = (NODE_DATA_TYPE)file_read_int(abf);
	typedef void (*parse_node_func)(ArnBinaryFile&, NodeBase*&);
	parse_node_func node_chunk_parser_func = 0;

	switch (ndt)
	{
	case NDT_MESH1:
		throw MyError(LEE_UNSUPPORTED_NODE);
	case NDT_MESH2:
		nodeBase = new NodeMesh2();
		node_chunk_parser_func = parse_nodeMesh2;
		break;
	case NDT_SKELETON:
		nodeBase = new NodeSkeleton();
		node_chunk_parser_func = parse_nodeSkeleton;
		break;
	case NDT_HIERARCHY:
		nodeBase = new NodeHierarchy();
		node_chunk_parser_func = parse_nodeHierarchy;
		break;
	case NDT_LIGHT:
		nodeBase = new NodeLight();
		node_chunk_parser_func = parse_nodeLight;
		break;
	case NDT_CAMERA:
		nodeBase = new NodeCamera();
		node_chunk_parser_func = parse_nodeCamera;
		break;
	case NDT_ANIM1:
		nodeBase = new NodeAnim1();
		node_chunk_parser_func = parse_nodeAnim1;
		break;
	default:
		throw MyError(LEE_UNSUPPORTED_NODE);
	}

	nodeBase->m_ndt = ndt;
	nodeBase->m_nodeName = file_read_string(abf);
	nodeBase->m_nodeChunkSize = file_read_uint(abf);

	if (node_chunk_parser_func)
	{
		node_chunk_parser_func(abf, nodeBase);
	}
	else
		throw MyError(LEE_UNDEFINED_ERROR);
}

void parse_nodeMesh2( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_MESH2);
	NodeMesh2* node = (NodeMesh2*)nodeBase;

	node->m_meshVerticesCount	= file_read_uint(abf);
	assert(node->m_meshVerticesCount >= 3);
	node->m_vdd					= file_read<ARN_VDD>(abf, node->m_meshVerticesCount);
	node->m_meshFacesCount		= file_read_uint(abf);
	node->m_triangleIndice		= file_read_uint_array(abf, node->m_meshFacesCount * 3);
	node->m_materialRefs		= file_read_uint_array(abf, node->m_meshFacesCount);
	node->m_materialCount		= file_read_uint(abf);
	if (node->m_materialCount > 0)
	{
		node->m_mtds			= new ARN_MTD_Data[node->m_materialCount];
		unsigned int i;
		for (i = 0; i < node->m_materialCount; ++i)
		{
			node->m_mtds[i].m_strMatName = file_read_string(abf);
			node->m_mtds[i].m_d3dMat = file_read<D3DMATERIAL9>(abf);
			node->m_mtds[i].m_strTexFileName = file_read_string(abf);
		}
	}
	else
	{
		node->m_mtds = 0;
	}
	
	parse_node(abf, node->m_nodeAnim1);
}


void parse_nodeSkeleton( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_SKELETON);
	NodeSkeleton* node = (NodeSkeleton*)nodeBase;

	node->m_associatedMeshName		= file_read_string(abf);
	node->m_maxWeightsPerVertex		= file_read_uint(abf);
	node->m_boneCount				= file_read_uint(abf);
	if (node->m_boneCount > 0)
	{
		node->m_bones				= new BoneData[node->m_boneCount];
		unsigned int i;
		for (i = 0; i < node->m_boneCount; ++i)
		{
			node->m_bones[i].m_boneName			= file_read_string(abf);
			node->m_bones[i].m_offsetMatrix		= file_read<D3DMATRIX>(abf);
			node->m_bones[i].m_infVertexCount	= file_read_uint(abf);
			node->m_bones[i].m_vertexIndices	= file_read_uint_array(abf, node->m_bones[i].m_infVertexCount);
			node->m_bones[i].m_weights			= file_read<float>(abf, node->m_bones[i].m_infVertexCount);
			parse_node(abf, node->m_bones[i].m_nodeAnim1);
		}
	}
	else
		throw MyError(LEE_SKELETON_BONES_ERROR);
}

void parse_nodeHierarchy( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_HIERARCHY);
	NodeHierarchy* node = (NodeHierarchy*)nodeBase;

	node->m_frameCount		= file_read_uint(abf);
	if (node->m_frameCount > 0)
	{
		node->m_frames		= new MyFrameData[node->m_frameCount];
		unsigned int i;
		for (i = 0; i < node->m_frameCount; ++i)
		{
			node->m_frames[i].m_frameName	= file_read_string(abf);
			node->m_frames[i].m_rootFlag	= file_read_BOOL(abf);
			node->m_frames[i].m_sibling	= file_read_int(abf);
			node->m_frames[i].m_firstChild = file_read_int(abf);
		}
	}
	else
		throw MyError(LEE_HIERARCHY_FRAMES_ERROR);
	
}

void parse_nodeLight( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_LIGHT);
	NodeLight* node = (NodeLight*)nodeBase;

	node->m_light = file_read<D3DLIGHT9>(abf);
}
void parse_nodeCamera( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_CAMERA);
	NodeCamera* node = (NodeCamera*)nodeBase;

	node->m_camera = file_read<ARN_NDD_CAMERA_CHUNK>(abf);
}

void parse_nodeAnim1(ArnBinaryFile& abf, NodeBase*& nodeBase)
{
	assert(nodeBase->m_ndt == NDT_ANIM1);
	NodeAnim1* node = (NodeAnim1*)nodeBase;

	node->m_keyCount = file_read_uint(abf);
	node->m_rstKeys = file_read<RST_DATA>(abf, node->m_keyCount);
}

void release_arnfile( ArnFileData& afd )
{
	delete [] afd.m_nodes;
	file_unload(afd.m_file);
}



//////////////////////////////////////////////////////////////////////////

void file_load( const char* fileName, ArnBinaryFile& file )
{
	FILE* f;
	fopen_s(&f, fileName, "rb");
	if (!f)
		throw std::runtime_error("File open error");
	fseek(f, 0, SEEK_END);
	file.m_fileSize = ftell(f);
	fseek(f, 0, SEEK_SET);
	file.m_data = new char[file.m_fileSize];
	fread(file.m_data, file.m_fileSize, 1, f);
	fclose(f);
	file.m_curPos = 0;
}

void file_unload( ArnBinaryFile& file )
{
	delete file.m_data;
}


char*			file_read_string(ArnBinaryFile& file) { char* c = file.m_data + file.m_curPos; file.m_curPos += strlen(c) + 1; return c; }
unsigned int	file_read_uint(ArnBinaryFile& file) { unsigned int ui = *(unsigned int*)(file.m_data + file.m_curPos); file.m_curPos += sizeof(unsigned int); return ui; }
unsigned int*	file_read_uint_array(ArnBinaryFile& file, unsigned int count) { unsigned int* uia = (unsigned int*)(file.m_data + file.m_curPos); file.m_curPos += sizeof(unsigned int) * count; return uia; }
int				file_read_int(ArnBinaryFile& file) { int i = *(int*)(file.m_data + file.m_curPos); file.m_curPos += sizeof(int); return i; }
float			file_read_float(ArnBinaryFile& file) { float f = *(float*)(file.m_data + file.m_curPos); file.m_curPos += sizeof(float); return f; }
BOOL			file_read_BOOL(ArnBinaryFile& file) { BOOL b = *(BOOL*)(file.m_data + file.m_curPos); if (b == TRUE || b == FALSE) { file.m_curPos += sizeof(BOOL); return b; } throw MyError(LEE_BOOL_DATA_PARSE); }

