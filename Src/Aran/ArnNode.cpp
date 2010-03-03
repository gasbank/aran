#include "AranPCH.h"
#include "ArnNode.h"

ArnNode::ArnNode(NODE_DATA_TYPE type)
: ArnObject(type)
, m_parent()
{
}

ArnNode::~ArnNode(void)
{
	deleteAllChildren();
	detachParent();
}

void
ArnNode::attachChild( ArnNode* child )
{
	child->detachParent();
	m_children.push_back(child);
	child->setParentName(getName());
	child->setParent(this);
}

void
ArnNode::attachChildToFront( ArnNode* child )
{
	child->detachParent();
	m_children.push_front(child);
	child->setParentName(getName());
	child->setParent(this);
}

void
ArnNode::detachChild( ArnNode* child )
{
	m_children.remove(child);
	child->setParent(0);
}

void
ArnNode::deleteAllChildren()
{
	const size_t totalChildCount = m_children.size();
	size_t deletedChildCount = 0;
	while (m_children.size())
	{
		ArnNodeList::iterator it = m_children.begin();
		delete (*it);
		++deletedChildCount;
		if (deletedChildCount > totalChildCount)
		{
			ARN_THROW_UNEXPECTED_CASE_ERROR
		}
	}
}

ArnNode*
ArnNode::getLastNode()
{
        ArnNodeList::reverse_iterator it = m_children.rbegin();
	if (it != m_children.rend())
		return *it;
	else
		return 0;
}

ArnNode*
ArnNode::getNodeByName(const std::string& name)
{
	if (getName() == name)
		return this;

	ArnNodeList::const_iterator it = m_children.begin();
	for (; it != m_children.end(); ++it)
	{
		ArnNode* ret = (*it)->getNodeByName(name);
		if (ret != 0)
			return ret;
	}

	return 0;
}

const ArnNode*
ArnNode::getConstNodeByName(const std::string& name) const
{
	if (getName() == name)
		return this;

	ArnNodeList::const_iterator it = m_children.begin();
	for (; it != m_children.end(); ++it)
	{
		const ArnNode* ret = (*it)->getConstNodeByName(name);
		if (ret != 0)
			return ret;
	}

	return 0;
}

ArnNode*
ArnNode::getNodeAt( unsigned int idx ) const
{
	if (idx < m_children.size())
	{
		ArnNodeList::const_iterator it = m_children.begin();
		while (idx)
		{
			++it;
			--idx;
		}
		return (*it);
	}
	else
		throw MyError(MEE_STL_INDEX_OUT_OF_BOUNDS);
}

ArnNode*
ArnNode::getNodeById(unsigned int id)
{
	if (getObjectId() == id)
		return this;
	foreach (ArnNode* n, m_children)
	{
		if (n->getNodeById(id))
			return n->getNodeById(id);
	}
	return 0;
}

const ArnNode*
ArnNode::getConstNodeById( unsigned int id ) const
{
	if (getObjectId() == id)
		return this;
	foreach (const ArnNode* n, m_children)
	{
		if (n->getConstNodeById(id))
			return n->getConstNodeById(id);
	}
	return 0;
}

void
ArnNode::interconnect( ArnNode* sceneRoot )
{
	ArnNodeList::iterator it = m_children.begin();
	for (; it != m_children.end(); ++it)
	{
		(*it)->interconnect(sceneRoot);
	}
}

void
ArnNode::update( double fTime, float fElapsedTime )
{
	ArnNodeList::iterator it = m_children.begin();
	for (; it != m_children.end(); ++it)
	{
		(*it)->update(fTime, fElapsedTime);
	}
}

void
ArnNode::printNodeHierarchy(int depth) const
{
	ArnNodeList::const_iterator cit = m_children.begin();
	for (; cit != m_children.end(); ++cit)
	{
		for (int i = 0; i < depth; ++i)
			printf("     ");
		printf("%s\n", (*cit)->getName());
		(*cit)->printNodeHierarchy(depth + 1);
	}
}

const ArnRenderableObject*
ArnNode::getRenderableObject() const
{
	foreach(ArnNode* n, m_children)
	{
		if (n->getType() == NDT_RT_RENDERABLEOBJECT)
			return reinterpret_cast<const ArnRenderableObject*>(n);
	}
	return 0;
}

int
ArnNode::getIndexOfChild(const ArnNode* c) const
{
	int ret = 0;
	foreach (const ArnNode* n, getChildren())
	{
		if (n == c)
			break;
		++ret;
	}
	if (ret == getChildren().size())
		return -1;
	else
		return ret;
}

ArnNode*
ArnNode::findFirstNodeOfType( NODE_DATA_TYPE ndt )
{
	foreach (ArnNode* node, getChildren())
	{
		if (node->getType() == ndt)
		{
			return node;
		}
	}
	return 0;
}

void ArnNode::detachRenderableObjects()
{
	ArnNodeList toBeDeleted;
	foreach (ArnNode *renderable, m_children)
	{
		if (renderable->getType() == NDT_RT_RENDERABLEOBJECT)
			toBeDeleted.push_back(renderable);
	}
	foreach (ArnNode *renderable, toBeDeleted)
	{
		delete renderable;
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void ArnGetNameFromRtNdt(char out[64], NODE_DATA_TYPE ndt)
{
	const char* name;
	switch (ndt)
	{
		case NDT_RT_CONTAINER: name = "Container"; break;
		case NDT_RT_MESH: name = "Mesh"; break;
		case NDT_RT_CAMERA: name = "Camera"; break;
		case NDT_RT_LIGHT: name = "Light"; break;
		case NDT_RT_ANIM: name = "Animation"; break;
		case NDT_RT_MATERIAL: name = "Material"; break;
		case NDT_RT_HIERARCHY: name = "Hierarchy"; break;
		case NDT_RT_SKELETON: name = "Skeleton"; break;
		case NDT_RT_BONE: name ="Bone"; break;
		case NDT_RT_IPO: name = "IPO"; break;
		case NDT_RT_ACTIONS: name = "Actions"; break;
		case NDT_RT_SYMLINK: name = "Symlink"; break;
		case NDT_RT_SCENEGRAPH: name = "SceneGraph"; break;
		case NDT_RT_TEXTURE: name = "Texture"; break;
		case NDT_RT_RENDERABLEOBJECT: name = "RenderableObject"; break;
		case NDT_RT_BINARYCHUNK: name = "BinaryChunk"; break;
		case NDT_RT_IKNODE: name = "IkNode"; break;
		case NDT_RT_GENERALBODY: name = "GeneralBody"; break;
		case NDT_RT_GENERALJOINT: name = "GeneralJoint"; break;
		default: name = "Unknown";
	}
	strncpy(out, name, 64);
}

void ArnPrintNodes(const ArnNode* n, int step)
{
    if (n)
    {
        for (int i = 0; i < step; ++i)
            std::cout << "    ";
        std::cout << n->getName() << std::endl;

        foreach (ArnNode* c, n->getChildren())
        {
            ArnPrintNodes(c, step+1);
        }
    }
}

void ArnDetachRenderableObjects( ArnNode* node )
{
	if (node)
		node->detachRenderableObjects ();
	foreach (ArnNode *n, node->getChildren ())
		ArnDetachRenderableObjects (n);
}
