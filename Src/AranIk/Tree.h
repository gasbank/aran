/*!
 * @file Tree.h
 * @author http://math.ucsd.edu/~sbuss/ResearchWeb/ikmethods/index.html
 * @author Geoyeob Kim
 */
#ifndef _CLASS_TREE
#define _CLASS_TREE

#include "LinearR3.h"
#include "Node.h"

TYPEDEF_SHARED_PTR(Node)
TYPEDEF_SHARED_PTR(Tree)

class ARANIK_API Tree
{
public:
								Tree();
								~Tree();
	int							GetNumNode() const { return m_nNode; }
	int							GetNumEffector() const { return m_nEffector; }
	int							GetNumJoint() const { return m_nJoint; }
	void						InsertRoot(Node*);
	void						InsertLeftChild(Node* parent, Node* child);
	void						InsertRightSibling(Node* parent, Node* child);
	void						InsertCopiedNodesBySwitchingRoot(Node* prevInsertedNode, Node* node, bool childOrSibling, Node* skipNode);
	// Accessors based on node numbers
	Node*						GetJoint(int);
	Node*						GetEffector(int);
	const VectorR3&				GetEffectorPosition(int);
	// Accessors for tree traversal
	Node*						GetRoot() const { return m_root; }
	Node*						GetSuccessor ( Node* ) const;
	Node*						GetParent( const Node* node ) const { return node->getRealParent(); }
	void						Compute();
	void						Print();
	void						Init();
	void						UnFreeze();
	void						printHierarchy() const; // Name only, tree style
	void						PrintTree(const Node*) const; // Detailed information, list style
	bool						hasNode(const Node* node) const;
	Node*						getPrevSiblingNode(Node* node);
	Node*						getNodeByName(const char* name);
	void						updatePurpose();
private:
	void						SetSeqNum(Node*);
	void						resetSeqNum(Node* node);
	Node*						SearchJoint(Node*, int);
	Node*						SearchEffector(Node*, int);
	void						ComputeTree(Node*);
	void						InitTree(Node*);
	void						UnFreezeTree(Node*);
	Node*						m_root;
	int							m_nNode;			// nNode = nEffector + nJoint
	int							m_nEffector;
	int							m_nJoint;
};

inline Node* Tree::GetSuccessor ( Node* node ) const
{
	if (node->getLeftNode())
	{
		return node->getLeftNode();
	}
	while (node)
	{
		if (node->getRightNode())
			return node->getRightNode();
		
		node = node->getRealParent();
	}
	return 0;
}

#endif
