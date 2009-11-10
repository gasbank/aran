#include "AranIkPCH.h"
#include "ArnIkSolver.h"
#include "ArnSkeleton.h"
#include "ArnBone.h"
#include "ArnContainer.h"
#include "Tree.h"
#include "Jacobian.h"
#include "AranIk.h"

ArnIkSolver::ArnIkSolver()
: m_skel(0)
{
}

ArnIkSolver::~ArnIkSolver()
{
	// Do not release m_skel.
}

ArnIkSolver*
ArnIkSolver::createFrom(const ArnSkeleton* skel)
{
	ArnIkSolver* ret = new ArnIkSolver();
	ret->m_tree.reset(new Tree);
	VectorR3 rootHeadPoint;
	ArnVec3Assign(rootHeadPoint, skel->getLocalXform_Trans());
	Node* rootNode = Node::create(rootHeadPoint, VectorR3::UnitZ, 1.0, JOINT, ArnToRadian(-180.0), ArnToRadian(180.0), 0);
	rootNode->setName("Root");
	ret->m_tree->InsertRoot(rootNode);

	bool firstChild = true;
	Node* prevNode = rootNode;
	foreach (const ArnNode* node, skel->getChildren())
	{
		// This loop should be called once.
		assert(node->getType() == NDT_RT_BONE);
		const ArnBone* bone = reinterpret_cast<const ArnBone*>(node);
		prevNode = ret->addToTree(prevNode, skel, bone, firstChild);
		firstChild = false;
	}

	ret->initializeJacobian();
	ret->reset();
	//std::cout.setf(ios::left);
	std::cout << " ----- ArnIkSolver creation report -----" << std::endl;
	std::cout << "   ArnSkeleton        : " << skel->getName() << std::endl;
	std::cout << "   # of End-effectors : " << std::setw(10) << ret->getTree()->GetNumEffector() << std::endl;
	std::cout << "   # of Joints        : " << std::setw(10) << ret->getTree()->GetNumJoint() << std::endl;
	std::cout << " ---------------------------------------" << std::endl;
	ret->printHierarchy();
	ret->m_skel = skel;
	return ret;
}

static Node*
CreateNodeFromArnBone(const ArnSkeleton* skel, const ArnBone* bone, bool bEndEffector)
{
	VectorR3 tailPoint; // Global coordinates of the bone's head.
	ArnVec3 arnHeadPoint;
	ArnVec3 arnTailPoint;
	bone->computeWorldHeadTail(arnHeadPoint, arnTailPoint);
	ArnVec3Assign(tailPoint, arnTailPoint);
	VectorR3 rotAxis;
	double size = 1.0;
	Purpose purpose = JOINT;
	double minAngle = ArnToRadian(-180.0);
	double maxAngle = ArnToRadian( 180.0);
	double restAngle = ArnToRadian(0.0);
	if (bEndEffector)
	{
		assert(bone->getChildBoneCount(true) == 0);
		purpose = ENDEFFECTOR;
		rotAxis = VectorR3::Zero;
	}
	else
	{
		purpose = JOINT;
	}
	Node* ret = Node::create(tailPoint, rotAxis, size, purpose, minAngle, maxAngle, restAngle);
	ret->setName(bone->getName());
	return ret;
}

static void
GetJointRotAxisOfBoneFromName(bool& x, bool& y, bool& z, const ArnBone* bone)
{
	unsigned int offset = 0;
	const size_t nameLen = strlen(bone->getName());
	x = false;
	y = false;
	z = false;
	while (bone->getName()[offset] != '_' && offset < nameLen )
	{
		if (bone->getName()[offset] == 'X')
			x = true;
		else if (bone->getName()[offset] == 'Y')
			y = true;
		else if (bone->getName()[offset] == 'Z')
			z = true;
		else if (bone->getName()[offset] == 'E')
		{ // End effector
			x = false;
			y = false;
			z = false;
		}
		else
		{
			ARN_THROW_UNEXPECTED_CASE_ERROR
		}
		++offset;
	}
}

