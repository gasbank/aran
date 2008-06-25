/* 
 * load_arn.c
 * 2008 Geoyeob Kim (gasbank@gmail.com)
 */
#include "load_arn.h"
#include "Structs.h"

static int parse_mesh_data(char* arn_data, ArnMeshOb* mesh_ob)
{
	int pos = 0;
	if (arn_data[0] != ARNTYPE_MESH)
		return -1;
	mesh_ob->raw = arn_data;
	mesh_ob->hdr = (ArnMeshObHdr*)arn_data;
	pos += sizeof(ArnMeshObHdr);
	mesh_ob->vertex = (ArnVertex*)&arn_data[pos];
	pos += sizeof(ArnVertex) * mesh_ob->hdr->vertexCount;
	mesh_ob->faces = (int(*)[3])&arn_data[pos];
	pos += sizeof(int) * 3 * mesh_ob->hdr->faceCount;
	return pos;
}
static int parse_camera_data(char* arn_data, ArnMeshOb* mesh_ob)
{
	return 0;
}
static int parse_light_data(char* arn_data, ArnMeshOb* mesh_ob)
{
	return 0;
}

int load_arn(const char* filename, void* ob)
{
	FILE* f;
	int f_size;
	char* arn_data;
	int pos;
	int ret;
	f = fopen(filename, "rb");
	fseek(f, 0, SEEK_END);
	f_size = ftell(f);
	fseek(f, 0, SEEK_SET);
	arn_data = (char*)malloc(f_size);
	fread(arn_data, 1, f_size, f);
	fclose(f);
	pos = 0;
	switch (arn_data[pos])
	{
	case ARNTYPE_MESH:
		ret = parse_mesh_data(arn_data+pos, (ArnMeshOb*)ob);
		if (ret <= 0)
			return ret;
		else
			pos += ret;
		break;
	default:
		return -1;
	}
	return 0;
}

