#include "AranPhyPCH.h"
#include "Biped.h"
#include "GeneralBody.h"
#include "HingeJoint.h"
#include "UniversalJoint.h"
#include "UtilFunc.h"
//#include "EulerAngles.h"
#include "GeneralJoint.h"
#include "ArnPhyBox.h"

std::vector<dContact> g_contactsWithGround;

Biped::Biped(const OdeSpaceContext* osc)
: m_bInitialized(false)
, m_osc(osc)
, m_bSimulate(false)
, m_bWidenFeet(false)
, m_bBendTrunk(false)
, m_bNarrowFeet(false)
, m_bRStepForward(false)
, m_bLStepForward(false)
, m_bRStepForward_Secondary(false)
, m_bStandOnSingleFoot(false)
, m_bCrouch(false)
, m_bStayAllJoints(false)
, m_curControlState(CSE_BIPED_DO_NOTHING)
, m_nextControlState(CSE_BIPED_DO_NOTHING)
{
	//ctor
	m_bodies.resize(BE_COUNT);
	m_joints.resize(JE_COUNT);

	memset(&m_bp, 0, sizeof(BipedParameters));

	clearBodyContact();
}

Biped::~Biped()
{
	//dtor
	size_t s = m_bodies.size();
	for (size_t i = 0; i < s; ++i)
	{
		delete m_bodies[i];
	}

	s = m_joints.size();
	for (size_t i = 0; i < s; ++i)
	{
		delete m_joints[i];
	}
}

