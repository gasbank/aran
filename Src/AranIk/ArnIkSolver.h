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
	void							setTarget(unsigned int i, const ArnVec3& v);
	void							update();
	void							printHierarchy() const;
	void							reconfigureRoot(NodePtr newRoot);
	bool							hasNode(const NodeConstPtr node);
	NodePtr							getNodeByName(const char* name);
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

class ArnSceneGraph;

/*!
 * @brief ArnSceneGraph에 있는 모든 ArnSkeleton에 대해 ArnIkSolver를 생성
 * @return 생성된 ArnIkSolver 개수
 * @remark ArnSkeleton을 재귀적으로 모두 찾는 것이 아니라 최상위 노드(root)에서만 검색합니다.
 */
ARANIK_API unsigned int ArnCreateArnIkSolversOnSceneGraph(ArnSceneGraphPtr sg);

#endif // ARANIKSOLVER_H
