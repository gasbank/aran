#include "AranIkPCH.h"
#include "LinearR3.h"
#include "MathMisc.h"
#include "Node.h"

//////////////////extern int RotAxesOn;
//int RotAxesOn;

Node::Node(const VectorR3& attach, const VectorR3& v, double size, Purpose purpose, double minTheta, double maxTheta, double restAngle)
{
	Node::freezed = false;
	Node::size = size;
	Node::purpose = purpose;
	seqNumJoint = -1;
	seqNumEffector = -1;
	Node::attach = attach;		// Global attachment point when joints are at zero angle
	r.Set(0.0, 0.0, 0.0);		// r will be updated when this node is inserted into tree
	Node::v = v;				// Rotation axis when joints at zero angles
	theta = 0.0;
	Node::minTheta = minTheta;
	Node::maxTheta = maxTheta;
	Node::restAngle = restAngle;
	left = right = realparent = 0;
}

Node::Node(const Node& node)
{
	Node::freezed = false;
	Node::size = node.size;
	Node::purpose = node.purpose;
	seqNumJoint = -1;
	seqNumEffector = -1;
	Node::attach = node.attach;		// Global attachment point when joints are at zero angle
	r.Set(0.0, 0.0, 0.0);		// r will be updated when this node is inserted into tree
	Node::v = node.v;				// Rotation axis when joints at zero angles
	theta = 0.0;
	Node::minTheta = node.minTheta;
	Node::maxTheta = node.maxTheta;
	Node::restAngle = node.restAngle;
	left = right = realparent = 0;
	m_name = node.m_name;
}

// Compute the global position of a single node
void Node::ComputeS(void)
{
	Node* y = this->realparent;
	Node* w = this;
	s = r;							// Initialize to local (relative) position
	while ( y ) {
		s.Rotate( y->theta, y->v );
		y = y->realparent;
		w = w->realparent;
		s += w->r;
	}
}

// Compute the global rotation axis of a single node
void Node::ComputeW(void)
{
	Node* y = this->realparent;
	w = v;							// Initialize to local rotation axis
	while (y) {
		w.Rotate(y->theta, y->v);
		y = y->realparent;
	}
}

void Node::PrintNode() const
{
	cerr << "Attach : (" << attach << ")\n";
	cerr << "r : (" << r << ")\n";
	cerr << "s : (" << s << ")\n";
	cerr << "w : (" << w << ")\n";
	cerr << "realparent : " << realparent->seqNumJoint << "\n";
}

void Node::InitNode()
{
	theta = 0.0;
}

void Node::printNodeHierarchy(int step) const
{
	for (int i = 0; i < step; ++i)
		std::cout << "  ";
	std::cout << getName() << std::endl;

	if (left)
		left->printNodeHierarchy(step + 1);

	if (right)
		right->printNodeHierarchy(step);
}

bool Node::hasNode(const Node* node) const
{
	if (this == node)
		return true;
	if (left)
	{
		bool ret = left->hasNode(node);
		if (ret)
			return true;
	}
	if (right)
	{
		bool ret = right->hasNode(node);
		if (ret)
			return true;
	}
	return false;
}

Node* Node::getNodeByName(const char* name)
{
	if (strcmp(m_name.c_str(), name) == 0)
		return this;

	Node* ret = 0;
	if (right)
		ret = right->getNodeByName(name);
	if (ret)
		return ret;
	else if (left)
		return left->getNodeByName(name);
	else
		return 0;
}

void Node::updatePurpose()
{
	if (left)
	{
		purpose = JOINT;
		left->updatePurpose();
	}
	else
	{
		purpose = EFFECTOR;
	}
	if (right)
		right->updatePurpose();
}