void Biped::initialize(const BipedParameters& bp, ArnVec4 bo)
{
	m_bp = bp;

	m_bp.leg_h = m_bp.groin_h + m_bp.calf_h;

	static const double bodyCenters[BE_COUNT][3] =
	{
		{ bo[0] + 0,               bo[1] +   0,                                 bo[2] + m_bp.foot_h + m_bp.leg_h  + m_bp.torso_h/2,  }, // HAT
		{ bo[0] + m_bp.torso_w/4,  bo[1] +   0,                                 bo[2] + m_bp.foot_h + m_bp.calf_h + m_bp.groin_h/2,  }, // GROIN_R
		{ bo[0] - m_bp.torso_w/4,  bo[1] +   0,                                 bo[2] + m_bp.foot_h + m_bp.calf_h + m_bp.groin_h/2,  }, // GROIN_L
		{ bo[0] + m_bp.torso_w/4,  bo[1] +   0,                                 bo[2] + m_bp.foot_h + m_bp.calf_h/2,                 }, // CALF_R
		{ bo[0] - m_bp.torso_w/4,  bo[1] +   0,                                 bo[2] + m_bp.foot_h + m_bp.calf_h/2,                 }, // CALF_L
		{ bo[0] + m_bp.torso_w/4,  bo[1] +   m_bp.foot_d/2 - m_bp.calf_r,       bo[2] + m_bp.foot_h/2,                               }, // FOOT_R
		{ bo[0] - m_bp.torso_w/4,  bo[1] +   m_bp.foot_d/2 - m_bp.calf_r,       bo[2] + m_bp.foot_h/2,                               } // FOOT_L
	};

	memcpy(m_bp.bodyCenters, bodyCenters, sizeof(bodyCenters));

	static const double bodyRadiusAndHeight[BE_COUNT][2] =
	{
		{ -1, -1 },
		{ m_bp.groin_r, m_bp.groin_h - m_bp.groin_r*2 },
		{ m_bp.groin_r, m_bp.groin_h - m_bp.groin_r*2 },
		{ m_bp.calf_r, m_bp.calf_h - m_bp.calf_r*2 },
		{ m_bp.calf_r, m_bp.calf_h - m_bp.calf_r*2 },
		{ -1, -1 },
		{ -1, -1 }
	};
	memcpy(m_bp.bodyRadiusAndHeight, bodyRadiusAndHeight, sizeof(bodyRadiusAndHeight));

	//enum BodyEnum { BE_HAT, BE_GROIN_R, BE_GROIN_L, BE_CALF_R, BE_CALF_L, BE_FOOT_R, BE_FOOT_L, BE_COUNT };
	static const double bodyXyAndHeightForBoxes[BE_COUNT][3] =
	{
		{ m_bp.torso_w, m_bp.torso_d, m_bp.torso_h },
		{ m_bp.groin_r, m_bp.groin_r, m_bp.groin_h },
		{ m_bp.groin_r, m_bp.groin_r, m_bp.groin_h },
		{ m_bp.calf_r, m_bp.calf_r,   m_bp.calf_h },
		{ m_bp.calf_r, m_bp.calf_r,   m_bp.calf_h },
		{ m_bp.foot_w, m_bp.foot_d,   m_bp.foot_h },
		{ m_bp.foot_w, m_bp.foot_d,   m_bp.foot_h },
	};
	memcpy(m_bp.bodyXyAndHeightForBoxes, bodyXyAndHeightForBoxes, sizeof(bodyXyAndHeightForBoxes));


	static const double bodyWeights[BE_COUNT] = { m_bp.torso_m, m_bp.groin_m, m_bp.groin_m, m_bp.calf_m, m_bp.calf_m, m_bp.foot_m, m_bp.foot_m };
	memcpy(m_bp.bodyWeights, bodyWeights, sizeof(bodyWeights));

	static const double jointCenters[JE_COUNT][3] =
	{
		{ bo[0] + m_bp.torso_w/4,  bo[1] +0, bo[2] + m_bp.foot_h + m_bp.leg_h,   },
		{ bo[0] - m_bp.torso_w/4,  bo[1] +0, bo[2] + m_bp.foot_h + m_bp.leg_h,   },
		{ bo[0] + m_bp.torso_w/4,  bo[1] +0, bo[2] + m_bp.foot_h + m_bp.calf_h,  },
		{ bo[0] - m_bp.torso_w/4,  bo[1] +0, bo[2] + m_bp.foot_h + m_bp.calf_h,  },
		{ bo[0] + m_bp.torso_w/4,  bo[1] +0, bo[2] + m_bp.foot_h,                },
		{ bo[0] - m_bp.torso_w/4,  bo[1] +0, bo[2] + m_bp.foot_h,                }
	};
	memcpy(m_bp.jointCenters, jointCenters, sizeof(jointCenters));

	static const unsigned jointLinks[JE_COUNT][2] =
	{
		{ BE_HAT, BE_GROIN_R },
		{ BE_HAT, BE_GROIN_L },
		{ BE_GROIN_R, BE_CALF_R },
		{ BE_GROIN_L, BE_CALF_L },
		{ BE_CALF_R, BE_FOOT_R },
		{ BE_CALF_L, BE_FOOT_L }
	};
	memcpy(m_bp.jointLinks, jointLinks, sizeof(jointLinks));

	// Configure bodies

	// Crate a HAT(torso-arm-trunk)
	ArnVec3 hatCom(m_bp.bodyCenters[BE_HAT][0], m_bp.bodyCenters[BE_HAT][1], m_bp.bodyCenters[BE_HAT][2]);
	ArnVec3 hatSize(m_bp.torso_w, m_bp.torso_d, m_bp.torso_h);
	m_bodies[BE_HAT] = ArnPhyBox::createFrom(m_osc, "HAT", hatCom, hatSize, m_bp.bodyWeights[BE_HAT], false);

	// Create groins, calves (HAT and feet are excluded)
	for (int i = 1; i < BE_COUNT - 2; i++)
	{
		ArnVec3 com(m_bp.bodyCenters[i][0], m_bp.bodyCenters[i][1], m_bp.bodyCenters[i][2]);
		ArnVec3 size(m_bp.bodyRadiusAndHeight[i][0], m_bp.bodyRadiusAndHeight[i][1], 0);
		m_bodies[i] = ArnPhyBox::createFrom(m_osc, BodyEnumStrings[i], com, size, m_bp.bodyWeights[i], false);
	}

	// Create feet
	for (int i = BE_COUNT - 2; i < BE_COUNT; i++)
	{
		ArnVec3 com(m_bp.bodyCenters[i][0], m_bp.bodyCenters[i][1], m_bp.bodyCenters[i][2]);
		ArnVec3 size(m_bp.bodyRadiusAndHeight[i][0], m_bp.bodyRadiusAndHeight[i][1], 0);
		m_bodies[i] = ArnPhyBox::createFrom(m_osc, BodyEnumStrings[i], com, size, m_bp.bodyWeights[i], false);
	}

	// Configure joints
	for (int i = 0; i < JE_COUNT; i++)
	{
		switch (i)
		{
			case JE_R1:
			case JE_L1:
			case JE_R3:
			case JE_L3:
			{
				UniversalJoint* joint = new UniversalJoint(m_osc);
				joint->attach(*m_bodies[ m_bp.jointLinks[i][0] ], *m_bodies[ m_bp.jointLinks[i][1] ]);
				joint->setAnchor(ArnVec3(m_bp.jointCenters[i][0], m_bp.jointCenters[i][1], m_bp.jointCenters[i][2]));
				joint->setAxis(1, ArnVec3(1, 0, 0));
				joint->setAxis(2, ArnVec3(0, 1, 0));
				joint->enableJointFeedback();

				if (i == JE_R1)
				{
					joint->setParamLoHiStop(1, -M_PI/180 * 150, M_PI/180 * 150);
					joint->setParamLoHiStop(2, -M_PI/2, M_PI/2);
				}
				else if (i == JE_L1)
				{
					joint->setParamLoHiStop(1, -M_PI/180 * 150, M_PI/180 * 150);
					joint->setParamLoHiStop(2, -M_PI/2, M_PI/2);
				}
				else if (i == JE_R3)
				{
					joint->setParamLoHiStop(1, -M_PI/4, M_PI/180 * 70);
					joint->setParamLoHiStop(2, -M_PI/4, M_PI/8);
				}
				else if (i == JE_L3)
				{
					joint->setParamLoHiStop(1, -M_PI/4, M_PI/180 * 70);
					joint->setParamLoHiStop(2, -M_PI/4, M_PI/8);
				}

				m_joints[i] = joint;
				break;
			}
			case JE_R2:
			case JE_L2:
			{
				HingeJoint* joint = new HingeJoint(m_osc);
				joint->attach(*m_bodies[ m_bp.jointLinks[i][0] ], *m_bodies[ m_bp.jointLinks[i][1] ]);
				joint->setAnchor(ArnVec3(m_bp.jointCenters[i][0], m_bp.jointCenters[i][1], m_bp.jointCenters[i][2]));
				joint->setAxis(1, ArnVec3(1, 0, 0));
				joint->setParamLoHiStop(1, 0, M_PI/180 * 170);
				//joint->setParamLoHiStop(1, 0, M_PI);
				m_joints[i] = joint;
				break;
			}
			default:
				abort();
		}
		m_joints[i]->setName(JointEnumStrings[i]);
	}
	assert(!m_bInitialized);
	m_bInitialized = true;
}

