#pragma once

#include "NodeExporter.h"

class ArMesh;
class Remap;

class MeshExporter : public NodeExporter
{
public:
	MeshExporter(IGameNode* node, IGameMesh* mesh, ArMesh*& arMesh);
	~MeshExporter(void);

	virtual void make();

private:

	void exportMaterial(IGameMaterial* material);
	void exportFaceVertices(unsigned int faceIndex, FaceEx* faceEx);
	void exportFace2MaterialMap(unsigned int faceIndex, FaceEx* faceEx);
	void exportRemappingData();
	

	// export source and target
	IGameMesh* m_mesh;
	ArMesh* m_arMesh;

	// export helper
	Remap* m_remap;
	
};
