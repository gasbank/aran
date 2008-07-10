/* 
 * load_arn.cpp
 * 2008 Geoyeob Kim (gasbank@gmail.com)
 */
#include "stdafx.h"
#include "load_arn.h"
#include "ArnMesh.h"
#include "ArnCamera.h"
#include "ArnLight.h"
#include "ArnMaterial.h"

static int parse_mesh_data(char* arn_data, ArnMeshOb* mesh_ob)
{
	int pos = 0;
	NODE_DATA_TYPE type = *(NODE_DATA_TYPE*)&arn_data[pos];
	if (type != NDT_MESH4)
		throw MyError(MEE_RTTI_INCONSISTENCY);
	mesh_ob->hdr = (ArnMeshObHdr*)arn_data;
	pos += sizeof(ArnMeshObHdr);
	mesh_ob->attrToMaterialMap = (DWORD*)&arn_data[pos];
	pos += sizeof(DWORD) * mesh_ob->hdr->materialCount;
	mesh_ob->vertex = (ArnVertex*)&arn_data[pos];
	pos += sizeof(ArnVertex) * mesh_ob->hdr->vertexCount;
	mesh_ob->faces = (unsigned short(*)[3])&arn_data[pos];
	pos += sizeof(unsigned short) * 3 * mesh_ob->hdr->faceCount;
	mesh_ob->attr = (DWORD*)&arn_data[pos];
	pos += sizeof(DWORD) * mesh_ob->hdr->faceCount;
	return pos;
}
static int parse_camera_data(char* arn_data, ArnCameraOb* cam_ob)
{
	int pos = 0;
	NODE_DATA_TYPE type = *(NODE_DATA_TYPE*)&arn_data[pos];
	if (type != NDT_CAMERA2)
		return -1;
	cam_ob->hdr = (ArnCameraObHdr*)&arn_data[pos];
	pos += sizeof(ArnCameraObHdr);
	return pos;
}
static int parse_light_data(char* arn_data, ArnLightOb* light_ob)
{
	int pos = 0;
	NODE_DATA_TYPE type = *(NODE_DATA_TYPE*)&arn_data[pos];
	if (type != NDT_LIGHT2)
		return -1;
	light_ob->hdr = (ArnLightObHdr*)&arn_data[pos];
	pos += sizeof(ArnLightObHdr);
	light_ob->d3dLight = (D3DLIGHT9*)&arn_data[pos];
	pos += sizeof(D3DLIGHT9);
	return pos;
}
static int parse_material_data(char* arn_data, ArnMaterialOb* material_ob)
{
	int pos = 0;
	NODE_DATA_TYPE type = *(NODE_DATA_TYPE*)&arn_data[pos];
	if (type != NDT_MATERIAL)
		return -1;
	material_ob->hdr = (ArnMaterialObHdr*)&arn_data[pos];
	pos += sizeof(ArnMaterialObHdr);
	return pos;
}
int load_arn(const char* filename, std::vector<ArnObject*>& objects)
{
	FILE* f;
	int f_size;
	char* arn_data;
	int pos;
	int ret;
	int n = 0;
	fopen_s(&f, filename, "rb");
	if (!f)
		throw std::runtime_error("File open failure");
	fseek(f, 0, SEEK_END);
	f_size = ftell(f);
	fseek(f, 0, SEEK_SET);
	arn_data = new char[f_size];
	fread(arn_data, 1, f_size, f);
	fclose(f);
	pos = 0;
	while (true)
	{
		NODE_DATA_TYPE type = *(NODE_DATA_TYPE*)&arn_data[pos];
		if (type == NDT_MESH4)
		{
			ArnMesh* mesh = new ArnMesh(type);
			ret = parse_mesh_data(arn_data+pos, &mesh->getOb());
			objects.push_back(mesh);
			if (ret <= 0)
				return ret;
			else
				pos += ret;
			++n;
		}
		else if (type == NDT_CAMERA2)
		{
			ArnCamera* cam = new ArnCamera;
			ret = parse_camera_data(arn_data+pos, &cam->getOb());
			objects.push_back(cam);
			if (ret <= 0)
				return ret;
			else
				pos += ret;
			++n;
		}
		else if (type == NDT_LIGHT2)
		{
			ArnLight* light = new ArnLight;
			ret = parse_light_data(arn_data+pos, &light->getOb());
			objects.push_back(light);
			if (ret <= 0)
				return ret;
			else
				pos += ret;
			++n;
		}
		else if (type == NDT_MATERIAL)
		{
			ArnMaterial* material = new ArnMaterial;
			ret = parse_material_data(arn_data+pos, &material->getOb());
			objects.push_back(material);
			if (ret <= 0)
				return ret;
			else
				pos += ret;
			++n;
		}
		else
		{
			break;
		}
		if (f_size <= pos)
			break;
	}
	if (f_size != pos)
		throw std::runtime_error("File loading inconsistency");

	size_t i, j;
	for (i = 0; i < objects.size(); ++i)
	{
		if (objects[i]->getType() == NDT_MESH4)
		{
			ArnMesh* mesh = (ArnMesh*)objects[i];
			const char* parName = mesh->getOb().hdr->parName;
			if (strlen(parName))
			{
				ArnNode* node = 0;
				for (j = 0; j < objects.size(); ++j)
				{
					const char* obName = objects[j]->getName();
					if (strcmp(obName, parName) == 0)
					{
						ASSERTCHECK(objects[i]->getType() != NDT_MATERIAL);
						mesh->setParent((ArnNode*)objects[j]);
						break;
					}
				}
			}
		}
	}
	return 0;
}

