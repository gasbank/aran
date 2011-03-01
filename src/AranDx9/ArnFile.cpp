#include "AranDx9PCH.h"
#include "ArnFile.h"
#include "Log.h"

void load_arnfile( const TCHAR* fileName, ArnFileData& afd )
{
	file_load(fileName, afd.m_file);

	afd.m_fileDescriptor = file_read_string(afd.m_file);
	if (!(afd.m_fileDescriptor[0] == 'A'
		&& afd.m_fileDescriptor[1] == 'R'
		&& afd.m_fileDescriptor[2] == 'N'))
		throw MyError(MEE_FILEDESCRIPTOR_CORRUPTED);

	afd.m_nodeCount = file_read_uint(afd.m_file);

	while (afd.m_file.m_fileSize - 5 > afd.m_file.m_curPos)
	{
		NodeBase* node;
		parse_node(afd.m_file, node);
		afd.m_nodes.push_back(node);
	}

	afd.m_terminalDescriptor = file_read_string(afd.m_file);
	if (strcmp(afd.m_terminalDescriptor, "TERM") != 0)
		throw MyError(MEE_TERMINALSTRING_CORRUPTED);
}

void parse_node( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	NODE_DATA_TYPE ndt = (NODE_DATA_TYPE)file_read_int(abf);
	typedef void (*parse_node_func)(ArnBinaryFile&, NodeBase*&);
	parse_node_func node_chunk_parser_func = 0;

	switch (ndt)
	{
	case NDT_MATERIAL1:
		nodeBase = new NodeMaterial1();
		node_chunk_parser_func = parse_nodeMaterial1;
		break;
	case NDT_MATERIAL2:
		nodeBase = new NodeMaterial2();
		node_chunk_parser_func = parse_nodeMaterial2;
		break;
	case NDT_MESH2:
		nodeBase = new NodeMesh2();
		node_chunk_parser_func = parse_nodeMesh2;
		break;
	case NDT_MESH3:
		nodeBase = new NodeMesh3();
		node_chunk_parser_func = parse_nodeMesh3;
		break;
	case NDT_SKELETON1:
		nodeBase = new NodeSkeleton1();
		node_chunk_parser_func = parse_nodeSkeleton1;
		break;
	case NDT_HIERARCHY1:
		nodeBase = new NodeHierarchy1();
		node_chunk_parser_func = parse_nodeHierarchy1;
		break;
	case NDT_HIERARCHY2:
		nodeBase = new NodeHierarchy2();
		node_chunk_parser_func = parse_nodeHierarchy2;
		break;
	case NDT_LIGHT1:
		nodeBase = new NodeLight1();
		node_chunk_parser_func = parse_nodeLight1;
		break;
	case NDT_LIGHT2:
		nodeBase = new NodeLight2();
		node_chunk_parser_func = parse_nodeLight2;
		break;
	case NDT_CAMERA1:
		nodeBase = new NodeCamera1();
		node_chunk_parser_func = parse_nodeCamera1;
		break;
	case NDT_CAMERA2:
		nodeBase = new NodeCamera2();
		node_chunk_parser_func = parse_nodeCamera2;
		break;
	case NDT_ANIM1:
		nodeBase = new NodeAnim1();
		node_chunk_parser_func = parse_nodeAnim1;
		break;
	case NDT_BONE1:
		nodeBase = new NodeBone1();
		node_chunk_parser_func = parse_nodeBone1;
		break;
	case NDT_BONE2:
		nodeBase = new NodeBone2();
		node_chunk_parser_func = parse_nodeBone2;
		break;
	case NDT_IPO1:
		nodeBase = new NodeIpo1();
		node_chunk_parser_func = parse_nodeIpo1;
		break;
	case NDT_IPO2:
		nodeBase = new NodeIpo2();
		node_chunk_parser_func = parse_nodeIpo2;
		break;
	case NDT_ACTION1:
		nodeBase = new NodeAction1();
		node_chunk_parser_func = parse_nodeAction1;
		break;
	default:
		// unidentified node, maybe corrupted or unsupported; skip the node
		nodeBase = new NodeUnidentified();
		node_chunk_parser_func = parse_nodeUnidentified;
		break;
	}

	nodeBase->m_ndt				= ndt;
	nodeBase->m_nodeName		= file_read_string(abf);
	nodeBase->m_nodeChunkSize	= file_read_uint(abf);

	if (node_chunk_parser_func)
	{
		node_chunk_parser_func(abf, nodeBase);
	}
	else
		throw MyError(MEE_UNDEFINED_ERROR);
}

