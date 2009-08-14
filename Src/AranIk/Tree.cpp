#include "AranIkPCH.h"
#include "LinearR3.h"
#include "Tree.h"
#include "Node.h"

Tree::Tree()
: m_nNode(0)
, m_nEffector(0)
, m_nJoint(0)
{
}

void Tree::SetSeqNum(NodePtr node)
{
	switch (node->getPurpose()) {
	case JOINT:
		node->setSeqNumJoint(m_nJoint);
		m_nJoint++;
		node->setSeqNumEffector(-1);
		break;
	case EFFECTOR:
		node->setSeqNumJoint(-1);
		node->setSeqNumEffector(m_nEffector);
		m_nEffector++;
		break;
	}
}

void Tree::InsertRoot(NodePtr root)
{
	assert( m_nNode==0 );
	m_nNode++;
	Tree::m_root = root;
	root->setR(root->getAttach());
	assert( !(root->getLeftNode() || root->getRightNode()) );
	SetSeqNum(root);
}

void Tree::InsertLeftChild(NodePtr parent, NodePtr child)
{
	assert(parent);
	m_nNode++;
	parent->setLeftNode(child);
	child->setRealParent(parent);
	child->setR(child->getAttach() - child->getRealParent()->getAttach());
	assert( !(child->getLeftNode() || child->getRightNode()) );
	SetSeqNum(child);
}

void Tree::InsertRightSibling(NodePtr parent, NodePtr child)
{
	assert(parent);
	m_nNode++;
	parent->setRightNode(child);
	child->setRealParent(parent->getRealParent());
	child->setR(child->getAttach() - child->getRealParent()->getAttach());
	assert( !(child->getLeftNode() || child->getRightNode()) );
	SetSeqNum(child);
}

// Search recursively below "node" for the node with index value.
NodePtr Tree::SearchJoint(NodePtr node, int index)
{
	NodePtr ret;
	if (node != 0) {
		if (node->getSeqNumJoint() == index) {
			return node;
		} else {
			ret = SearchJoint(node->getLeftNode(), index);
			if (ret) {
				return ret;
			}
			ret = SearchJoint(node->getRightNode(), index);
			if (ret) {
				return ret;
			}
			return NodePtr();
		}
	}
	else {
		return NodePtr();
	}
}


// Get the joint with the index value
NodePtr Tree::GetJoint(int index)
{
	return SearchJoint(m_root, index);
}

// Search recursively below node for the end effector with the index value
NodePtr Tree::SearchEffector(NodePtr node, int index)
{
	NodePtr ret;
	if (node != 0) {
		if (node->getSeqNumEffector() == index) {
			return node;
		} else {
			ret = SearchEffector(node->getLeftNode(), index);
			if (ret) {
				return ret;
			}
			ret = SearchEffector(node->getRightNode(), index);
			if (ret) {
				return ret;
			}
			return NodePtr();
		}
	} else {
		return NodePtr();
	}
}


// Get the end effector for the index value
NodePtr Tree::GetEffector(int index)
{
	return SearchEffector(m_root, index);
}

// Returns the global position of the effector.
const VectorR3& Tree::GetEffectorPosition(int index)
{
	NodePtr effector = GetEffector(index);
	assert(effector);
	return (effector->GetS());
}

void Tree::ComputeTree(NodePtr node)
{
	if (node != 0) {
		node->ComputeS();
		node->ComputeW();
		ComputeTree(node->getLeftNode());
		ComputeTree(node->getRightNode());
	}
}

void Tree::Compute(void)
{
	ComputeTree(m_root);
}

void Tree::PrintTree(NodeConstPtr node) const
{
	if (node)
	{
		node->PrintNode();
		PrintTree(node->getLeftNode());
		PrintTree(node->getRightNode());
	}
}

void Tree::Print(void)
{
	PrintTree(m_root);
	cout << "\n";
}

// Recursively initialize tree below the node
void Tree::InitTree(NodePtr node)
{
	if (node)
	{
		node->InitNode();
		InitTree(node->getLeftNode());
		InitTree(node->getRightNode());
	}
}

// Initialize all nodes in the tree
void Tree::Init(void)
{
	InitTree(m_root);
}

void Tree::UnFreezeTree(NodePtr node)
{
	if (node != 0) {
		node->UnFreeze();
		UnFreezeTree(node->getLeftNode());
		UnFreezeTree(node->getRightNode());
	}
}

void Tree::UnFreeze(void)
{
	UnFreezeTree(m_root);
}

void Tree::printHierarchy() const
{
	m_root->printNodeHierarchy(0);
}

bool Tree::hasNode(const NodeConstPtr node) const
{
	if (m_root == node)
		return true;
	else
		return m_root->hasNode(node);
}

NodePtr Tree::getPrevSiblingNode(NodePtr node)
{
	NodePtr realParent = node->getRealParent();
	if (!realParent)
		return NodePtr();
	NodePtr prevSibling = realParent->getLeftNode();
	while (prevSibling && prevSibling->getRightNode() != node)
	{
		prevSibling = prevSibling->getRightNode();
	}
	return prevSibling;
}

void Tree::InsertCopiedNodesBySwitchingRoot(NodePtr prevInsertedNode, NodePtr node, bool childOrSibling /* false = child, true = sibling */, NodePtr skipNode)
{
	assert(node);

	if (node == skipNode)
		return;

	NodePtr createdNode = Node::createCloneWithoutLink(node);

	if (!prevInsertedNode && !m_root)
	{
		InsertRoot(createdNode);
	}
	else if (prevInsertedNode && !childOrSibling)
	{
		// Insert child to 'prevInsertedNode'.
		NodePtr leftNode = prevInsertedNode->getLeftNode();
		if (!leftNode)
		{
			InsertLeftChild(prevInsertedNode, createdNode);
		}
		else
		{
			NodePtr lastChild = leftNode;
			while (lastChild->getRightNode())
			{
				lastChild = lastChild->getRightNode();
			}
			InsertRightSibling(lastChild, createdNode);
		}
	}
	else if (prevInsertedNode && childOrSibling)
	{
		InsertRightSibling(prevInsertedNode, createdNode);
	}

	NodePtr child = node->getLeftNode();
	if (child)
	{
		InsertCopiedNodesBySwitchingRoot(createdNode, child, false, skipNode);

		while (child->getRightNode())
		{
			child = child->getRightNode();
			InsertCopiedNodesBySwitchingRoot(createdNode, child, false, skipNode);
		}
	}

	if (node->getRealParent())
	{
		if (!getNodeByName(node->getRealParent()->getName()))
		{
			InsertCopiedNodesBySwitchingRoot(createdNode, node->getRealParent(), false, node);
		}
	}
}

NodePtr Tree::getNodeByName(const char* name)
{
	return m_root->getNodeByName(name);
}

void Tree::updatePurpose()
{
	m_root->updatePurpose();

	m_nJoint = 0;
	m_nEffector = 0;
	resetSeqNum(m_root);
	assert(m_nJoint + m_nEffector == m_nNode);
}

void Tree::resetSeqNum(NodePtr node)
{
	SetSeqNum(node);
	if (node->getLeftNode())
		resetSeqNum(node->getLeftNode());
	if (node->getRightNode())
		resetSeqNum(node->getRightNode());
}
