const MeshData& ArnMesh::getMeshData() const
{
	return m_data;
}

ArnMaterial* ArnMesh::getMaterialNode(unsigned int i) const
{
	return m_materialRefList[i];
}

bool ArnMesh::isCollide() const
{
	return m_bCollide;
}

void ArnMesh::setCollide(bool val)
{
	m_bCollide = val;
}

const NodeMesh3* ArnMesh::getNodeMesh3() const
{
	return m_nodeMesh3;
}