void parse_nodeUnidentified( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
#ifdef WIN32
	DebugBreak();
#endif
	// skip the whole chunk data
	file_read<char>(abf, nodeBase->m_nodeChunkSize);
	_LogWrite(_T("WARNING: Unidentified node detected and skipped while parsing!"), LOG_OKAY);
}


void parse_nodeMaterial1( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_MATERIAL1);
	NodeMaterial1* node = (NodeMaterial1*)nodeBase;

	node->m_materialCount = file_read_uint(abf);
}
void parse_nodeMaterial2( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_MATERIAL2);
	NodeMaterial2* node = (NodeMaterial2*)nodeBase;
	node->m_parentName	= file_read_string(abf);
	node->m_d3dMaterial = file_read<ArnMaterialData>(abf);
	node->m_texCount	= file_read_uint(abf);
	unsigned int i;
	for (i = 0; i < node->m_texCount; ++i)
		node->m_texNameList.push_back(file_read_string(abf));
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
			node->m_mtds[i].m_strMatName		= file_read_string(abf);
			node->m_mtds[i].m_d3dMat			= file_read<ArnMaterialData>(abf);
			node->m_mtds[i].m_strTexFileName	= file_read_string(abf);
		}
	}
	else
	{
		node->m_mtds = 0;
	}
}

void parse_nodeMesh3( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_MESH3);
	unsigned int i, j;
	NodeMesh3* node = (NodeMesh3*)nodeBase;

	node->m_parentName			= file_read_string(abf);
	node->m_ipoName				= file_read_string(abf);
	node->m_localXform			= file_read<ArnMatrix>(abf);
	node->m_bArmature			= file_read_BOOL(abf);
	for (i = 0; i < 15; ++i)
		file_read<int>(abf); // Unused data

	node->m_materialCount		= file_read_uint(abf);
	node->m_meshVerticesCount	= file_read_uint(abf);
	node->m_meshFacesCount		= file_read_uint(abf);

	node->m_attrToMaterialMap = 0;
	for (i = 0; i < node->m_materialCount; ++i)
		node->m_matNameList.push_back(file_read_string(abf));

	node->m_vertex = file_read<ArnVertex>(abf, node->m_meshVerticesCount);
	node->m_faces = file_read<unsigned short>(abf, node->m_meshFacesCount * 3);
	node->m_attr = file_read<DWORD>(abf, node->m_meshFacesCount);


	if (node->m_bArmature)
	{
		node->m_armatureName = file_read_string(abf);
		const unsigned int totalBoneMatIdxMap = file_read_uint(abf);
		for (j = 0; j < totalBoneMatIdxMap; ++j)
			node->m_boneMatIdxMap.push_back( file_read_string(abf) );

		node->m_weights = reinterpret_cast<float*>( file_read_implicit_array(abf, node->m_meshVerticesCount * sizeof(float) * 3) ); // Three weights per vertex. Fourth weight can be induced from these.
		node->m_matIdx = reinterpret_cast<unsigned char*>( file_read_implicit_array(abf, node->m_meshVerticesCount * sizeof(unsigned char) * 4) ); // This is not a string!

		// consistent
		const unsigned int totalBoneCount = file_read_uint(abf);
		node->m_bones.resize(totalBoneCount);
		if (totalBoneCount != totalBoneMatIdxMap)
			throw MyError(MEE_BONE_COUNT_INCONSISTENCY);
		for (j = 0; j < totalBoneCount; ++j)
		{
			node->m_bones[j].boneName = file_read_string(abf);
			node->m_bones[j].indWeightCount = file_read_uint(abf);
			assert(node->m_bones[j].indWeightCount);

			/*
			The following line is shortcut of these codes;

			for (k = 0; k < node->m_bones[j].indWeightCount; ++k)
			{
			node->m_bones[j].indWeight[k].ind = file_read_uint(abf);
			node->m_bones[j].indWeight[k].weight = file_read_float(abf);
			}
			*/
			node->m_bones[j].indWeight = reinterpret_cast<BoneIndWeight*>(file_read_implicit_array(abf, sizeof(BoneIndWeight) * node->m_bones[j].indWeightCount));
		}
	}
	else
	{
		node->m_armatureName = 0;
		node->m_weights = 0;
		node->m_matIdx = 0;
	}
}


