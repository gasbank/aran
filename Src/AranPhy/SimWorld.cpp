#include "AranPhyPCH.h"
#include "GeneralBody.h"
#include "SimWorld.h"
#include "SliderJoint.h"
#include "ArnPhyBox.h"
#include "Biped.h"
#include "ArnSceneGraph.h"
#include "ArnMesh.h"
#include "BallSocketJoint.h"
#include "ArnConsts.h"

SimWorld::SimWorld()
: m_totalElapsedTime(0)
, m_osc(new OdeSpaceContext)
, m_leftSupportPosZ(0)
, m_leftSupportPosY(0)
, m_trunkSupportPosZ(0)
, m_trunkSupportPosY(0)
, m_bRenderSupports(false)
, m_footSupportHeight(0.01)
{
	m_osc->world = dWorldCreate();
	dWorldSetGravity(m_osc->world, 0, 0, -9.8);
	dWorldSetERP(m_osc->world, 0.4);
	dWorldSetCFM(m_osc->world, 0.00001);
	// Set up the collision environment
	m_osc->space = dHashSpaceCreate(0);
	m_osc->contactGroup = dJointGroupCreate(0);
	m_osc->plane = dCreatePlane(m_osc->space, 0, 0, 1, 0);
	dGeomSetData(m_osc->plane, (void*)"Ground");
}

SimWorld::~SimWorld()
{
	foreach(GeneralBody* gb, m_bodies)
	{
		delete gb;
	}
	dJointGroupDestroy(m_osc->contactGroup);
	delete m_osc;
}

SimWorld*
SimWorld::createFromEmpty()
{
	SimWorld* ret = new SimWorld();
	return ret;
}

SimWorld*
SimWorld::createFrom( ArnSceneGraph* sg )
{
	SimWorld* ret = new SimWorld();
	const unsigned int nodeCount = sg->getNodeCount();

	// Find rigid bodies of the scene and make physical instances of them.
	// Also we make fixed joints of rigid bodies which have 0-mass.
	for (unsigned int i = 0; i < nodeCount; ++i)
	{
		ArnNode* node = sg->getNodeAt(i);
		if (node->getType() == NDT_RT_MESH)
		{
			ArnMesh* mesh = reinterpret_cast<ArnMesh*>(node);
			if (mesh->isPhyActor() && mesh->getBoundingBoxType() == ABBT_BOX)
			{
				ArnVec3 size;
				mesh->getBoundingBoxDimension(&size, false);
				ArnPhyBoxPtr boxPtr;
				if (mesh->getMass())
				{
					// Normal rigid bodies which have nonzero mass.
					boxPtr.reset(ArnPhyBox::createFrom(0, mesh->getName(), mesh->getLocalXform_Trans(), size, mesh->getMass(), false));
				}
				else
				{
					// Fixed rigid bodies.
					boxPtr.reset(ArnPhyBox::createFrom(0, mesh->getName(), mesh->getLocalXform_Trans(), size, 1.0f, true));
				}
				boxPtr->setInitialQuaternion(mesh->getLocalXform_Rot());
				boxPtr->setXformableTarget(mesh);
				ret->registerBody(boxPtr);
			}
		}
	}

	// Find joints of the scene and make joint instances of them.
	for (unsigned int i = 0; i < nodeCount; ++i)
	{
		ArnNode* node = sg->getNodeAt(i);
		if (node->getType() == NDT_RT_MESH)
		{
			ArnMesh* mesh = reinterpret_cast<ArnMesh*>(node);
			if (mesh->isPhyActor() && mesh->getBoundingBoxType() == ABBT_BOX)
			{
				foreach (const ArnJointData& ajd, mesh->getJointData())
				{
					std::string jointName(mesh->getName());
					jointName += '-';
					jointName += ajd.target;
					const GeneralBodyPtr gb1 = ret->getBodyByNameFromSet(mesh->getName());
					const GeneralBodyPtr gb2 = ret->getBodyByNameFromSet(ajd.target.c_str());
					assert(gb1.get() && gb2.get());
					ArnVec3 anchor(mesh->getLocalXform_Trans());
					anchor += ajd.pivot;
					BallSocketJointPtr bsjPtr(BallSocketJoint::createFrom(0, jointName.c_str(), gb1, gb2, anchor, ArnConsts::ARNVEC3_X, ArnConsts::ARNVEC3_Y, ArnConsts::ARNVEC3_Z));
					ret->registerJoint(bsjPtr);
					foreach (const ArnJointData::ArnJointLimit& ajl, ajd.limits)
					{
						if (ajl.type.compare("AngX") == 0)
							bsjPtr->setParamLoHiStop(1, ajl.minimum, ajl.maximum);
						else if (ajl.type.compare("AngY") == 0)
							bsjPtr->setParamLoHiStop(2, ajl.minimum, ajl.maximum);
						else if (ajl.type.compare("AngZ") == 0)
							bsjPtr->setParamLoHiStop(3, ajl.minimum, ajl.maximum);
						else
							ARN_THROW_UNEXPECTED_CASE_ERROR
					}
				}
			}
		}
	}
	return ret;
}
GeneralBody*
SimWorld::placeBox(const char* name, const ArnVec3& com, const ArnVec3& size, float mass)
{
	ArnPhyBox* gb = ArnPhyBox::createFrom(m_osc, name, com, size, mass, false);
	m_bodies.push_back(gb);
	return gb;
}

