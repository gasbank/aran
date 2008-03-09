#include "stdafx.h"
#include "ModelExporter.h"
#include "ModelExporter2.h"
#include "MeshExporter.h"
#include "ArMesh.h"
#include "ArMaterial.h"
#include "MaterialExporter.h"
#include "Remap.h"

MeshExporter::MeshExporter(IGameNode* node, IGameMesh* mesh, ArMesh*& arMesh)
: NodeExporter(node)
{
	m_mesh = mesh;
	m_arMesh = arMesh;
	m_remap = new Remap;
}

MeshExporter::~MeshExporter(void)
{
	SAFE_DELETE(m_remap);
}

void MeshExporter::make()
{
	const ModelExporter2& exporter = ModelExporter2::getSingleton();

	GMatrix gm;
	IGameNode* nodeParent = getNode()->GetNodeParent();
	if (nodeParent)
	{
		std::string nodeParentName(nodeParent->GetName());
		gm = getNode()->GetNodeParent()->GetWorldTM().Inverse();
		m_arMesh->setParent(exporter.getNodeByName(nodeParentName));
	}
	else
	{
		// scene root!
		m_arMesh->setParent(NULL);
		gm.SetIdentity();
	}
	gm = gm * getNode()->GetWorldTM();
	
	ArMatrix4 mat4(gm);
	m_arMesh->setWorldMatrix(mat4);

	// MeshExporter make() step
	//
	// (1) Initialize IGameMesh*
	// (2) If init failed then set face count to zero and exit
	// (3) Get face count
	// (4) Iterate faces
	//    a. Make material
	//    b. Make three vertices, vertex index
	//    c. Make face to material map

	if (!m_mesh->InitializeData())
	{
		// Not a general geometry(i.e. has no faces)
		return;
	}

	int faceCount = m_mesh->GetNumberOfFaces();
	_ASSERT(faceCount);

	m_mesh->SetCreateOptimizedNormalList();

	int i;
	for (i = 0; i < faceCount; ++i)
	{
		FaceEx* faceEx = m_mesh->GetFace(i);
		IGameMaterial* material = m_mesh->GetMaterialFromFace(faceEx);

		if (material)
			exportMaterial(material);
		
		exportFaceVertices(i, faceEx);
		
		exportFace2MaterialMap(i, faceEx);
	}

	exportRemappingData();
}

void MeshExporter::exportMaterial( IGameMaterial* material )
{
	// check for duplicate
	if (!m_arMesh->getMaterialByName(material->GetMaterialName()))
	{
		ArMaterial* arMaterial = new ArMaterial;

		MaterialExporter* materialExporter = new MaterialExporter(material, arMaterial);
		materialExporter->make();
		SAFE_DELETE(materialExporter);

		m_arMesh->insertMaterial(arMaterial);
	}
}

void MeshExporter::exportFaceVertices(unsigned int faceIndex, FaceEx* faceEx)
{
	// three vertices per face
	int i;
	
	for (i = 0; i < 3; i++)
	{	
		ArFullVertex* fv = new ArFullVertex;
		Point3 pos = AranUtil::TransformVector4(getNode()->GetWorldTM().Inverse(), Point4(m_mesh->GetVertex(faceEx->vert[i], false), 1.0f));
		//Point3 pos = m_mesh->GetVertex(faceEx->vert[i], false);
		Point3 nor = m_mesh->GetNormal(faceEx->norm[i], false);
		Point2 texCoord = m_mesh->GetTexVertex(faceEx->texCoord[i]);

		fv->origVert = faceEx->vert[i]; // vert id
		fv->pos.x = pos.x;
		fv->pos.y = pos.y;
		fv->pos.z = pos.z;
		fv->normal.x = nor.x;
		fv->normal.y = nor.y;
		fv->normal.z = nor.z;
		fv->uvw.x = texCoord.x;
		fv->uvw.y = texCoord.y;
		fv->uvw.z = 0.0f;

#if 0
		DebugPrint(
			_T( "(%.2f,%.2f,%.2f), (%.2f,%.2f,%.2f), (%.2f,%.2f,%.2f)\n" ),
			pos.x, pos.y, pos.z, nor.x, nor.y, nor.z, texCoord.x, texCoord.y, 0.0f);
#endif

		if (!m_arMesh->insertVertex(fv))
		{
			// insert rejected since duplicative
			SAFE_DELETE(fv);
		}
	}
}

void MeshExporter::exportFace2MaterialMap(unsigned int faceIndex, FaceEx* faceEx)
{
	IGameMaterial* igmat = m_mesh->GetMaterialFromFace(faceEx);
	if (igmat)
	{
		const ArMaterial* material = m_arMesh->getMaterialByName(igmat->GetMaterialName());
		if (igmat && material)
		{
			m_arMesh->setFace2MaterialMap(faceIndex, material);
		}
	}
}

void MeshExporter::exportRemappingData()
{
	//  Calculate mapping from old vertex index to new vertex indices
	ZeroMemory(m_remap, sizeof(Remap));
	size_t vertexCount = m_arMesh->getVertexCount();
	if (vertexCount > 0)
	{
		int mo = -1;
		size_t i;
		for (i = 0; i < vertexCount; ++i)
		{
			if (mo < m_arMesh->getVertexAt(i)->origVert)
			{
				mo = m_arMesh->getVertexAt(i)->origVert;
			}
		}
		m_remap->SetOldCount(mo + 1);
		for (i = 0; i < (std::size_t)vertexCount; ++i)
		{
			m_remap->MapOldToNew(m_arMesh->getVertexAt(i)->origVert, (int)i);
		}
	}
}