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
	ENDEFFECTOR
};

class VectorR3;

//TYPEDEF_SHARED_PTR(Node)

class ARANIK_API Node :  public ArnObject
{
public:
	virtual								~Node();
	static Node*						create(const VectorR3&, const VectorR3&, double, Purpose, double minTheta=-PI, double maxTheta=PI, double restAngle=0.);
	static Node*						createCloneWithoutLink(Node* node);
	void								printNode() const;
	void								initNode();
	const VectorR3&						getAttach() const { return m_attach; }
	double								getTheta() const { return m_theta; }
	void								setTheta(double newTheta) { m_theta = newTheta; }
	double								addToTheta( double delta ) { m_theta += delta; return m_theta; }
	const VectorR3&						getGlobalRotAxis() const { return m_globalRotAxis; }
	double								getMinTheta() const { return m_minTheta; }
	void								setMinTheta(double v);
	double								getMaxTheta() const { return m_maxTheta; }
	void								setMaxTheta(double v);
	double								getRestAngle() const { return m_restAngle; } ;
	void								computeGlobalPosition(void);
	void								computeGlobalRotAxis(void);
	bool								isEndeffector() const { return m_purpose==ENDEFFECTOR; }
	bool								isJoint() const { return m_purpose==JOINT; }
	bool								isFrozen() const { return m_freezed; }
	int									getEffectorNum() const { return m_seqNumEffector; }
	int									getJointNum() const { return m_seqNumJoint; }
	void								freeze() { m_freezed = true; }
	void								unFreeze() { m_freezed = false; }
	const VectorR3&						getGlobalPosition() const { return m_globalPosition; }
	void								setRelativePosition(const VectorR3& v) { m_relativePosition = v; }
	const VectorR3&						getRelativePosition() const { return m_relativePosition; }
	double								getJointAngle() const { return m_theta; }
	const VectorR3&						getRotationAxis() const { return m_rotAxis; }
	void								setRotationAxis(const VectorR3& v) { m_rotAxis = v; }
	double								getSize() const { return m_size; }
	Node*								getLeftNode() const { return m_left; }
	void								setLeftNode(Node* v) { m_left = v; }
	Node*								getRightNode() const { return m_right; }
	void								setRightNode(Node* v) { m_right = v; }
	virtual const char*					getName() const { return m_name.c_str(); }
	void								setName(const char* name) { m_name = name; }
	Node*								getRealParent() const { return m_realParent; }
	void								setRealParent(Node* v) { m_realParent = v; }
	Purpose								getPurpose() const { return m_purpose; }
	int									getSeqNumJoint() const { return m_seqNumJoint; }
	int									getSeqNumEffector() const { return m_seqNumEffector; }
	void								setSeqNumJoint(int v) { m_seqNumJoint = v; }
	void								setSeqNumEffector(int v) { m_seqNumEffector = v; }
	void								printNodeHierarchy(int step) const;
	bool								hasNode(const Node* node) const;
	Node*								getNodeByName(const char* name);
	Node*								getNodeByObjectId(unsigned int id);
	void								updatePurpose();
	void								setTarget(const VectorR3& v) { assert(m_purpose==ENDEFFECTOR); m_target = v; }
	void								setTargetDiff(double dx, double dy, double dz) { assert(m_purpose==ENDEFFECTOR); m_target.x += dx, m_target.y += dy, m_target.z += dz; }
	const VectorR3&						getTarget() const { assert(m_purpose==ENDEFFECTOR); return m_target; }
	bool								isAdditionalNode() const { return m_bAdditionalNode; }
	void								setAdditionalNode(bool val) { m_bAdditionalNode = val; }
private:
										Node(const VectorR3&, const VectorR3&, double, Purpose, double minTheta=-PI, double maxTheta=PI, double restAngle=0.);
										Node(const Node& node);

	// Duplicated data structures w.r.t. ArnBone
	std::string							m_name;
	VectorR3							m_attach;				// attachment point
	VectorR3							m_relativePosition;		// relative position vector
	VectorR3							m_rotAxis;				// rotation axis
	double								m_theta;				// joint angle (radian)
	VectorR3							m_globalPosition;		// Global Position
	VectorR3							m_globalRotAxis;		// Global rotation axis

	// Should be removed
	Node*								m_left;					// left child
	Node*								m_right;				// right sibling
	Node*								m_realParent;			// pointer to real parent

	// Needed
	bool								m_freezed;				// Is this node frozen?
	int									m_seqNumJoint;			// sequence number if this node is a joint
	int									m_seqNumEffector;		// sequence number if this node is an effector
	double								m_size;					// size
	Purpose								m_purpose;				// joint / endeffector
	double								m_minTheta;				// lower limit of joint angle
	double								m_maxTheta;				// upper limit of joint angle
	double								m_restAngle;			// rest position angle
	VectorR3							m_target;				// target position only if this is an end-effector
	bool								m_bAdditionalNode;		// this node is created to make more DoF in the joint.
};

#endif
