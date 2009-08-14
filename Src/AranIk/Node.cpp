#include "AranIkPCH.h"
#include "LinearR3.h"
#include "MathMisc.h"
#include "Node.h"

Node::Node(const VectorR3& attach, const VectorR3& v, double size, Purpose purpose, double minTheta, double maxTheta, double restAngle)
{
	Node::m_freezed = false;
	Node::m_size = size;
	Node::m_purpose = purpose;
	m_seqNumJoint = -1;
	m_seqNumEffector = -1;
	Node::m_attach = attach;		// Global attachment point when joints are at zero angle
	m_r.Set(0.0, 0.0, 0.0);		// r will be updated when this node is inserted into tree
	Node::m_v = v;				// Rotation axis when joints at zero angles
	m_theta = 0.0;
	Node::m_minTheta = minTheta;
	Node::m_maxTheta = maxTheta;
	Node::m_restAngle = restAngle;
}

Node::Node(const Node& node)
{
	Node::m_freezed = false;
	Node::m_size = node.m_size;
	Node::m_purpose = node.m_purpose;
	m_seqNumJoint = -1;
	m_seqNumEffector = -1;
	Node::m_attach = node.m_attach;		// Global attachment point when joints are at zero angle
	m_r.Set(0.0, 0.0, 0.0);		// r will be updated when this node is inserted into tree
	Node::m_v = node.m_v;				// Rotation axis when joints at zero angles
	m_theta = 0.0;
	Node::m_minTheta = node.m_minTheta;
	Node::m_maxTheta = node.m_maxTheta;
	Node::m_restAngle = node.m_restAngle;
	m_name = node.m_name;
}

Node::~Node()
{
}

// Compute the global position of a single node
void Node::ComputeS(void)
{
	NodePtr y = this->m_realParent;
	Node* w = this;
	m_s = m_r;							// Initialize to local (relative) position
	while ( y ) {
		m_s.Rotate( y->m_theta, y->m_v );
		y = y->getRealParent();
		w = w->getRealParent().get();
		m_s += w->getR();
	}
}

// Compute the global rotation axis of a single node
void Node::ComputeW(void)
{
	NodePtr y = this->m_realParent;
	m_w = m_v;							// Initialize to local rotation axis
	while (y) {
		m_w.Rotate(y->m_theta, y->m_v);
		y = y->m_realParent;
	}
}

void Node::PrintNode() const
{
	cerr << "Attach : (" << m_attach << ")\n";
	cerr << "r : (" << m_r << ")\n";
	cerr << "s : (" << m_s << ")\n";
	cerr << "w : (" << m_w << ")\n";
	cerr << "realparent : " << m_realParent->m_seqNumJoint << "\n";
}

void Node::InitNode()
{
	m_theta = 0.0;
}

void Node::printNodeHierarchy(int step) const
{
	for (int i = 0; i < step; ++i)
		std::cout << "  ";
	std::cout << getName() << " [" << getAttach().x << " " << getAttach().y << " " << getAttach().z << "]" << std::endl;

	if (m_left)
		m_left->printNodeHierarchy(step + 1);

	if (m_right)
		m_right->printNodeHierarchy(step);
}

bool Node::hasNode(const NodeConstPtr node) const
{
	if (this == node.get())
		return true;
	if (m_left)
	{
		bool ret = m_left->hasNode(node);
		if (ret)
			return true;
	}
	if (m_right)
	{
		bool ret = m_right->hasNode(node);
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
	if (m_right)
		ret = m_right->getNodeByName(name);
	if (ret)
		return ret;
	else if (m_left)
		return m_left->getNodeByName(name);
	else
	{
		assert(!ret);
		return NodePtr();
	}
}

void Node::updatePurpose()
{
	if (m_left)
	{
		m_purpose = JOINT;
		m_left->updatePurpose();
	}
	else
	{
		m_purpose = EFFECTOR;
	}
	if (m_right)
		m_right->updatePurpose();
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