void Biped::setFsm(const FiniteStateMachine& fsm)
{
}

void Biped::render()
{
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR

	/*
	unsigned i;

	// Render bodies
	for (i = 0; i < BE_COUNT; ++i)
	{
		m_bodies[i]->render(basicObjects);
		//m_bodies[i]->renderComAxis();
	}

	// Render joints
	for (i = 0; i < JE_COUNT; ++i)
	{
		m_joints[i]->render(basicObjects);
	}


	// Render joints axis
	for (i = 0; i < JE_COUNT; ++i)
	{
		//m_joints[i]->renderJointAxis();
	}
	*/
}

void Biped::reset()
{
	if (!m_bInitialized)
		abort();

	int i;
	dQuaternion q = {1.0, 0.0, 0.0, 0.0};

	for(i = 0; i < BE_COUNT; ++i) {
		dBodyID body = m_bodies[i]->getBodyId();
		dBodySetAngularVel(body, 0.0, 0.0, 0.0);
		dBodySetLinearVel(body, 0.0, 0.0, 0.0);
		dBodySetQuaternion(body, q);
		dBodySetPosition(body, m_bp.bodyCenters[i][0], m_bp.bodyCenters[i][1], m_bp.bodyCenters[i][2]);
		dBodySetForce(body, 0.0, 0.0, 0.0);
		dBodySetTorque(body, 0.0, 0.0, 0.0);
	}
	for (i = 0; i < JE_COUNT; ++i)
	{
		m_joints[i]->reset();
	}
	g_contactsWithGround.clear();

	m_curControlState = CSE_BIPED_DO_NOTHING;
	m_nextControlState = CSE_BIPED_DO_NOTHING;
}

