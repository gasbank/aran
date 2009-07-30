ArnNode* ArnNode::getParent() const
{
	return m_parent;
}

const std::string& ArnNode::getParentName() const
{
	return m_parentName;
}

const char* ArnNode::getName() const
{
	return m_name.c_str();
}

void ArnNode::setName(const char* name)
{
	m_name = name;
}

unsigned int ArnNode::getNodeCount() const
{
	return m_children.size();
}

ArnNode* ArnNode::getSceneRoot()
{
	if (m_parent)
		return m_parent->getSceneRoot();
	else
		return this;
}

const ArnNodeList& ArnNode::getChildren() const
{
	return m_children;
}

void ArnNode::setParent(ArnNode* node)
{
	m_parent = node;
}

void ArnNode::detachParent()
{
	if (m_parent)
		m_parent->detachChild(this);
}

void ArnNode::setParentName(const char* name)
{
	m_parentName = name;
}
