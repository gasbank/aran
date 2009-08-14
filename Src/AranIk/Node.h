/*!
 * @file Node.h
 * @author http://math.ucsd.edu/~sbuss/ResearchWeb/ikmethods/index.html
 * @author Geoyeob Kim
 */
#ifndef _CLASS_NODE
#define _CLASS_NODE

#include "ArnObject.h"
#include "LinearR3.h"

enum Purpose
{
	JOINT,
	EFFECTOR
};

class VectorR3;

TYPEDEF_SHARED_PTR(Node);

class ARANIK_API Node : public ArnObject, public std::tr1::enable_shared_from_this<Node>
{
public:
										~Node();
	static NodePtr						create(const VectorR3&, const VectorR3&, double, Purpose, double minTheta=-PI, double maxTheta=PI, double restAngle=0.);
	static NodePtr						createCloneWithoutLink(NodePtr node);
	void								PrintNode() const;
	void								InitNode();
	const VectorR3&						GetAttach() const { return m_attach; }
	double								GetTheta() const { return m_theta; }
	double								AddToTheta( double delta ) { m_theta += delta; return m_theta; }
	const VectorR3&						GetS() const { return m_s; }
	const VectorR3&						GetW() const { return m_w; }
	double								GetMinTheta() const { return m_minTheta; }
	double								GetMaxTheta() const { return m_maxTheta; }
	double								GetRestAngle() const { return m_restAngle; } ;
	void								SetTheta(double newTheta) { m_theta = newTheta; }
	void								ComputeS(void);
	void								ComputeW(void);
	bool								IsEffector() const { return m_purpose==EFFECTOR; }
	bool								IsJoint() const { return m_purpose==JOINT; }
	int									GetEffectorNum() const { return m_seqNumEffector; }
	int									GetJointNum() const { return m_seqNumJoint; }
	bool								IsFrozen() const { return m_freezed; }
	void								Freeze() { m_freezed = true; }
	void								UnFreeze() { m_freezed = false; }
	const VectorR3&						getRelativePosition() const { return m_r; }
	double								getJointAngle() const { return m_theta; }
	const VectorR3&						getRotationAxis() const { return m_v; }
	double								getSize() const { return m_size; }
	NodePtr								getLeftNode() const { return m_left; }
	void								setLeftNode(NodePtr v) { m_left = v; }
	NodePtr								getRightNode() const { return m_right; }
	void								setRightNode(NodePtr v) { m_right = v; }
	virtual const char*					getName() const { return m_name.c_str(); }
	void								setName(const char* name) { m_name = name; }
	NodePtr								getRealParent() const { return m_realParent; }
	void								setRealParent(NodePtr v) { m_realParent = v; }
	Purpose								getPurpose() const { return m_purpose; }
	int									getSeqNumJoint() const { return m_seqNumJoint; }
	int									getSeqNumEffector() const { return m_seqNumEffector; }
	void								setSeqNumJoint(int v) { m_seqNumJoint = v; }
	void								setSeqNumEffector(int v) { m_seqNumEffector = v; }
	void								printNodeHierarchy(int step) const;
	bool								hasNode(const NodeConstPtr node) const;
	NodePtr								getNodeByName(const char* name);
	NodePtr								getNodeByObjectId(unsigned int id);
	void								updatePurpose();
	const VectorR3&						getAttach() const { return m_attach; }
	const VectorR3&						getR() const { return m_r; }
	void								setR(const VectorR3& v) { m_r = v; }
private:
										Node(const VectorR3&, const VectorR3&, double, Purpose, double minTheta=-PI, double maxTheta=PI, double restAngle=0.);
										Node(const Node& node);
	std::string							m_name;
	bool								m_freezed;			// Is this node frozen?
	int									m_seqNumJoint;		// sequence number if this node is a joint
	int									m_seqNumEffector;	// sequence number if this node is an effector
	double								m_size;				// size
	Purpose								m_purpose;			// joint / effector / both
	VectorR3							m_attach;			// attachment point
	VectorR3							m_r;				// relative position vector
	VectorR3							m_v;				// rotation axis
	double								m_theta;			// joint angle (radian)
	double								m_minTheta;			// lower limit of joint angle
	double								m_maxTheta;			// upper limit of joint angle
	double								m_restAngle;		// rest position angle
	VectorR3							m_s;				// Global Position
	VectorR3							m_w;				// Global rotation axis
	NodePtr								m_left;				// left child
	NodePtr								m_right;			// right sibling
	NodePtr								m_realParent;		// pointer to real parent
};

#endif
