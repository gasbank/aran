#pragma once

void load_arnfile(const TCHAR* fileName, ArnFileData& afd);
void release_arnfile(ArnFileData& afd);

void parse_node(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeUnidentified(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeMaterial1(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeMaterial2(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeMesh2(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeMesh3(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeSkeleton1(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeHierarchy2(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeHierarchy1(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeLight1(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeLight2(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeCamera1(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeCamera2(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeAnim1(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeBone1(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeBone2(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeIpo1(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeIpo2(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeAction1(ArnBinaryFile& abf, NodeBase*& nodeBase);
void parse_nodeSymLink(ArnBinaryFile& abf, NodeBase*& nodeBase);

//////////////////////////////////////////////////////////////////////////

void			file_load(const TCHAR* fileName, ArnBinaryFile& file);
void			file_unload(ArnBinaryFile& file);


char*			file_read_string(ArnBinaryFile& file);
unsigned int	file_read_uint(ArnBinaryFile& file);
unsigned int*	file_read_uint_array(ArnBinaryFile& file, unsigned int count);
int				file_read_int(ArnBinaryFile& file);
float			file_read_float(ArnBinaryFile& file);
BOOL			file_read_BOOL(ArnBinaryFile& file);
void*			file_read_implicit_array(ArnBinaryFile& file, unsigned int byteLen);

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
