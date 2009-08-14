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
	NodePtr rootNode = Node::create(rootHeadPoint, VectorR3::UnitZ, 1.0, JOINT, ArnToRadian(-180.0), ArnToRadian(180.0), 0);
	rootNode->setName("Root");
	ret->m_tree->InsertRoot(rootNode);

	bool firstChild = true;
	NodePtr prevNode = rootNode;
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
	std::cout << " --- ArnIkSolver creation report: ArnSkeleton " << skel->getName() << " has "
			<< ret->getTree()->GetNumEffector() << " end-effector(s) and "
			<< ret->getTree()->GetNumJoint() << " joint(s)." << std::endl;
	ret->printHierarchy();
	ret->m_skel = skel;
	return ret;
}

static NodePtr
CreateNodeFromArnBone(const ArnSkeleton* skel, const ArnBone* bone, bool bEndEffector)
{
	VectorR3 tailPoint; // Global coordinates of the bone's head.
	ArnVec3 arnHeadPoint;
	ArnVec3 arnTailPoint;
	ArnGetGlobalBonePosition(&arnHeadPoint, &arnTailPoint, skel, bone);
	ArnVec3Assign(tailPoint, arnTailPoint);
	VectorR3 rotAxis;
	double size = 1.0;
	Purpose purpose = JOINT;
	double minAngle = ArnToRadian(-180.0);
	double maxAngle = ArnToRadian( 180.0);
	double restAngle = ArnToRadian(0.0);
	if (bEndEffector)
	{
		assert(bone->getChildBoneCount() == 0);
		purpose = EFFECTOR;
		rotAxis = VectorR3::Zero;
	}
	else
	{
		purpose = JOINT;
		const char* boneName = bone->getName();
		if (strcmp(boneName, "KneeR") == 0 || strcmp(boneName, "KneeL") == 0)
		{
			minAngle = -ARN_PI;
			maxAngle = 0;
		}
		else if (strcmp(boneName, "AnkleR") == 0 || strcmp(boneName, "AnkleL") == 0)
		{
			minAngle = ArnToRadian(-30.0);
			maxAngle = ArnToRadian(30.0);
		}
		rotAxis = VectorR3::UnitX;
	}
	NodePtr ret = Node::create(tailPoint, rotAxis, size, purpose, minAngle, maxAngle, restAngle);
	ret->setName(bone->getName());
	return ret;
}

NodePtr
ArnIkSolver::addToTree(NodePtr prevNode, const ArnSkeleton* skel, const ArnBone* bone, bool firstChild)
{
	assert(prevNode);
	assert(bone);
	bool bEndEffector = false;
	if (bone->getChildBoneCount() == 0)
	{
		bEndEffector = true;
	}
	NodePtr node = CreateNodeFromArnBone(skel, bone, bEndEffector);
	if (prevNode && firstChild)
	{
		// b is a child of prevNode.
		m_tree->InsertLeftChild(prevNode, node);
	}
	else if (prevNode && !firstChild)
	{
		// b is a sibling of prevNode.
		m_tree->InsertRightSibling(prevNode, node);
	}
	else
	{
		ARN_THROW_UNEXPECTED_CASE_ERROR
	}

	NodePtr nextPrevNode = node;
	bool nextFirstChild = true;
	foreach (const ArnNode* arnnode, bone->getChildren())
	{
		assert(arnnode->getType() == NDT_RT_BONE);
		const ArnBone* bone2 = reinterpret_cast<const ArnBone*>(arnnode);
		nextPrevNode = addToTree(nextPrevNode, skel, bone2, nextFirstChild);
		nextFirstChild = false;
	}
	return node;
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
	m_jacobian.reset(new Jacobian(m_tree.get()));
}

void
ArnIkSolver::setTarget(unsigned int i, const ArnVec3& v)
{
	VectorR3 vec(v.x, v.y, v.z);
	m_jacobian->setTarget(i, vec);
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
ArnIkSolver::hasNode(const NodeConstPtr node)
{
	return m_tree->hasNode(node);
}

void
ArnIkSolver::reconfigureRoot(NodePtr newRoot)
{
	assert(newRoot);
	assert(m_tree->hasNode(newRoot));

	TreePtr newTree(new Tree);
	newTree->InsertCopiedNodesBySwitchingRoot(NodePtr(), newRoot, true, NodePtr());
	newTree->updatePurpose();
	m_tree = newTree;

	initializeJacobian();
	reset();
	std::cout << " --- ArnIkSolver root reconfig report: "
			<< getTree()->GetNumEffector() << " end-effector(s) and "
			<< getTree()->GetNumJoint() << " joint(s)." << std::endl;
	printHierarchy();
}

NodePtr
ArnIkSolver::getNodeByName(const char* name)
{
	return m_tree->getNodeByName(name);
}

//////////////////////////////////////////////////////////////////////////

unsigned int
ArnCreateArnIkSolversOnSceneGraph( ArnSceneGraphPtr sg )
{
	unsigned int count = 0;
	foreach(ArnNode* node, sg->getChildren())
	{
		if (node->getType() == NDT_RT_SKELETON)
		{
			ArnSkeleton* skel = static_cast<ArnSkeleton*>(node);
			ArnIkSolver* ikSolver = 0;
			CreateArnIkSolver(&ikSolver, skel);
			skel->setVisible(false);
			assert(ikSolver);
			++count;
		}
	}
	return count;
}