ArnVec3 Biped::getBodyRotationEuler(int id) const
{
	return getBody(id)->getRotationEuler();
}

double Biped::getFeetDist() const
{
	return fabs( m_bodies[BE_FOOT_L]->getPosition()[1] - m_bodies[BE_FOOT_R]->getPosition()[1] );
}

void Biped::saveBipedStateToFile(const char* fileName)
{
	FILE* fout = fopen(fileName, "wb");
	if (fout)
	{
		for (unsigned i = 0; i < BE_COUNT; i++)
		{
			GeneralBodyState gbs;
			m_bodies[i]->getState(gbs);
			fwrite(&gbs, 1, sizeof(gbs), fout);
		}
		fwrite(&m_curControlState, 1, sizeof(m_curControlState), fout);
		fwrite(&m_nextControlState, 1, sizeof(m_nextControlState), fout);
		fclose(fout);
		addLog("Biped state saved to ");
		addLog(fileName);
		addLog(".\n");
	}
	else
	{
		fprintf(stderr, "File opening for writing error: %s", fileName);
	}
}

void Biped::loadBipedStateFromFile(const char* fileName)
{
	FILE* f = fopen(fileName, "rb");
	if (f)
	{
		for (unsigned i = 0; i < BE_COUNT; i++)
		{
			GeneralBodyState gbs;
			fread(&gbs, 1, sizeof(gbs), f);
			m_bodies[i]->setState(gbs);
		}
		fread(&m_curControlState, 1, sizeof(m_curControlState), f);
		fread(&m_nextControlState, 1, sizeof(m_nextControlState), f);
		fclose(f);
		addLog("Biped state successfully loaded from ");
		addLog(fileName);
		addLog(".\n");
	}
	else
	{
		fprintf(stderr, "File opening for writing error: %s", fileName);
	}
}

void Biped::pullTrunkUpside()
{
	dBodyAddForce(m_bodies[0]->getBodyId(), 0, 0, 100000);
}

void Biped::pullTrunkForward()
{
	dBodyAddForce(m_bodies[0]->getBodyId(), 0, 5000, 0);
}

void Biped::toggleStandOnSingleFoot()
{
	m_bStandOnSingleFoot = !m_bStandOnSingleFoot;
}

void Biped::toggleCrouch()
{
	m_bCrouch = !m_bCrouch;
}

const dReal* Biped::getBodyForce(int id)
{
	return dBodyGetForce(getBody(id)->getBodyId());
}

const dReal* Biped::getBodyRotation(int id)
{
	return dBodyGetRotation(getBody(id)->getBodyId());
}

const dReal* Biped::getBodyQuaternion(int id)
{
	return dBodyGetQuaternion(getBody(id)->getBodyId());
}

void Biped::getHipJointsMidpoint(dVector3 point) const
{
	const ArnVec3& a = m_joints[JE_L1]->getAnchor();
	const ArnVec3& b = m_joints[JE_R1]->getAnchor();
	point[0] = (a[0] + b[0]) / 2;
	point[1] = (a[1] + b[1]) / 2;
	point[2] = (a[2] + b[2]) / 2;
}