static void
SetJointLimitFromBone(Node* node, const ArnBone* bone)
{
	const char* nodeName = node->getName();
	AxisEnum axis;
	float minimum, maximum;
	if (strncmp(nodeName, "X_", 2) == 0)
	{
		axis = AXIS_X;
		bone->getRotLimit(axis, minimum, maximum);
	}
	else if (strncmp(nodeName, "Y_", 2) == 0)
	{
		axis = AXIS_Y;
		bone->getRotLimit(axis, minimum, maximum);
	}
	else if (strncmp(nodeName, "Z_", 2) == 0)
	{
		axis = AXIS_Z;
		bone->getRotLimit(axis, minimum, maximum);
	}
	else if (strncmp(nodeName, "Root", 4) == 0)
	{
		minimum = float(-ARN_PI);
		maximum = float( ARN_PI);
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}

	if (minimum <= -ARN_PI)
		minimum = float(-ARN_PI);
	node->setMinTheta(minimum);

	if (maximum >= ARN_PI)
		maximum = float(ARN_PI);
	node->setMaxTheta(maximum);
}

Node*
ArnIkSolver::addToTree(Node* prevNode, const ArnSkeleton* skel, const ArnBone* bone, bool firstChild)
{
	assert(prevNode);
	assert(bone);
	bool bEndEffector = false;
	if (bone->getChildBoneCount(true) == 0)
	{
		bEndEffector = true;
	}
	bool rx, ry, rz;
	GetJointRotAxisOfBoneFromName(rx, ry, rz, bone);
	Node* firstCreated = 0;

	if (!(rx || ry || rz))
	{
		// End-effector
		assert(bEndEffector);
		Node* eNode = CreateNodeFromArnBone(skel, bone, bEndEffector);
		eNode->setRotationAxis(VectorR3::Zero);

		if (prevNode && firstChild)
		{
			// b is a child of prevNode.
			m_tree->InsertLeftChild(prevNode, eNode);
		}
		else if (prevNode && !firstChild)
		{
			// b is a sibling of prevNode.
			m_tree->InsertRightSibling(prevNode, eNode);
		}
		else
		{
			ARN_THROW_UNEXPECTED_CASE_ERROR
		}

		prevNode = eNode;
		if (!firstCreated)
			firstCreated = prevNode;
	}
	if (rx)
	{
		Node* rxNode = CreateNodeFromArnBone(skel, bone, bEndEffector);
		rxNode->setRotationAxis(VectorR3::UnitX);
		const size_t boneNameLen = strlen(bone->getName());
		size_t pos = 0;
		while (bone->getName()[pos] != '_' && pos < boneNameLen)
		{
			++pos;
		}
		std::string bn = "X_";
		bn += &bone->getName()[pos+1];
		rxNode->setName(bn.c_str());

		if (bone->getChildBoneCount(false) == 1)
		{
			SetJointLimitFromBone(rxNode, bone->getFirstChildBone());
		}

		if (prevNode && firstChild)
		{
			// b is a child of prevNode.
			m_tree->InsertLeftChild(prevNode, rxNode);
		}
		else if (prevNode && !firstChild)
		{
			// b is a sibling of prevNode.
			m_tree->InsertRightSibling(prevNode, rxNode);
			firstChild = true;
		}
		else
		{
			ARN_THROW_UNEXPECTED_CASE_ERROR
		}

		prevNode = rxNode;
		if (!firstCreated)
		{
			firstCreated = prevNode;
		}
		else
		{
			rxNode->setAdditionalNode(true);
		}
	}
	if (ry)
	{
		Node* ryNode = CreateNodeFromArnBone(skel, bone, bEndEffector);
		ryNode->setRotationAxis(VectorR3::UnitY);
		const size_t boneNameLen = strlen(bone->getName());
		size_t pos = 0;
		while (bone->getName()[pos] != '_' && pos < boneNameLen)
		{
			++pos;
		}
		std::string bn = "Y_";
		bn += &bone->getName()[pos+1];
		ryNode->setName(bn.c_str());

		if (bone->getChildBoneCount(false) == 1)
		{
			SetJointLimitFromBone(ryNode, bone->getFirstChildBone());
		}

		if (prevNode && firstChild)
		{
			// b is a child of prevNode.
			m_tree->InsertLeftChild(prevNode, ryNode);
		}
		else if (prevNode && !firstChild)
		{
			// b is a sibling of prevNode.
			m_tree->InsertRightSibling(prevNode, ryNode);
			firstChild = true;
		}
		else
		{
			ARN_THROW_UNEXPECTED_CASE_ERROR
		}

		prevNode = ryNode;
		if (!firstCreated)
		{
			firstCreated = prevNode;
		}
		else
		{
			ryNode->setAdditionalNode(true);
		}
	}
	if (rz)
	{
		Node* rzNode = CreateNodeFromArnBone(skel, bone, bEndEffector);
		rzNode->setRotationAxis(VectorR3::UnitZ);
		const size_t boneNameLen = strlen(bone->getName());
		size_t pos = 0;
		while (bone->getName()[pos] != '_' && pos < boneNameLen)
		{
			++pos;
		}
		std::string bn = "Z_";
		bn += &bone->getName()[pos+1];
		rzNode->setName(bn.c_str());

		if (bone->getChildBoneCount(false) == 1)
		{
			SetJointLimitFromBone(rzNode, bone->getFirstChildBone());
		}

		if (prevNode && firstChild)
		{
			// b is a child of prevNode.
			m_tree->InsertLeftChild(prevNode, rzNode);
		}
		else if (prevNode && !firstChild)
		{
			// b is a sibling of prevNode.
			m_tree->InsertRightSibling(prevNode, rzNode);
			firstChild = true;
		}
		else
		{
			ARN_THROW_UNEXPECTED_CASE_ERROR
		}

		prevNode = rzNode;
		if (!firstCreated)
		{
			firstCreated = prevNode;
		}
		else
		{
			rzNode->setAdditionalNode(true);
		}
	}
	assert(prevNode);
	Node* nextPrevNode = prevNode;
	bool nextFirstChild = true;
	foreach (const ArnNode* arnnode, bone->getChildren())
	{
		assert(arnnode->getType() == NDT_RT_BONE);
		const ArnBone* bone2 = reinterpret_cast<const ArnBone*>(arnnode);
		nextPrevNode = addToTree(nextPrevNode, skel, bone2, nextFirstChild);
		nextFirstChild = false;
	}
	return firstCreated;
}

