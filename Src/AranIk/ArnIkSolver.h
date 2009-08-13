#ifndef ARANIKSOLVER_H
#define ARANIKSOLVER_H

class ArnSkeleton;
class ArnBone;
class Node;
TYPEDEF_SHARED_PTR(Tree);
TYPEDEF_SHARED_PTR(Jacobian);

/*!
 * @brief IK 솔버
 */
class ARANIK_API ArnIkSolver
{
public:
									~ArnIkSolver();
	/*!
	 * @brief ArnSkeleton에서 ArnIkSolver를 생성
	 */
	static ArnIkSolver*				createFrom(const ArnSkeleton* skel);
	void							reset();
	const TreePtr					getTree() const { return m_tree; }
	void							setTarget(unsigned int i, const ArnVec3& v);
	void							update();
	void							printHierarchy() const;
	void							reconfigureRoot(Node* newRoot);
	bool							hasNode(const Node* node);
	Node*							getNodeByName(const char* name);
protected:
private:
									ArnIkSolver();
	Node*							addToTree(Node* prevNode, const ArnSkeleton* skel, const ArnBone* bone, bool firstChild);
	void							initializeJacobian();
	JacobianPtr						getJacobian() { return m_jacobian; }
	TreePtr							m_tree;
	JacobianPtr						m_jacobian;
};

#endif // ARANIKSOLVER_H