void Biped::controlLeftLeg(double leftSupportHeight, double leftSupportForward)
{
	for (unsigned i = 0; i < JE_COUNT; ++i)
	{
		m_joints[i]->control_P(1, 0, 1, 100);
		m_joints[i]->control_P(2, 0, 1, 100);
	}

	double pGain = 50;
	double fMax = 200;

	/*
	double target = acos(1 - leftSupportHeight / (m_bp.groin_h + m_bp.calf_h));
	m_joints[JE_L1]->control_P(1,   -target, pGain, fMax);
	m_joints[JE_L2]->control_P(1,  2*target, pGain, fMax);
	m_joints[JE_L3]->control_P(1,   -target, pGain, fMax);
	*/

//	dVector3 midPoint;
//	getHipJointsMidpoint(midPoint);
//	const double L = midPoint[2];
	const double l1 = m_bp.groin_h;
	const double l2 = m_bp.calf_h;
	const double L = l1 + l2;
	const double y = leftSupportForward - m_bodies[BE_HAT]->getPosition()[1];
	const double z = leftSupportHeight - L;
	const double c2 = ( y*y + z*z - l1*l1 - l2*l2 ) / ( 2*l1*l2 );
	const double s2 = sqrt(1 - c2*c2);
	double theta2 = atan2(s2, c2);
	const double k1 = l1 + l2*c2;
	const double k2 = l2*s2;
	double theta1 = atan2(z, y) - atan2(k2, k1);
	double theta3 = M_PI - theta1 - theta2;

	double rFootPosY = m_bodies[BE_FOOT_R]->getPosition()[1];
	double lFootPosY = m_bodies[BE_FOOT_L]->getPosition()[1];
	theta1 += M_PI/2;
	theta3 = M_PI - theta1 - theta2;

	if (theta1 < 2*M_PI && theta1 > -2*M_PI) // to avoid normalization error in ODE
		m_joints[JE_L1]->control_P(1,  theta1, pGain, fMax);
	if (theta2 < 2*M_PI && theta2 > -2*M_PI) // to avoid normalization error in ODE
		m_joints[JE_L2]->control_P(1,  theta2, pGain, fMax);

//	if (fabs(m_joints[JE_L3]->getValue(1) - (-theta1 - theta2)) > 0.001)
//		m_joints[JE_L3]->control_P(1,  -theta1 - theta2, 1, fMax);
//	else
//		m_joints[JE_L3]->rest(1);

	//m_joints[JE_L3]->control_P(1,  - m_joints[JE_L1]->getValue(1) - m_joints[JE_L2]->getValue(1), pGain, fMax);
	m_joints[JE_L3]->rest(1);

	//m_joints[JE_R1]->rest(1);

	//m_joints[JE_R1]->control_P(1, fabs(rFootPosY - lFootPosY), 1, 100);
	//m_joints[JE_R2]->control_P(1, -fabs(rFootPosY - lFootPosY), 1, 100);
	//m_joints[JE_R3]->rest(1);



	for (unsigned i = 0; i < JE_COUNT; ++i)
	{
		m_joints[i]->rest(1);
		m_joints[i]->rest(2);
	}
}

void Biped::updateFrame(double timeStep, double leftSupportHeight, double leftSupportForward)
{
	//m_stateController->updateFrame(timeStep);

	//controlLeftLeg(leftSupportHeight, leftSupportForward);

	foreach (GeneralJoint* gj, m_joints)
	{
		gj->updateFrame();
	}

	/*
	double rFootPosY = m_bodies[BE_FOOT_R]->getPosition()[1];
	double lFootPosY = m_bodies[BE_FOOT_L]->getPosition()[1];
	double footDist = lFootPosY - rFootPosY;
	double trunkRotX = getBody(BE_HAT)->getRotationEuler().x;
	double c1 = 0.9;

	if (trunkRotX + c1*footDist > 0.01)
	{
		m_joints[JE_R1]->controlIncr_P(1, -1);
		m_joints[JE_L1]->controlIncr_P(1, -1);
	}
	else if (trunkRotX + c1*footDist < -0.01)
	{
		m_joints[JE_R1]->controlIncr_P(1, 1);
		m_joints[JE_L1]->controlIncr_P(1, 1);
	}
	*/


	/*
	switch (m_curControlState)
	{
		case DO_NOTHING:			controlDoNothing();			break;
		case STAY_STILL:			controlStayStill(100);		break;
		case FIRST_STEP_FORWARD:	controlFirstStepForward();	break;
		case REST_ANKLES:			controlRestAnkles();		break;
		case CORRECT_TRUNK_ORIENTATION: controlCorrectTrunkOrientation(); break;
		case SECOND_STEP_FORWARD:	controlSecondStepForward();	break;
		default:					assert(!"What the?");		break;
	}
	transitControlState();
	*/
}

