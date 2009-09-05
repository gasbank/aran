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
	void						InsertRoot(NodePtr);
	void						InsertLeftChild(NodePtr parent, NodePtr child);
	void						InsertRightSibling(NodePtr parent, NodePtr child);
	void						InsertCopiedNodesBySwitchingRoot(NodePtr prevInsertedNode, NodePtr node, bool childOrSibling, NodePtr skipNode);
	// Accessors based on node numbers
	NodePtr						GetJoint(int);
	NodePtr						GetEffector(int);
	const VectorR3&				GetEffectorPosition(int);
	// Accessors for tree traversal
	NodePtr						GetRoot() const { return m_root; }
	NodePtr						GetSuccessor ( NodePtr ) const;
	tr1ns::weak_ptr<Node>	GetParent( NodeConstPtr node ) const { return node->getRealParent(); }
	void						Compute();
	void						Print();
	void						Init();
	void						UnFreeze();
	void						printHierarchy() const; // Name only, tree style
	void						PrintTree(NodeConstPtr) const; // Detailed information, list style
	bool						hasNode(const NodeConstPtr node) const;
	NodePtr						getPrevSiblingNode(NodePtr node);
	NodePtr						getNodeByName(const char* name);
	void						updatePurpose();
private:
	void						SetSeqNum(NodePtr);
	void						resetSeqNum(NodePtr node);
	NodePtr						SearchJoint(NodePtr, int);
	NodePtr						SearchEffector(NodePtr, int);
	void						ComputeTree(NodePtr);
	void						InitTree(NodePtr);
	void						UnFreezeTree(NodePtr);
	NodePtr						m_root;
	int							m_nNode;			// nNode = nEffector + nJoint
	int							m_nEffector;
	int							m_nJoint;
};

inline NodePtr Tree::GetSuccessor ( NodePtr node ) const
{
	if ( node->getLeftNode() )
	{
		return node->getLeftNode();
	}
	tr1ns::weak_ptr<Node> nodeWeak = node;
	while ( true )
	{
		if ( nodeWeak.lock()->getRightNode() )
		{
			return ( nodeWeak.lock()->getRightNode() );
		}
		nodeWeak = nodeWeak.lock()->getRealParent();
		if ( !nodeWeak.lock() )
		{
			return NodePtr();		// Back to root, finished traversal
		}
	}
}

#endif
