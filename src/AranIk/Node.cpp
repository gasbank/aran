#include "AranIkPCH.h"
#include "LinearR3.h"
#include "MathMisc.h"
#include "Node.h"

Node::Node(const VectorR3& attach, const VectorR3& v, double size, Purpose purpose, double minTheta, double maxTheta, double restAngle)
: ArnObject(NDT_RT_IKNODE)
, m_left(0)
, m_right(0)
, m_realParent(0)
{
	m_freezed				= false;
	m_size					= size;
	m_purpose				= purpose;
	m_seqNumJoint			= -1;
	m_seqNumEffector		= -1;
	m_attach				= attach;		// Global attachment point when joints are at zero angle
	m_target				= attach;
	m_relativePosition.Set(0.0, 0.0, 0.0);		// r will be updated when this node is inserted into tree
	m_rotAxis				= v;				// Rotation axis when joints at zero angles
	m_theta					= 0.0;
	m_minTheta				= minTheta;
	m_maxTheta				= maxTheta;
	m_restAngle				= restAngle;
	m_bAdditionalNode		= false;
}

Node::Node(const Node& node)
: ArnObject(NDT_RT_IKNODE)
, m_left(0)
, m_right(0)
, m_realParent(0)
{
	m_freezed				= node.m_freezed;
	m_size					= node.m_size;
	m_purpose				= node.m_purpose;
	m_seqNumJoint			= -1;
	m_seqNumEffector		= -1;
	m_attach				= node.getGlobalPosition();
	m_target				= node.m_target;
	m_relativePosition		= node.m_relativePosition;
	m_rotAxis				= node.m_rotAxis;
	m_theta					= node.m_theta;
	m_minTheta				= node.m_minTheta;
	m_maxTheta				= node.m_maxTheta;
	m_restAngle				= node.m_restAngle;
	m_name					= node.m_name;
	m_bAdditionalNode		= node.m_bAdditionalNode;
}

Node::~Node()
{
}

// Compute the global position of a single node
void Node::computeGlobalPosition(void)
{
	Node* y = getRealParent();
	Node* w = this;
	m_globalPosition = m_relativePosition;							// Initialize to local (relative) position
	while ( y ) {
		m_globalPosition.Rotate( y->m_theta, y->m_rotAxis );
		y = y->getRealParent();
		w = w->getRealParent();
		m_globalPosition += w->getRelativePosition();
	}
}

// Compute the global rotation axis of a single node
void Node::computeGlobalRotAxis(void)
{
	Node* y = this->m_realParent;
	m_globalRotAxis = m_rotAxis;							// Initialize to local rotation axis
	while (y) {
		m_globalRotAxis.Rotate(y->m_theta, y->m_rotAxis);
		y = y->m_realParent;
	}
}

void Node::printNode() const
{
	cerr << "Attach : (" << m_attach << ")\n";
	cerr << "r : (" << m_relativePosition << ")\n";
	cerr << "s : (" << m_globalPosition << ")\n";
	cerr << "w : (" << m_globalRotAxis << ")\n";
	cerr << "realparent : " << getRealParent()->getSeqNumJoint() << "\n";
}

void Node::initNode()
{
	m_theta = 0.0;
}

void Node::printNodeHierarchy(int step) const
{
	for (int i = 0; i < step; ++i)
		std::cout << "  ";
	//std::cout << getName() << " [" << getAttach().x << " " << getAttach().y << " " << getAttach().z << "]" << std::endl;
	//std::cout << getName() << " " << m_minTheta << " " << m_maxTheta;
	std::cout << getName();
	if (isEndeffector())
		std::cout << " endeffector " << getEffectorNum();
	else if (isJoint())
		std::cout << " joint " << getJointNum();
	std::cout << std::endl;

	if (m_left)
		m_left->printNodeHierarchy(step + 1);

	if (m_right)
		m_right->printNodeHierarchy(step);
}

bool Node::hasNode(const Node* node) const
{
	if (this == node)
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

Node* Node::getNodeByName(const char* name)
{
	if (strcmp(m_name.c_str(), name) == 0)
		return this;

    Node* ret = 0;
	if (m_right)
		ret = m_right->getNodeByName(name);
	if (ret)
		return ret;

    if (m_left)
        ret = m_left->getNodeByName(name);
    if (ret)
        return ret;

    return 0;
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
		m_purpose = ENDEFFECTOR;
	}
	if (m_right)
		m_right->updatePurpose();
}

Node*
Node::create(const VectorR3& attach, const VectorR3& v, double size, Purpose purpose, double minTheta, double maxTheta, double restAngle)
{
	Node* ret(new Node(attach, v, size, purpose, minTheta, maxTheta, restAngle));
	return ret;
}

Node*
Node::createCloneWithoutLink( Node* node )
{
	Node* ret(new Node(*node));
	return ret;
}

Node*
Node::getNodeByObjectId( unsigned int id )
{
	if (getObjectId() == id)
		return this;
	Node* ret;
	if (m_left)
	{
		ret = m_left->getNodeByObjectId(id);
		if (ret)
			return ret;
	}
	if (m_right)
	{
		ret = m_right->getNodeByObjectId(id);
		if (ret)
			return ret;
	}
    return 0;
}

void Node::setMinTheta( double v )
{
	m_minTheta = v;
}

void Node::setMaxTheta( double v )
{
	m_maxTheta = v;
}

Node *Node::getChildNodeAt (unsigned int idx) const
{
    if (!m_left)
        return 0;
    else
    {
        Node *ret = m_left;
        while (idx && ret)
        {
            ret = ret->getRightNode ();
            --idx;
        }
        return ret;
    }
}

int Node::getIndexOfChildNode(const Node *c) const
{
    int ret = 0;
    Node *child = m_left;
    while (child && (child != c))
    {
        child = child->getRightNode ();
        ++ret;
    }

    if (child)
        return ret;
    else
        return -1;
}

unsigned int Node::getChildNodeCount () const
{
    int count = 0;
    Node *child = m_left;
    while (child)
    {
        child = child->getRightNode ();
        ++count;
    }
    return count;
}