void Biped::controlDoNothing()
{
	for (unsigned i = 0; i < JE_COUNT; ++i)
	{
		m_joints[i]->rest(1);
		m_joints[i]->rest(2);
	}
}

void Biped::controlStayStill(double fMax)
{
	for (unsigned i = 0; i < JE_COUNT; ++i)
	{
		m_joints[i]->controlToStayStill_P(1, fMax);
		m_joints[i]->controlToStayStill_P(2, fMax);
	}

	if (isBipedNotMoving())
	{
		setNextControlState(CSE_BIPED_REST_ANKLES);
	}
}

void Biped::controlFirstStepForward()
{
	double widenFeetAngle = M_PI/180 * 1;
	double kneeBendAngle = M_PI/180 * 20;
	double trunkBendAngle = M_PI/180 * ( 0  -  15 ) ;
	double swingGain = 2;
	double swingAngleGain = 4;
	double sideBalancingGain = 3;
	double bendingOffset = M_PI/180 * sideBalancingGain * 10;


	// Stretch right leg
	// Stance on left leg
	m_joints[JE_R1]->control_P(1, trunkBendAngle * swingAngleGain     + bendingOffset, swingGain, 100);
	m_joints[JE_L1]->control_P(1, trunkBendAngle / swingAngleGain * 2 + bendingOffset, swingGain, 100);

	m_joints[JE_R1]->control_P(2, -widenFeetAngle * 1.5, sideBalancingGain, 100);
	m_joints[JE_L1]->control_P(2, -widenFeetAngle * 2.5, sideBalancingGain, 100);

	//m_joints[JE_R2]->controlAngle1_P(kneeBendAngle, 1, 0); // turn off motor control
	m_joints[JE_R2]->rest(1);
	m_joints[JE_L2]->control_P(1, kneeBendAngle / 2, 1, 100);

	m_joints[JE_R3]->control_P(1, -kneeBendAngle, swingGain, 100);
	m_joints[JE_L3]->control_P(1, -kneeBendAngle, swingGain, 100);

	m_joints[JE_R3]->control_P(2, widenFeetAngle * 2, 2, 0);

	///
	m_joints[JE_L3]->control_P(2, 0, 1, 100 );
	///

	const dReal* swingRFootPos = m_bodies[BE_FOOT_R]->getPosition();
	const dReal* swingLFootPos = m_bodies[BE_FOOT_L]->getPosition();


	//if (m_bodyContacts[BE_FOOT_R] && dDISTANCE(swingRFootPos, swingLFootPos) > 0.4 && swingRFootPos[1] > swingLFootPos[1])
	if (m_bodyContacts[BE_FOOT_R] && getBody(BE_FOOT_R)->getLinearVel()[2] < -0.3)
	{
		setNextControlState(CSE_BIPED_STAY_STILL);
	}
}