void parse_nodeSkeleton1( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_SKELETON1);
	NodeSkeleton1* node = (NodeSkeleton1*)nodeBase;

	node->m_associatedMeshName		= file_read_string(abf);
	node->m_maxWeightsPerVertex		= file_read_uint(abf);
	node->m_boneCount				= file_read_uint(abf);
}

void parse_nodeHierarchy2( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_HIERARCHY2);

	NodeHierarchy2* node = (NodeHierarchy2*)nodeBase;

	node->m_parentName				= file_read_string(abf);
	node->m_boneCount				= file_read_uint(abf);

	/*unsigned int i;
	for (i = 0; i < node->m_boneCount; ++i)
	{
		Bone2 b;
		b.boneName					= file_read_string(abf);
		b.boneParentName			= file_read_string(abf);
		b.localXform				= file_read<float>(abf, 16);
		b.indWeight					= 0;
		b.indWeightCount			= 0;
		node->m_bones.push_back(b);
	}*/
}


void parse_nodeBone1( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_BONE1);
	NodeBone1* node = (NodeBone1*)nodeBase;

	node->m_offsetMatrix	= file_read<ArnMatrix>(abf);
	node->m_infVertexCount	= file_read_uint(abf);
	node->m_vertexIndices	= file_read_uint_array(abf, node->m_infVertexCount);
	node->m_weights			= file_read<float>(abf, node->m_infVertexCount);
}

void parse_nodeBone2( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_BONE2);
	NodeBone2* node = (NodeBone2*)nodeBase;

	node->m_parentBoneName	= file_read_string(abf);
	node->m_offsetMatrix	= file_read<ArnMatrix>(abf);

	/*node->m_infVertCount	= file_read_uint(abf);
	if (node->m_infVertCount)
		node->m_indWeightArray = file_read<BoneIndWeight>(abf, node->m_infVertCount);
	else
		node->m_indWeightArray = 0;*/
}

void parse_nodeHierarchy1( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_HIERARCHY1);
	NodeHierarchy1* node = (NodeHierarchy1*)nodeBase;

	node->m_frameCount		= file_read_uint(abf);
	if (node->m_frameCount > 0)
	{
		node->m_frames		= new MyFrameDataShell[node->m_frameCount];
		unsigned int i;
		for (i = 0; i < node->m_frameCount; ++i)
		{
			node->m_frames[i].m_frameName	= file_read_string(abf);
			node->m_frames[i].m_rootFlag	= file_read_BOOL(abf);
			node->m_frames[i].m_sibling		= file_read_int(abf);
			node->m_frames[i].m_firstChild	= file_read_int(abf);
		}
	}
	else
		throw MyError(MEE_HIERARCHY_FRAMES_ERROR);

}

void parse_nodeLight1( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_LIGHT1);
	NodeLight1* node = (NodeLight1*)nodeBase;

	node->m_light = file_read<ArnLightData>(abf);
}
void parse_nodeLight2( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_LIGHT2);
	NodeLight2* node = (NodeLight2*)nodeBase;

	node->m_parentName	= file_read_string(abf);
	node->m_ipoName		= file_read_string(abf);
	node->m_localXform	= file_read<ArnMatrix>(abf);
	node->m_light		= file_read<ArnLightData>(abf);
}

void parse_nodeCamera1( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_CAMERA1);
	NodeCamera1* node = (NodeCamera1*)nodeBase;

	node->m_camera = file_read<ARN_NDD_CAMERA_CHUNK>(abf);
}
void parse_nodeCamera2( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_CAMERA2);
	NodeCamera2* node = (NodeCamera2*)nodeBase;

	node->m_parentName	= file_read_string(abf);
	node->m_ipoName		= file_read_string(abf);
	node->m_localXform	= file_read<ArnMatrix>(abf);
	node->m_camType		= (NodeCamera2::CamType)file_read_int(abf);
	node->m_angle		= file_read_float(abf);
	node->m_clipStart	= file_read_float(abf);
	node->m_clipEnd		= file_read_float(abf);
	node->m_scale		= file_read_float(abf);
}
void parse_nodeAnim1(ArnBinaryFile& abf, NodeBase*& nodeBase)
{
	assert(nodeBase->m_ndt == NDT_ANIM1);
	NodeAnim1* node = (NodeAnim1*)nodeBase;

	node->m_keyCount = file_read_uint(abf);
	node->m_rstKeys = file_read<RST_DATA>(abf, node->m_keyCount);
}


