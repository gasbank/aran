#ifndef ARANIKSOLVER_H
#define ARANIKSOLVER_H

class ArnSkeleton;
class ArnBone;
class Node;
TYPEDEF_SHARED_PTR(Tree);
TYPEDEF_SHARED_PTR(Jacobian);
TYPEDEF_SHARED_PTR(Node);

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
	void							setTarget(const char* nodeName, const ArnVec3& v);
	void							update();
	void							printHierarchy() const;
	void							reconfigureRoot(NodePtr newRoot);
	bool							hasNode(const NodeConstPtr node);
	NodePtr							getNodeByName(const char* name);
	NodePtr							getNodeByObjectId(unsigned int id);
protected:
private:
									ArnIkSolver();
	NodePtr							addToTree(NodePtr prevNode, const ArnSkeleton* skel, const ArnBone* bone, bool firstChild);
	void							initializeJacobian();
	JacobianPtr						getJacobian() { return m_jacobian; }

	TreePtr							m_tree;
	JacobianPtr						m_jacobian;
	const ArnSkeleton*				m_skel;
};

#endif // ARANIKSOLVER_H
