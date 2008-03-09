#include "stdafx.h"
#include "ArMesh.h"
#include "ArMaterial.h"

ArMesh::ArMesh(void)
{
}

ArMesh::~ArMesh(void)
{
	removeAllFaces();
	removeAllFullVertices();
	removeAllMaterials();
}

const ArMaterial* ArMesh::getMaterialByName( const char* name ) const
{
	const std::string str(name);
	MaterialList::const_iterator cit = m_materials.find(str);
	if (cit != m_materials.end())
		return cit->second;
	else
		return NULL;
}

void ArMesh::insertMaterial( ArMaterial* material )
{
	m_materials[material->getName()] = material;
}

void ArMesh::removeAllFaces()
{
	size_t i;
	for (i = 0; i < m_faces.size(); ++i)
	{
		delete m_faces[i];
	}
	m_faces.clear();
}

void ArMesh::removeAllFullVertices()
{
	size_t i;
	for (i = 0; i < m_fullVertices.size(); ++i)
	{
		delete m_fullVertices[i];
	}
	m_fullVertices.clear();
}

void ArMesh::removeAllMaterials()
{
	MaterialList::iterator it = m_materials.begin();
	for (; it != m_materials.end(); ++it)
	{
		delete it->second;
	}
	m_materials.clear();
}

bool ArMesh::insertVertex( ArFullVertex* vertex )
{
	size_t n = m_fullVertices.size();
	size_t i = 0;
	FloatDwordPair p(vertex->pos.x, 0);
	FloatDwordPair* base = NULL;

	if (m_acc.size() > 0) {
		std::pair<float, DWORD>* low = &m_acc[0];
		std::pair<float, DWORD>* high = low+m_acc.size();
		base = low;
		for (;;) {
			FloatDwordPair* mid = low + (high-low)/2; // get the middle of low-high range of 'm_acc'
			if (mid == low) { // mid and low point to the same
				i = mid-&m_acc[0]; // set i to the mid's index and break the infinite for loop
				break;
			}

			// make range narrower by comparing x value(.first) of mid and p
			if (mid->first < p.first) {
				low = mid;
			}
			else {
				high = mid;
			}
		}
	}


	for (; i < n; ++i) {
		if (m_fullVertices[base[i].second]->pos.x > vertex->pos.x) {
			//  no need searching more
			i = base[i].second;
			break;
		}
		if (!memcmp(m_fullVertices[base[i].second], vertex, sizeof(ArFullVertex))) {
			m_indices.push_back(base[i].second);
			return false;
		}
	}

	p.second = (DWORD)m_fullVertices.size(); // set p.second to current m_fullVertices's size


#if !defined(NDEBUG)
	// check for duplicated vertex (should not exist)
	size_t nv = m_fullVertices.size();
	for (size_t i = 0; i < nv; ++i) {
		_ASSERT( memcmp(vertex, m_fullVertices[i], sizeof(ArFullVertex)) );
	}
#endif

	m_acc.push_back(p);
	std::inplace_merge(&m_acc[0], &m_acc[m_acc.size()-1], &m_acc[0]+m_acc.size());
	m_fullVertices.push_back(vertex);

	m_indices.push_back(p.second);
	return true;
}

void ArMesh::setFace2MaterialMap(FaceIndex faceIndex, const ArMaterial* material)
{
	//m_face2MaterialMap[faceIndex] = material;
	m_faces.push_back(new ArFace(&m_indices[m_indices.size()-3], material));
}

