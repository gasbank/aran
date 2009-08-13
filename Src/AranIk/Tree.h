
#include "LinearR3.h"
#include "Node.h"

#ifndef _CLASS_TREE
#define _CLASS_TREE

class ARANIK_API Tree {

public:
	Tree();

	int GetNumNode() const { return nNode; }
	int GetNumEffector() const { return nEffector; }
	int GetNumJoint() const { return nJoint; }
	void InsertRoot(Node*);
	void InsertLeftChild(Node* parent, Node* child);
	void InsertRightSibling(Node* parent, Node* child);
	void InsertCopiedNodesBySwitchingRoot(Node* prevInsertedNode, Node* node, bool childOrSibling, Node* skipNode);

	// Accessors based on node numbers
	Node* GetJoint(int);
	Node* GetEffector(int);
	const VectorR3& GetEffectorPosition(int);

	// Accessors for tree traversal
	Node* GetRoot() const { return root; }
	Node* GetSuccessor ( const Node* ) const;
	Node* GetParent( const Node* node ) const { return node->getRealParent(); }

	void Compute();
	void Print();
	void Init();
	void UnFreeze();

	//void Draw();
	void printHierarchy() const; // Name only, tree style
	void PrintTree(const Node*) const; // Detailed information, list style
	bool hasNode(const Node* node) const;
	Node* getPrevSiblingNode(Node* node);
	Node* getNodeByName(const char* name);
	void updatePurpose();
private:
	Node* root;
	int nNode;			// nNode = nEffector + nJoint
	int nEffector;
	int nJoint;
	void SetSeqNum(Node*);
	void resetSeqNum(Node* node);
	Node* SearchJoint(Node*, int);
	Node* SearchEffector(Node*, int);
	void ComputeTree(Node*);

	//void DrawTree(Node*);

	void InitTree(Node*);
	void UnFreezeTree(Node*);
};

typedef std::tr1::shared_ptr<Tree> TreePtr;


inline Node* Tree::GetSuccessor ( const Node* node ) const
{
	if ( node->getLeftNode() ) {
		return node->getLeftNode();
	}
	while ( true ) {
		if ( node->getRightNode() ) {
			return ( node->getRightNode() );
		}
		node = node->getRealParent();
		if ( !node ) {
			return 0;		// Back to root, finished traversal
		}
	}
}

#endif