void
SimWorld::render() const
{
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR

	if (m_bRenderSupports)
	{
		foreach(GeneralBody* gb, m_bodies)
		{
			gb->render();
		}
	}
}

void
SimWorld::placePiston(const char* name, const ArnVec3& com, const ArnVec3& size, float mass)
{
	GeneralBody* gb1 = placeBox(name, ArnVec3(com.x, com.y, size.z/2), size, mass);
	GeneralBody* gb2 = placeBox(name, ArnVec3(com.x, com.y, size.z + size.z/2), size, mass);
	SliderJoint* sj = new SliderJoint(m_osc);
	sj->attach(*gb2, *gb1);
	sj->setAxis(1, ArnVec3(0, 0, 1));
	sj->controlToStayStill_P(1, 10000);
	m_sliderJoints.push_back(sj);
	dJointID fix = dJointCreateFixed(m_osc->world, 0);
	dJointAttach(fix, gb1->getBodyId(), 0);
	dJointSetFixed(fix);
}


static void
NearCallback(void* data, dGeomID o1, dGeomID o2)
{
	const OdeSpaceContext* osc = reinterpret_cast<const OdeSpaceContext*>(data);
	assert(osc);
	dBodyID b1 = dGeomGetBody(o1);
	dBodyID b2 = dGeomGetBody(o2);

	// Exclude contact between bodies which are connected by a certain joint.
	if (b1 && b2 && dAreConnectedExcluding(b1, b2, dJointTypeContact))
		return;

	// N = maximum number of contact joints between o1 and o2.
	static const int MAXIMUM_CONTACT_COUNT = 50;
	dContact contact[MAXIMUM_CONTACT_COUNT];
	int n = dCollide(o1, o2, MAXIMUM_CONTACT_COUNT, &contact[0].geom, sizeof(dContact));
	for (int i = 0; i < n; i++)
	{
		//contact[i].surface.mode = dContactSoftERP; // | dContactSoftCFM;
		contact[i].surface.mode = 0;

		contact[i].surface.mu   = dInfinity; //2.0;
		//contact[i].surface.mu   = 2.0;
		//contact[i].surface.mu   = 0;

		//contact[i].surface.mu   = 500;
		//contact[i].surface.soft_erp = 0.9;
		//contact[i].surface.soft_erp = 0.0001;
		//contact[i].surface.soft_cfm = 0.01;

		dJointID c = dJointCreateContact(osc->world, osc->contactGroup, &contact[i]);
		dJointAttach(c, b1, b2);
	}

}

void
SimWorld::updateFrame(double elapsedTime)
{
	m_totalElapsedTime += elapsedTime;

	dSpaceCollide(m_osc->space, m_osc, &NearCallback);
	dWorldStep(m_osc->world, elapsedTime);
	dJointGroupEmpty(m_osc->contactGroup);
	foreach (const GeneralBodyPtr& gbPtr, m_bodiesPtr)
	{
		gbPtr->notify();
	}

	//SliderJoint* sj = 0;
	//sj = getJointByName("TrunkSupportJoint");

	//double torsoBase = m_footStep / 8 * 0.5;
	//double torsoSwing = torsoBase * 0.4;
	//sj->control_P(1, -torsoBase + torsoSwing * cos(2*M_PI*getBiped()->getFeetDist() / ( 2 * m_footStep ) ), 20, 5000);
	////sj->controlWithinRange_P(1, -0.05 + m_trunkSupportPosZ, -0.01 + m_trunkSupportPosZ, 2, 5000);

	//sj = getJointByName("TrunkSupportJoint2");

	///*
	// *   Foot                        Foot
	// *    COM                         COM
	// *     |---------|--------|--------|
	// *               |<------>|
	// *                 Trunk COM should lie here
	// *
	// */
	//const double& feetDist =  m_biped->getFeetDist();
	//dVector3 feetMidpoint;
	//m_biped->getFeetMidpoint(feetMidpoint);
	//ArnVec3 hatPos;
	//m_biped->getBody(BE_HAT)->getPosition(&hatPos);
	//const float hatYPos = hatPos.y;
	//if ( m_biped->getFeetDist() > 0.15 )
	//{
	//	double bodyOffsetY = 0;
	//	sj->control_P(1, m_trunkSupportPosY + bodyOffsetY, 10, 5000);
	//}
	//else
	//{
	//	sj->rest(1);
	//}

	//foreach(SliderJoint* sj, m_sliderJoints)
	//{
	//	sj->updateFrame();
	//}

	//m_footStep = m_nextFootStep;
	//m_footStepMaxHeight = m_nextFootStepMaxHeight;
}