void
ArnIkSolver::reset()
{
	assert(m_tree && m_jacobian);
	m_tree->Init();
	m_tree->Compute();
	m_jacobian->Reset();
}

void
ArnIkSolver::initializeJacobian()
{
	m_jacobian.reset(new Jacobian(m_tree));
}

void
ArnIkSolver::setTarget(const char* nodeName, const ArnVec3& v)
{
	VectorR3 vec(v.x, v.y, v.z);
	m_tree->getNodeByName(nodeName)->setTarget(vec);
}

void
ArnIkSolver::update()
{
	if ( /*UseJacobianTargets*/ true )
	{
		m_jacobian->SetJtargetActive();
	}
	else
	{
		m_jacobian->SetJendActive();
	}
	m_jacobian->ComputeJacobian();				// Set up Jacobian and deltaS vectors

	//m_jacobian->CalcDeltaThetasTranspose();		// Jacobian transpose method
	//m_jacobian->CalcDeltaThetasDLS(); 				// Damped least squares method
	//m_jacobian->CalcDeltaThetasPseudoinverse();		// Pure pseudoinverse method
	m_jacobian->CalcDeltaThetasSDLS();			// Selectively damped least squares method

	m_jacobian->UpdateThetas();					// Apply the change in the theta values
	m_jacobian->UpdatedSClampValue();
}

void
ArnIkSolver::printHierarchy() const
{
	m_tree->printHierarchy();
}

bool
ArnIkSolver::hasNode(const Node* node)
{
	return m_tree->hasNode(node);
}

void
ArnIkSolver::reconfigureRoot(Node* newRoot)
{
	assert(newRoot);
	assert(m_tree->hasNode(newRoot));

	TreePtr newTree(new Tree);
	newTree->InsertCopiedNodesBySwitchingRoot(0, newRoot, true, 0);
	newTree->updatePurpose();
	m_tree = newTree;

	initializeJacobian();
	reset();
	std::cout << " ----- ArnIkSolver creation report -----" << std::endl;
	std::cout << "   # of End-effectors : " << std::setw(10) << getTree()->GetNumEffector() << std::endl;
	std::cout << "   # of Joints        : " << std::setw(10) << getTree()->GetNumJoint() << std::endl;
	std::cout << " ---------------------------------------" << std::endl;
	printHierarchy();
	m_selectedEndeffector = 0;
}

Node*
ArnIkSolver::getNodeByName(const char* name)
{
	return m_tree->getNodeByName(name);
}

Node*
ArnIkSolver::getNodeByObjectId( unsigned int id )
{
	return m_tree->GetRoot()->getNodeByObjectId(id);
}
