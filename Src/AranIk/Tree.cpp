#include "AranIkPCH.h"
#include "LinearR3.h"
#include "Tree.h"
#include "Node.h"

Tree::Tree()
{
	root = 0;
	nNode = nEffector = nJoint = 0;
}

void Tree::SetSeqNum(Node* node)
{
	switch (node->getPurpose()) {
	case JOINT:
		node->setSeqNumJoint(nJoint);
		nJoint++;
		node->setSeqNumEffector(-1);
		break;
	case EFFECTOR:
		node->setSeqNumJoint(-1);
		node->setSeqNumEffector(nEffector);
		nEffector++;
		break;
	}
}

void Tree::InsertRoot(Node* root)
{
	assert( nNode==0 );
	nNode++;
	Tree::root = root;
	root->r = root->attach;
	assert( !(root->left || root->right) );
	SetSeqNum(root);
}

void Tree::InsertLeftChild(Node* parent, Node* child)
{
	assert(parent);
	nNode++;
	parent->left = child;
	child->realparent = parent;
	child->r = child->attach - child->realparent->attach;
	assert( !(child->left || child->right) );
	SetSeqNum(child);
}

void Tree::InsertRightSibling(Node* parent, Node* child)
{
	assert(parent);
	nNode++;
	parent->right = child;
	child->realparent = parent->realparent;
	child->r = child->attach - child->realparent->attach;
	assert( !(child->left || child->right) );
	SetSeqNum(child);
}

// Search recursively below "node" for the node with index value.
Node* Tree::SearchJoint(Node* node, int index)
{
	Node* ret;
	if (node != 0) {
		if (node->seqNumJoint == index) {
			return node;
		} else {
			ret = SearchJoint(node->left, index);
			if (ret) {
				return ret;
			}
			ret = SearchJoint(node->right, index);
			if (ret) {
				return ret;
			}
			return NULL;
		}
	}
	else {
		return NULL;
	}
}


// Get the joint with the index value
Node* Tree::GetJoint(int index)
{
	return SearchJoint(root, index);
}

// Search recursively below node for the end effector with the index value
Node* Tree::SearchEffector(Node* node, int index)
{
	Node* ret;
	if (node != 0) {
		if (node->seqNumEffector == index) {
			return node;
		} else {
			ret = SearchEffector(node->left, index);
			if (ret) {
				return ret;
			}
			ret = SearchEffector(node->right, index);
			if (ret) {
				return ret;
			}
			return NULL;
		}
	} else {
		return NULL;
	}
}


// Get the end effector for the index value
Node* Tree::GetEffector(int index)
{
	return SearchEffector(root, index);
}

// Returns the global position of the effector.
const VectorR3& Tree::GetEffectorPosition(int index)
{
	Node* effector = GetEffector(index);
	assert(effector);
	return (effector->s);
}

void Tree::ComputeTree(Node* node)
{
	if (node != 0) {
		node->ComputeS();
		node->ComputeW();
		ComputeTree(node->left);
		ComputeTree(node->right);
	}
}

void Tree::Compute(void)
{
	ComputeTree(root);
}

void Tree::PrintTree(const Node* node) const
{
	if (node != 0) {
		node->PrintNode();
		PrintTree(node->left);
		PrintTree(node->right);
	}
}

void Tree::Print(void)
{
	PrintTree(root);
	cout << "\n";
}

// Recursively initialize tree below the node
void Tree::InitTree(Node* node)
{
	if (node != 0) {
		node->InitNode();
		InitTree(node->left);
		InitTree(node->right);
	}
}

// Initialize all nodes in the tree
void Tree::Init(void)
{
	InitTree(root);
}

void Tree::UnFreezeTree(Node* node)
{
	if (node != 0) {
		node->UnFreeze();
		UnFreezeTree(node->left);
		UnFreezeTree(node->right);
	}
}

void Tree::UnFreeze(void)
{
	UnFreezeTree(root);
}

void Tree::printHierarchy() const
{
	root->printNodeHierarchy(0);
}

bool Tree::hasNode(const Node* node) const
{
	if (root == node)
		return true;
	else
		return root->hasNode(node);
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

	Node* createdNode = new Node(*node);

	if (!prevInsertedNode && !root)
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

	if (node->realparent)
	{
		if (!getNodeByName(node->realparent->getName()))
		{
			InsertCopiedNodesBySwitchingRoot(createdNode, node->realparent, false, node);
		}
	}
}

Node* Tree::getNodeByName(const char* name)
{
	return root->getNodeByName(name);
}

void Tree::updatePurpose()
{
	root->updatePurpose();

	nJoint = 0;
	nEffector = 0;
	resetSeqNum(root);
	assert(nJoint + nEffector == nNode);
}

void Tree::resetSeqNum(Node* node)
{
	SetSeqNum(node);
	if (node->getLeftNode())
		resetSeqNum(node->getLeftNode());
	if (node->getRightNode())
		resetSeqNum(node->getRightNode());
}