void
SimWorld::reset()
{
	m_footStep = 0.8;
	m_nextFootStep = m_footStep;

	m_footStepMaxHeight = 0.4;
	m_nextFootStepMaxHeight = m_footStepMaxHeight;

	foreach(GeneralBody* gb, m_bodies)
	{
		gb->reset();
	}
	foreach(SliderJoint* sj, m_sliderJoints)
	{
		sj->reset();
	}

	m_totalElapsedTime = 0;
}

void
SimWorld::placeSupport(Biped* biped)
{
	const BipedParameters& bp = biped->getBipedParameters();

	// Support for left leg
	{
		double cx = 0;
		double cy = 0;
		ArnVec3 footPos;
		biped->getBody(BE_FOOT_L)->getPosition(&footPos);
		GeneralBody* gb3 = placeBox("LeftSupportFix2", ArnVec3(cx + footPos[0] - bp.foot_w*2, cy + footPos[1], 0.05),                    ArnVec3(bp.foot_w, bp.foot_d*6, 0.1),               1000);
		GeneralBody* gb1 = placeBox("LeftSupportFix" , ArnVec3(cx + footPos[0] - bp.foot_w*2, cy + footPos[1], 0.1 + 0.5),               ArnVec3(bp.foot_w, bp.foot_d*2, 1),                 0.00001);
		GeneralBody* gb2 = placeBox("LeftSupport"    , ArnVec3(cx + footPos[0]              , cy + footPos[1], m_footSupportHeight / 2), ArnVec3(bp.foot_w, bp.foot_d, m_footSupportHeight), 0.0001);

		gb1->setBodyData((void*)1985);
		gb2->setBodyData((void*)1986);

		SliderJoint* sj2 = new SliderJoint(m_osc);
		sj2->attach(*gb1, *gb3);
		sj2->setAxis(1, ArnVec3(0, 1, 0));
		sj2->rest(1);
		sj2->setName("LeftSupportJoint2");
		m_sliderJoints.push_back(sj2);

		SliderJoint* sj = new SliderJoint(m_osc);
		sj->attach(*gb2, *gb1);
		sj->setAxis(1, ArnVec3(0, 0, 1));
		sj2->rest(1);
		sj->setName("LeftSupportJoint");
		m_sliderJoints.push_back(sj);

		dJointID fix2 = dJointCreateFixed(m_osc->world, 0);
		dJointAttach(fix2, gb3->getBodyId(), 0);
		dJointSetFixed(fix2);
	}

	// Support for right leg
	{
		double cx = 0;
		double cy = 0;
		ArnVec3 footPos;
		biped->getBody(BE_FOOT_R)->getPosition(&footPos);
		GeneralBody* gb3 = placeBox("RightSupportFix2", ArnVec3(cx + footPos[0] + bp.foot_w*2, cy + footPos[1], 0.05), ArnVec3(bp.foot_w, bp.foot_d*6, 0.1), 1000);
		GeneralBody* gb1 = placeBox("RightSupportFix" , ArnVec3(cx + footPos[0] + bp.foot_w*2, cy + footPos[1], 0.1 + 0.5),  ArnVec3(bp.foot_w, bp.foot_d*2, 1), 0.00001);
		GeneralBody* gb2 = placeBox("RightSupport"    , ArnVec3(cx + footPos[0]              , cy + footPos[1], m_footSupportHeight / 2), ArnVec3(bp.foot_w, bp.foot_d  , m_footSupportHeight), 0.0001);

		gb1->setBodyData((void*)1987);
		gb2->setBodyData((void*)1988);

		SliderJoint* sj2 = new SliderJoint(m_osc);
		sj2->attach(*gb1, *gb3);
		sj2->setAxis(1, ArnVec3(0, 1, 0));
		sj2->rest(1);
		sj2->setName("RightSupportJoint2");
		m_sliderJoints.push_back(sj2);

		SliderJoint* sj = new SliderJoint(m_osc);
		sj->attach(*gb2, *gb1);
		sj->setAxis(1, ArnVec3(0, 0, 1));
		sj2->rest(1);
		sj->setName("RightSupportJoint");
		m_sliderJoints.push_back(sj);

		dJointID fix2 = dJointCreateFixed(m_osc->world, 0);
		dJointAttach(fix2, gb3->getBodyId(), 0);
		dJointSetFixed(fix2);
	}

	// Support for trunk
	{
		ArnVec3 com;
		biped->getBody(BE_HAT)->getPosition(&com);
		GeneralBody* gb3 = placeBox("TrunkSupportFix2", ArnVec3(com[0], com[1] - 1, 0.05), ArnVec3(0.5, 0.5, 0.1), 1000);
		GeneralBody* gb1 = placeBox("TrunkSupportFix" , ArnVec3(com[0], com[1] - 1, 1.1), ArnVec3(0.3, 0.2, 2), 0.00001);
		GeneralBody* gb2 = placeBox("TrunkSupport"    , ArnVec3(com[0], com[1] - 0.6, com[2]), ArnVec3(0.2, 0.2, 0.2), 0.00001);

		gb1->setBodyData((void*)1989);
		gb2->setBodyData((void*)1990);

		SliderJoint* sj2 = new SliderJoint(m_osc);
		sj2->attach(*gb1, *gb3);
		sj2->setAxis(1, ArnVec3(0, 1, 0));
		sj2->rest(1);
		sj2->setName("TrunkSupportJoint2");
		m_sliderJoints.push_back(sj2);

		SliderJoint* sj = new SliderJoint(m_osc);
		sj->attach(*gb2, *gb1);
		sj->setAxis(1, ArnVec3(0, 0, 1));
		sj->setParamLoHiStop(1, -0.1, 0.1);
		sj->setName("TrunkSupportJoint");
		sj->rest(1);
		m_sliderJoints.push_back(sj);


		dJointID fix2 = dJointCreateFixed(m_osc->world, 0);
		dJointAttach(fix2, gb3->getBodyId(), 0);
		dJointSetFixed(fix2);
	}
}