void Biped::controlRestAnkles()
{
	double rFootAngX = getBody(BE_FOOT_R)->getRotationEuler().x;
	double lFootAngX = getBody(BE_FOOT_L)->getRotationEuler().x;

	if (fabs(rFootAngX) > 0.1)
	{
		getJoint(JE_R3)->rest(1);
	}

	if (fabs(lFootAngX) > 0.1)
	{
		getJoint(JE_L3)->rest(1);
	}

	if (fabs(rFootAngX) < 0.01 && fabs(lFootAngX) < 0.01)
	{
		getJoint(JE_R3)->controlToStayStill_P(1, 100);
		getJoint(JE_R3)->controlToStayStill_P(2, 100);
		getJoint(JE_L3)->controlToStayStill_P(1, 100);
		getJoint(JE_L3)->controlToStayStill_P(2, 100);
		setNextControlState(CSE_BIPED_CORRECT_TRUNK_ORIENTATION);
	}
}

void Biped::controlCorrectTrunkOrientation()
{
	double trunkY = getBody(BE_HAT)->getRotationEuler().y;
	double trunkZ = getBody(BE_HAT)->getRotationEuler().z;

	if (fabs(trunkY) > 0.01)
	{
		m_joints[JE_R1]->control_P(2, 0, 1, 100);
		m_joints[JE_L1]->control_P(2, 0, 1, 100);
	}
	else
	{
		setNextControlState(CSE_BIPED_SECOND_STEP_FORWARD);
		saveBipedStateToFile("ssf_start.state");
	}
}

bool Biped::isBipedNotMoving() const
{
	double velSum = 0;
	for (unsigned i = 0; i < BE_COUNT; ++i)
	{
		const dReal* linVel = m_bodies[i]->getLinearVel();
		const dReal* angVel = m_bodies[i]->getAngularVel();

		velSum += dLENGTH(linVel);
		velSum += dLENGTH(angVel);
	}
	if (velSum < 6)
		return true;
	else
		return false;
}

void Biped::setNextControlState(ControlStateEnum cs)
{
	assert(!"Should not use this");
	m_nextControlState = cs;
}

void Biped::addLog(const char* msg)
{
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR
	//m_logger->insertPlainText(msg);
}

const char* Biped::getCurControlStateStr() const
{
	#define EnumToString(x) case x: return #x; break;
	switch (m_curControlState)
	{
		EnumToString(CSE_BIPED_DO_NOTHING)
		EnumToString(CSE_BIPED_STAY_STILL)
		EnumToString(CSE_BIPED_FIRST_STEP_FORWARD)
		EnumToString(CSE_BIPED_REST_ANKLES)
		EnumToString(CSE_BIPED_CORRECT_TRUNK_ORIENTATION)
		EnumToString(CSE_BIPED_SECOND_STEP_FORWARD)
		default: assert(!"What the!"); break;
	}
	#undef EnumToString
	return 0;
}

