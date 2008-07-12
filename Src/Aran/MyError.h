// MyError.h
// 2008 Geoyeob Kim (gasbank@gmail.com)
#pragma once

enum MyErrorEnum
{
	MEE_FILEDESCRIPTOR_CORRUPTED,
	MEE_TERMINALSTRING_CORRUPTED,
	MEE_UNSUPPORTED_NODE,
	MEE_UNDEFINED_ERROR,
	MEE_SKELETON_BONES_ERROR,
	MEE_HIERARCHY_FRAMES_ERROR,
	MEE_BOOL_DATA_PARSE,
	MEE_NOT_A_MESH_TYPE,
	MEE_RTTI_INCONSISTENCY,
	MEE_DEVICE_NOT_READY,
	MEE_NOT_IMPLEMENTED_YET,
	MEE_ANIM1NODE_CORRUPTED,
	MEE_SKELETON1NODE_CORRUPTED,
	MEE_NODE_NOT_FOUND,
	MEE_UNSUPPORTED_ARNFILE,
	MEE_STL_INDEX_OUT_OF_BOUNDS,
};

class MyError
{
public:
	MyError(MyErrorEnum lee)
	{
		switch (lee)
		{
		case MEE_FILEDESCRIPTOR_CORRUPTED:	m_str = "File descriptor in ARN file is corrupted";
		case MEE_TERMINALSTRING_CORRUPTED:	m_str = "Terminal string in ARN file is corrupted";
		case MEE_UNDEFINED_ERROR:			m_str = "Undefined error occurred!";
		case MEE_UNSUPPORTED_NODE:			m_str = "Unsupported or not implemented node detected; skip this node...";
		case MEE_SKELETON_BONES_ERROR:		m_str = "Skeleton node's bone count error";
		case MEE_HIERARCHY_FRAMES_ERROR:	m_str = "Hierarchy node's MyFrame count error";
		case MEE_BOOL_DATA_PARSE:			m_str = "Error while parsing BOOL data type from ARN file";
		case MEE_NOT_A_MESH_TYPE:			m_str = "Non-mesh type enum was set at mesh type object";
		case MEE_RTTI_INCONSISTENCY:		m_str = "Enum based RTTI inconsistent! Type-conversion failed";
		case MEE_DEVICE_NOT_READY:			m_str = "Direct3D Renderer device is not ready";
		case MEE_NOT_IMPLEMENTED_YET:		m_str = "Reached to unimplemented area";
		case MEE_ANIM1NODE_CORRUPTED:		m_str = "Anim1 data corrupted";
		case MEE_SKELETON1NODE_CORRUPTED:	m_str = "Skeleton1 data corrupted";
		case MEE_NODE_NOT_FOUND:			m_str = "Desired node not found";
		case MEE_UNSUPPORTED_ARNFILE:		m_str = "Unsupported ARN file format";
		case MEE_STL_INDEX_OUT_OF_BOUNDS:	m_str = "STL data structure: Index out of bounds";
		default:							m_str = "<Should not see me! Never!>";
		}
	}
private:
	char* m_str;
};