SliderJoint*
SimWorld::getJointByName(const char* name) const
{
	SliderJointVector::const_iterator cit = m_sliderJoints.begin();
	SliderJointVector::const_iterator citEnd = m_sliderJoints.end();
	for (; cit != citEnd; ++cit)
	{
		if (strcmp((*cit)->getName(), name) == 0)
			return *cit;
	}
	return 0;
}

GeneralJointPtr
SimWorld::getGeneralJointByName(const char* name) const
{
	foreach (const GeneralJointPtr& gbPtr, m_jointsPtr)
	{
		if (strcmp(gbPtr->getName(), name) == 0)
			return gbPtr;
	}
	return GeneralJointPtr();
}

GeneralBody*
SimWorld::getBodyByName(const char* name) const
{
	GeneralBodyVector::const_iterator cit = m_bodies.begin();
	GeneralBodyVector::const_iterator citEnd = m_bodies.end();
	for (; cit != citEnd; ++cit)
	{
		if (strcmp((const char*)((*cit)->getGeomData()), name) == 0)
			return *cit;
	}
	return 0;
}

const GeneralBodyPtr
SimWorld::getBodyByNameFromSet( const char* name ) const
{
	foreach (const GeneralBodyPtr& b, m_bodiesPtr)
	{
		if (strcmp(b->getName(), name) == 0)
			return b;
	}
	return GeneralBodyPtr();
}
/*
GeneralBody* SimWorld::getBodyByGeomID(dGeomID g) const
{
	GeneralBodyVector::const_iterator cit = m_bodies.begin();
	GeneralBodyVector::const_iterator citEnd = m_bodies.end();
	for (; cit != citEnd; ++cit)
	{
		if ((*cit)->getGeomID() == g)
			return *cit;
	}
	return 0;
}
*/
void
SimWorld::clearBodyContact()
{
	GeneralBodyVector::const_iterator cit = m_bodies.begin();
	GeneralBodyVector::const_iterator citEnd = m_bodies.end();
	for (; cit != citEnd; ++cit)
	{
		(*cit)->setContactGround(false);
	}
}

bool
SimWorld::registerBody( const GeneralBodyPtr& gbPtr )
{
	gbPtr->configureOdeContext(m_osc);
	m_bodiesPtr.insert(gbPtr);
	return true;
}

bool
SimWorld::registerJoint( const GeneralJointPtr& gjPtr )
{
	gjPtr->configureOdeContext(m_osc);
	m_jointsPtr.insert(gjPtr);
	return true;
}
