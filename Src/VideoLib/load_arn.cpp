/* 
 * load_arn.cpp
 * 2008 Geoyeob Kim (gasbank@gmail.com)
 */
#include "stdafx.h"
#include "load_arn.h"
#include "Structs.h"
#include "ArnMesh.h"
#include "ArnCamera.h"
#include "ArnLight.h"
#include "ArnMaterial.h"

static int parse_mesh_data(char* arn_data, ArnMeshOb* mesh_ob)
{
	int pos = 0;
	ArnNodeType type = *(ArnNodeType*)&arn_data[pos];
	if (type != ANT_MESH)
		return -1;
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
	ArnNodeType type = *(ArnNodeType*)&arn_data[pos];
	if (type != ANT_CAMERA)
		return -1;
	cam_ob->hdr = (ArnCameraObHdr*)&arn_data[pos];
	pos += sizeof(ArnCameraObHdr);
	return pos;
}
static int parse_light_data(char* arn_data, ArnLightOb* light_ob)
{
	int pos = 0;
	ArnNodeType type = *(ArnNodeType*)&arn_data[pos];
	if (type != ANT_LIGHT)
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
	ArnNodeType type = *(ArnNodeType*)&arn_data[pos];
	if (type != ANT_MATERIAL)
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
	f = fopen(filename, "rb");
	fseek(f, 0, SEEK_END);
	f_size = ftell(f);
	fseek(f, 0, SEEK_SET);
	arn_data = new char[f_size];
	fread(arn_data, 1, f_size, f);
	fclose(f);
	pos = 0;
	while (true)
	{
		ArnNodeType type = *(ArnNodeType*)&arn_data[pos];
		if (type == ANT_MESH)
		{
			ArnMesh* mesh = new ArnMesh;
			ret = parse_mesh_data(arn_data+pos, &mesh->getOb());
			objects.push_back(mesh);
			if (ret <= 0)
				return ret;
			else
				pos += ret;
			++n;
		}
		else if (type == ANT_CAMERA)
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
		else if (type == ANT_LIGHT)
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
		else if (type == ANT_MATERIAL)
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
		if (objects[i]->getType() == ANT_MESH)
		{
			ArnMesh* mesh = (ArnMesh*)objects[i];
			const char* parName = mesh->getOb().hdr->parName;
			if (strlen(parName))
			{
				ArnNode* node = NULL;
				for (j = 0; j < objects.size(); ++j)
				{
					const char* obName = objects[j]->getName();
					if (strcmp(obName, parName) == 0)
					{
						ASSERTCHECK(objects[i]->getType() != ANT_MATERIAL);
						mesh->setParent((ArnNode*)objects[j]);
						break;
					}
				}
			}
		}
	}
	return 0;
}

