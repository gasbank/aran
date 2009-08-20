#ifndef ARANIKSOLVER_H
#define ARANIKSOLVER_H

class ArnSkeleton;
class ArnBone;
class Node;
TYPEDEF_SHARED_PTR(Tree)
TYPEDEF_SHARED_PTR(Jacobian)
TYPEDEF_SHARED_PTR(Node)

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
	void							setSelectedEndeffector(NodePtr node) { m_selectedEndeffector = node; }
	NodePtr							getSelectedEndeffector() const { return m_selectedEndeffector; }
protected:
private:
									ArnIkSolver();
	/*!
	 * @brief \a skel 에 속한 \a bone 을 IK 솔버 트리에 추가
	 *
	 * IK 솔버 트리에 추가할 때에 \a prevNode 를 제공합니다.
	 * IK 솔버 트리의 각 노드는 첫 번째 자식과 그 다음 시블링에 대한 포인터를
	 * 가지고 있습니다. 그러므로 트리에 추가를 하고 싶을 때는 그 전 노드와
	 * 그 노드가 새로 넣을 노드의 부모가 될지 시블링이 될지를 명시해 줘야 합니다.
	 * 그 전 노드는 \a prevNode 로 정해주고, \a prevNode 가 새로 넣을 노드의 부모가 된다면
	 * \a firstChild 를 \c true로 설정해주고, \a prevNode 가 새로 넣을 노드의 시블링이라면
	 * \a firstChild 를 \c false로 설정합니다. 본 함수는 재귀적으로 호출됩니다.
	 *
	 * 특히 \a bone 의 이름 prefix에 따라 IK 솔버 트리에 추가되는 노드의 개수가 달라집니다.
	 * 예를 들어 이름이 \c XYZ_HipR 인 ArnBone 이 있다고 하면 \c X_HipR , \c Y_HipR , \c Z_HipR
	 * 이란 이름의 세 개 노드가 추가되게 됩니다. 다시 말해, 각 회전축에 따라 노드가 분리되어
	 * 추가됩니다. 이는 원래 ArnSkeleton 을 이루던 ArnBone 개수보다 많은 수의 노드가 추가
	 * 될 수 있음을 의미합니다.
	 */
	NodePtr							addToTree(NodePtr prevNode, const ArnSkeleton* skel, const ArnBone* bone, bool firstChild);
	void							initializeJacobian();
	JacobianPtr						getJacobian() { return m_jacobian; }

	TreePtr							m_tree;
	JacobianPtr						m_jacobian;
	const ArnSkeleton*				m_skel;
	NodePtr							m_selectedEndeffector;
};

#endif // ARANIKSOLVER_H
