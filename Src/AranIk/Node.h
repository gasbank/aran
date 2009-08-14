/*!
 * @file Node.h
 * @author http://math.ucsd.edu/~sbuss/ResearchWeb/ikmethods/index.html
 * @author Geoyeob Kim
 */
#ifndef _CLASS_NODE
#define _CLASS_NODE

#include "LinearR3.h"

enum Purpose
{
	JOINT,
	EFFECTOR
};

class VectorR3;

TYPEDEF_SHARED_PTR(Node);

class ARANIK_API Node : public std::tr1::enable_shared_from_this<Node>
{
public:
										~Node();
	static NodePtr						create(const VectorR3&, const VectorR3&, double, Purpose, double minTheta=-PI, double maxTheta=PI, double restAngle=0.);
	static NodePtr						createCloneWithoutLink(NodePtr node);
	void								PrintNode() const;
	void								InitNode();
	const VectorR3&						GetAttach() const { return attach; }
	double								GetTheta() const { return theta; }
	double								AddToTheta( double delta ) { theta += delta; return theta; }
	const VectorR3&						GetS() const { return s; }
	const VectorR3&						GetW() const { return w; }
	double								GetMinTheta() const { return minTheta; }
	double								GetMaxTheta() const { return maxTheta; }
	double								GetRestAngle() const { return restAngle; } ;
	void								SetTheta(double newTheta) { theta = newTheta; }
	void								ComputeS(void);
	void								ComputeW(void);
	bool								IsEffector() const { return purpose==EFFECTOR; }
	bool								IsJoint() const { return purpose==JOINT; }
	int									GetEffectorNum() const { return seqNumEffector; }
	int									GetJointNum() const { return seqNumJoint; }
	bool								IsFrozen() const { return freezed; }
	void								Freeze() { freezed = true; }
	void								UnFreeze() { freezed = false; }
	const VectorR3&						getRelativePosition() const { return r; }
	double								getJointAngle() const { return theta; }
	const VectorR3&						getRotationAxis() const { return v; }
	double								getSize() const { return size; }
	NodePtr								getLeftNode() const { return left; }
	void								setLeftNode(NodePtr v) { left = v; }
	NodePtr								getRightNode() const { return right; }
	void								setRightNode(NodePtr v) { right = v; }
	const char*							getName() const { return m_name.c_str(); }
	void								setName(const char* name) { m_name = name; }
	NodePtr								getRealParent() const { return realparent; }
	void								setRealParent(NodePtr v) { realparent = v; }
	Purpose								getPurpose() const { return purpose; }
	int									getSeqNumJoint() const { return seqNumJoint; }
	int									getSeqNumEffector() const { return seqNumEffector; }
	void								setSeqNumJoint(int v) { seqNumJoint = v; }
	void								setSeqNumEffector(int v) { seqNumEffector = v; }
	void								printNodeHierarchy(int step) const;
	bool								hasNode(const NodeConstPtr node) const;
	NodePtr								getNodeByName(const char* name);
	void								updatePurpose();
	const VectorR3&						getAttach() const { return attach; }
	const VectorR3&						getR() const { return r; }
	void								setR(const VectorR3& v) { r = v; }
private:
										Node(const VectorR3&, const VectorR3&, double, Purpose, double minTheta=-PI, double maxTheta=PI, double restAngle=0.);
										Node(const Node& node);
	std::string							m_name;
	bool								freezed;			// Is this node frozen?
	int									seqNumJoint;		// sequence number if this node is a joint
	int									seqNumEffector;		// sequence number if this node is an effector
	double								size;				// size
	Purpose								purpose;			// joint / effector / both
	VectorR3							attach;				// attachment point
	VectorR3							r;					// relative position vector
	VectorR3							v;					// rotation axis
	double								theta;				// joint angle (radian)
	double								minTheta;			// lower limit of joint angle
	double								maxTheta;			// upper limit of joint angle
	double								restAngle;			// rest position angle
	VectorR3							s;					// GLobal Position
	VectorR3							w;					// Global rotation axis
	NodePtr								left;				// left child
	NodePtr								right;				// right sibling
	NodePtr								realparent;			// pointer to real parent
};

#endif