void parse_nodeIpo1( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_IPO1);
	NodeIpo1* node = (NodeIpo1*)nodeBase;

	node->m_ipoCount = file_read_uint(abf);
}

void parse_nodeIpo2( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_IPO2);
	NodeIpo2* node = (NodeIpo2*)nodeBase;

	node->m_parentName = file_read_string(abf);
	node->m_curveCount = file_read_uint(abf);
	if (node->m_curveCount > 0)
	{
		node->m_curves = new CurveDataShell[node->m_curveCount];
		unsigned int i;
		for (i = 0; i < node->m_curveCount; ++i)
		{
			node->m_curves[i].name			= file_read_string(abf);
			node->m_curves[i].pointCount	= file_read_uint(abf);
			node->m_curves[i].type			= (CurveType)file_read_uint(abf);
			node->m_curves[i].points		= file_read<BezTripleData>(abf, node->m_curves[i].pointCount);
		}
	}
	else
	{
		node->m_curves = 0;
	}
}

void parse_nodeSymLink( ArnBinaryFile& abf, NodeBase*& nodeBase )
{
	assert(nodeBase->m_ndt == NDT_SYMLINK1);
	//NodeSymLink1* node = (NodeSymLink1*)nodeBase;
}

void parse_nodeAction1(ArnBinaryFile& abf, NodeBase*& nodeBase)
{
	assert(nodeBase->m_ndt == NDT_ACTION1);
	NodeAction1* node = (NodeAction1*)nodeBase;

	node->m_actionCount = file_read_uint(abf);
	if (node->m_actionCount)
	{
		node->m_actions.resize(node->m_actionCount);
		unsigned i, j;
		for (i = 0; i < node->m_actionCount; ++i)
		{
			node->m_actions[i].first = file_read_string(abf);
			const unsigned channelCount = file_read_uint(abf);
			assert(channelCount);
			node->m_actions[i].second.resize(channelCount);
			for (j = 0; j < channelCount; ++j)
			{
				node->m_actions[i].second[j].first = file_read_string(abf); // Global bone name
				node->m_actions[i].second[j].second = file_read_string(abf); // Global ipo name
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////

void release_arnfile( ArnFileData& afd )
{
	if (afd.m_fileDescriptor == 0) // afd was not initialized
		return;

	ArnFileData::NodeList::iterator it = afd.m_nodes.begin();
	for (; it != afd.m_nodes.end(); ++it)
	{
		delete (*it);
	}
	afd.m_nodes.clear();
	file_unload(afd.m_file);
}



//////////////////////////////////////////////////////////////////////////

void file_load( const TCHAR* fileName, ArnBinaryFile& file )
{
	FILE* f;
#ifdef WIN32
	f = _tfopen(fileName, _T("rb"));
#else
	f = fopen(fileName, "rb");
#endif
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
	delete [] file.m_data;
	file.m_data = 0;
}


char*			file_read_string(ArnBinaryFile& file) { char* c = file.m_data + file.m_curPos; file.m_curPos += strlen(c) + 1; return c; }
unsigned int	file_read_uint(ArnBinaryFile& file) { unsigned int ui = *(unsigned int*)(file.m_data + file.m_curPos); file.m_curPos += sizeof(unsigned int); return ui; }
unsigned int*	file_read_uint_array(ArnBinaryFile& file, unsigned int count) { unsigned int* uia = (unsigned int*)(file.m_data + file.m_curPos); file.m_curPos += sizeof(unsigned int) * count; return uia; }
int				file_read_int(ArnBinaryFile& file) { int i = *(int*)(file.m_data + file.m_curPos); file.m_curPos += sizeof(int); return i; }
float			file_read_float(ArnBinaryFile& file) { float f = *(float*)(file.m_data + file.m_curPos); file.m_curPos += sizeof(float); return f; }
BOOL			file_read_BOOL(ArnBinaryFile& file) { BOOL b = *(BOOL*)(file.m_data + file.m_curPos); if (b == TRUE || b == FALSE) { file.m_curPos += sizeof(BOOL); return b; } throw MyError(MEE_BOOL_DATA_PARSE); }
void*			file_read_implicit_array(ArnBinaryFile& file, unsigned int byteLen ) { void* ret = file.m_data + file.m_curPos; file.m_curPos += byteLen; return ret; }