#pragma once
#include "arnode.h"

struct ArFullVertex;
class ArMaterial;

class ArMesh :
	public ArNode
{
public:
	typedef unsigned int							FaceIndex;
	typedef std::map<std::string, ArMaterial*>		MaterialList;

	struct ArFace {
		ArFace(const DWORD* indices, const ArMaterial* mat)
			: material(mat)
		{
			vertexIndices[0] = indices[0];
			vertexIndices[1] = indices[1];
			vertexIndices[2] = indices[2];
		}
		DWORD vertexIndices[3];
		const ArMaterial* material;
	};

	ArMesh(void);
	ArMesh(const char* name) : ArNode(name) {}
	virtual ~ArMesh(void);

	size_t getFaceCount() const { return m_faces.size(); }

	const ArMaterial* getMaterialByName(const char* name) const;

	void insertMaterial(ArMaterial* material); // return true if inserted
	bool insertVertex(ArFullVertex* vertex);
	void setFace2MaterialMap(FaceIndex faceIndex, const ArMaterial* material);
	void addFace(ArFace* face) { m_faces.push_back(face); }

	size_t getVertexCount() const { return m_fullVertices.size(); }
	ArFullVertex* getVertexAt(size_t i) { return m_fullVertices[i]; }
	
	
private:
	void removeAllFaces();
	void removeAllFullVertices();
	void removeAllMaterials();
	
	std::vector<DWORD> m_indices;
	std::vector<ArFace*> m_faces;
	std::vector<ArFullVertex*> m_fullVertices;
	MaterialList m_materials;
		
	// EXPORT HELPER VARIABLES
	// DO NOT MODIFIY!!!
	typedef std::pair<float, DWORD> FloatDwordPair;
	typedef std::vector<FloatDwordPair> FloatDwordPairVector;
	FloatDwordPairVector m_acc; // export accelerator
public:
	// EXPORT HELPER METHODS
	// DO NOT MODIFIY!!!
	
};