void Biped::controlSecondStepForward()
{

	double hatDir = getBodyRotationEuler(BE_HAT).z;
	hatDir *= 5.003;
	double hatXrot = getBodyRotationEuler(BE_HAT).x;
	hatXrot *= 1.5;
	//hatXrot = 0;

	double hatYrot = getBodyRotationEuler(BE_HAT).y;
	hatYrot *= 0.6;

	double hatZpos = m_bodies[BE_HAT]->getPosition()[2];
	hatZpos = fabs(hatZpos - 1.4);
	double hatZposGain = hatZpos / 1.1;
	hatZpos *= 21.5;

	double swingGainMod = 1.35;




	double widenFeetAngle = M_PI/180 * 1;
	double kneeBendAngle = M_PI/180 * 20;
	double trunkBendAngle = M_PI/180 * ( 0  -  15 ) ;
	double swingGain = 4;
	double swingAngleGain = 4;
	double sideBalancingGain = 3;
	double bendingOffset = M_PI/180 * sideBalancingGain * 10;


	// Stretch left leg
	// Stance on right leg
	m_joints[JE_L1]->control_P(1, trunkBendAngle * swingAngleGain     + bendingOffset - hatXrot + hatZposGain, swingGainMod + swingGain + hatDir, 150);
	m_joints[JE_R1]->control_P(1, trunkBendAngle / swingAngleGain * 0 + bendingOffset - hatXrot + hatZposGain, swingGainMod + swingGain + hatDir, 150);

	m_joints[JE_L1]->control_P(2, widenFeetAngle * 2.5 - hatYrot, sideBalancingGain, 100);
	m_joints[JE_R1]->control_P(2, widenFeetAngle * 2.5 - hatYrot, sideBalancingGain, 100);

	m_joints[JE_L2]->control_P(1, kneeBendAngle, 1, 0); // turn off motor control
	m_joints[JE_R2]->control_P(1, kneeBendAngle / 2, 1, 100);

	m_joints[JE_L3]->control_P(1, -kneeBendAngle, swingGain, 100);
	m_joints[JE_R3]->control_P(1, -kneeBendAngle, swingGain, 100);

	//m_joints[JE_L3]->controlAngle2_P(-widenFeetAngle * 2, 2, 0);
	//m_joints[JE_L3]->controlToStayStillAngle2_P(100);

	///
	m_joints[JE_R3]->control_P(2, 0, 1, 100 );
	///

	const dReal* swingRFootPos = m_bodies[BE_FOOT_R]->getPosition();
	const dReal* swingLFootPos = m_bodies[BE_FOOT_L]->getPosition();
	//printf(" left : %.2f / right : %.2f\n",  swingRFootPos[2], m_bp.foot_h / 2.5);
	//if (swingRFootPos[2] <= m_bp.foot_h / 2)


	//m_joints[JE_R1]->controlAngle1_P(m_stance1, 2.5, 500);


	m_joints[JE_R2]->control_P(1, m_stance2 / hatZpos, 2.5, 150);
	m_joints[JE_R3]->control_P(1, m_stance3 / hatZpos, 2.5, 100);


	if (m_bodyContacts[BE_FOOT_L] && dDISTANCE(swingRFootPos, swingLFootPos) > 0.2 && swingLFootPos[1] > swingRFootPos[1])
	{
		//m_joints[JE_R1]->controlAngle1_P(-trunkBendAngle * swingAngleGain + bendingOffset, swingGain);
		//m_joints[JE_L1]->controlAngle1_P(-trunkBendAngle / swingAngleGain + bendingOffset, swingGain);

		//m_joints[JE_L3]->controlAngle2_P(0, 0, 0);

		m_joints[JE_R2]->control_P(1, 0, 1.5, 200);
		m_joints[JE_R3]->control_P(1, 0, 1.5, 100);

		m_joints[JE_L1]->control_P(2, -widenFeetAngle * 2.5 - hatYrot, sideBalancingGain, 100);
		m_joints[JE_R1]->control_P(2, -widenFeetAngle * 2.5 - hatYrot, sideBalancingGain, 100);


		if (m_joints[JE_L3]->getAngle(1) > m_joints[JE_R3]->getAngle(1))
		{
			m_joints[JE_L3]->controlToStayStill_P(1, 150);
			//m_joints[JE_R3]->controlAngle1_P(m_joints[JE_L3]->getAngle1() / 2, 1, 150);
		}


		if (getBodyRotationEuler(BE_FOOT_L).x < M_PI/180 * 5)
		{
			//m_joints[JE_L2]->controlToStayStillAngle1_P(100);

			m_stance1 = m_joints[JE_R1]->getAngle(1);
			m_stance2 = m_joints[JE_R2]->getAngle(1);
			m_stance3 = m_joints[JE_R3]->getAngle(1);

			//toggleRStepForward_Secondary();
			//toggleRStepForward();
		}
	}
}

void Biped::getFeetMidpoint(dVector3 point) const
{
	for (int i = 0; i < 3; ++i)
		point[i] = (m_bodies[BE_FOOT_L]->getPosition()[i] + m_bodies[BE_FOOT_R]->getPosition()[i]) / 2;
}

void Biped::setBodyContact( int bodyid )
{
	if (bodyid < BE_COUNT)
		m_bodyContacts[bodyid] = true;
	else
		throw std::runtime_error("Index array out of range.");
}
