#include "AranIkPCH.h"
#include "LinearR3.h"
#include "Tree.h"
#include "Node.h"

void ReleaseNode(Node* node)
{
	if (node)
	{
		ReleaseNode(node->getLeftNode());
		ReleaseNode(node->getRightNode());
		delete node;
	}
}

Tree::Tree()
: m_nNode(0)
, m_nEffector(0)
, m_nJoint(0)
, m_root(0)
{
}

Tree::~Tree()
{
	ReleaseNode(m_root);
}

void Tree::SetSeqNum(Node* node)
{
	switch (node->getPurpose()) {
	case JOINT:
		node->setSeqNumJoint(m_nJoint);
		m_nJoint++;
		node->setSeqNumEffector(-1);
		break;
	case ENDEFFECTOR:
		node->setSeqNumJoint(-1);
		node->setSeqNumEffector(m_nEffector);
		m_nEffector++;
		break;
	}
}

void Tree::InsertRoot(Node* root)
{
	assert( m_nNode==0 );
	m_nNode++;
	assert(!m_root);
	Tree::m_root = root;
	root->setRelativePosition(root->getAttach());
	assert( !(root->getLeftNode() || root->getRightNode()) );
	SetSeqNum(root);
}

void Tree::InsertLeftChild(Node* parent, Node* child)
{
	assert(parent);
	m_nNode++;
	parent->setLeftNode(child);
	child->setRealParent(parent);
	child->setRelativePosition(child->getAttach() - child->getRealParent()->getAttach());
	assert( !(child->getLeftNode() || child->getRightNode()) );
	SetSeqNum(child);
}

void Tree::InsertRightSibling(Node* parent, Node* child)
{
	assert(parent);
	m_nNode++;
	parent->setRightNode(child);
	child->setRealParent(parent->getRealParent());
	child->setRelativePosition(child->getAttach() - child->getRealParent()->getAttach());
	assert( !(child->getLeftNode() || child->getRightNode()) );
	SetSeqNum(child);
}

// Search recursively below "node" for the node with index value.
Node* Tree::SearchJoint(Node* node, int index)
{
	Node* ret;
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
			return 0;
		}
	}
	else {
		return 0;
	}
}


// Get the joint with the index value
Node* Tree::GetJoint(int index)
{
	return SearchJoint(m_root, index);
}

// Search recursively below node for the end effector with the index value
Node* Tree::SearchEffector(Node* node, int index)
{
	Node* ret;
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
			return 0;
		}
	} else {
		return 0;
	}
}


// Get the end effector for the index value
Node* Tree::GetEffector(int index)
{
	return SearchEffector(m_root, index);
}

// Returns the global position of the effector.
const VectorR3& Tree::GetEffectorPosition(int index)
{
	Node* effector = GetEffector(index);
	assert(effector);
	return (effector->getGlobalPosition());
}

void Tree::ComputeTree(Node* node)
{
	if (node != 0) {
		node->computeGlobalPosition();
		node->computeGlobalRotAxis();
		ComputeTree(node->getLeftNode());
		ComputeTree(node->getRightNode());
	}
}

void Tree::Compute(void)
{
	ComputeTree(m_root);
}

void Tree::PrintTree(const Node* node) const
{
	if (node)
	{
		node->printNode();
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
void Tree::InitTree(Node* node)
{
	if (node)
	{
		node->initNode();
		InitTree(node->getLeftNode());
		InitTree(node->getRightNode());
	}
}

// Initialize all nodes in the tree
void Tree::Init(void)
{
	InitTree(m_root);
}

void Tree::UnFreezeTree(Node* node)
{
	if (node != 0) {
		node->unFreeze();
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

bool Tree::hasNode(const Node* node) const
{
	if (m_root == node)
		return true;
	else
		return m_root->hasNode(node);
}

Node* Tree::getPrevSiblingNode(Node* node)
{
	Node* realParent = node->getRealParent();
	if (!realParent)
		return 0;
	Node* prevSibling = realParent->getLeftNode();
	while (prevSibling && prevSibling->getRightNode() != node)
	{
		prevSibling = prevSibling->getRightNode();
	}
	return prevSibling;
}

void Tree::InsertCopiedNodesBySwitchingRoot(Node* prevInsertedNode, Node* node, bool childOrSibling /* false = child, true = sibling */, Node* skipNode)
{
	assert(node);

	if (node == skipNode)
		return;

	Node* createdNode = Node::createCloneWithoutLink(node);

	if (!prevInsertedNode && !m_root)
	{
		InsertRoot(createdNode);
	}
	else if (prevInsertedNode && !childOrSibling)
	{
		// Insert child to 'prevInsertedNode'.
		Node* leftNode = prevInsertedNode->getLeftNode();
		if (!leftNode)
		{
			InsertLeftChild(prevInsertedNode, createdNode);
		}
		else
		{
			Node* lastChild = leftNode;
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

	Node* child = node->getLeftNode();
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
        const char *realParentName = node->getRealParent()->getName();
        Node *alreadyExists = getNodeByName(realParentName);
        if (!alreadyExists)
		{
            // Insert only if it is not exist on newly created tree.
			InsertCopiedNodesBySwitchingRoot(createdNode, node->getRealParent(), false, node);
		}
	}
}

Node* Tree::getNodeByName(const char* name)
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

void Tree::resetSeqNum(Node* node)
{
	SetSeqNum(node);
	if (node->getLeftNode())
		resetSeqNum(node->getLeftNode());
	if (node->getRightNode())
		resetSeqNum(node->getRightNode());
}
