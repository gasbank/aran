#include "AranIkPCH.h"
#include "LinearR3.h"
#include "MathMisc.h"
#include "Node.h"

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
	m_name = node.m_name;
}

Node::~Node()
{
}

// Compute the global position of a single node
void Node::ComputeS(void)
{
	NodePtr y = this->realparent;
	Node* w = this;
	s = r;							// Initialize to local (relative) position
	while ( y ) {
		s.Rotate( y->theta, y->v );
		y = y->getRealParent();
		w = w->getRealParent().get();
		s += w->getR();
	}
}

// Compute the global rotation axis of a single node
void Node::ComputeW(void)
{
	NodePtr y = this->realparent;
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

bool Node::hasNode(const NodeConstPtr node) const
{
	if (this == node.get())
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

NodePtr Node::getNodeByName(const char* name)
{
	if (strcmp(m_name.c_str(), name) == 0)
		return shared_from_this();

	NodePtr ret;
	if (right)
		ret = right->getNodeByName(name);
	if (ret)
		return ret;
	else if (left)
		return left->getNodeByName(name);
	else
	{
		assert(!ret);
		return NodePtr();
	}
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

NodePtr
Node::create(const VectorR3& attach, const VectorR3& v, double size, Purpose purpose, double minTheta, double maxTheta, double restAngle)
{
	return NodePtr(new Node(attach, v, size, purpose, minTheta, maxTheta, restAngle));
}

NodePtr
Node::createCloneWithoutLink( NodePtr node )
{
	return NodePtr(new Node(*node));
}
