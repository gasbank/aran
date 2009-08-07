#ifndef BIPED_H
#define BIPED_H
//#include "QuatTypes.h"

class FiniteStateMachine;
class GeneralBody;
class GeneralJoint;
class AngularJoint;
class BasicObjects;
class QPlainTextEdit;
class StateController;

enum ControlStateEnum
{
	// For BipedStateController
	CSE_BIPED_DO_NOTHING = 1000,
	CSE_BIPED_STAY_STILL,
	CSE_BIPED_FIRST_STEP_FORWARD,
	CSE_BIPED_REST_ANKLES,
	CSE_BIPED_CORRECT_TRUNK_ORIENTATION,
	CSE_BIPED_SECOND_STEP_FORWARD,
	CSE_BIPED_REST_ANKLES_2,

	// For SimWorldStateController
	CSE_SIMWORLD_FIRST_STEP_FORWARD,
	CSE_SIMWORLD_R_STEP_FORWARD,
	CSE_SIMWORLD_L_STEP_FORWARD,
};

struct BipedParameters
{
	// Mass distribution [kg]
	dReal torso_m;
	dReal groin_m;
	dReal calf_m;
	dReal foot_m;

	// Length, radius [m]
	// Torso and foot are box shaped
	dReal torso_w;
	dReal torso_h;
	dReal torso_d;

	dReal foot_w;
	dReal foot_h;
	dReal foot_d;

	// Groin and calf are capsule shaped
	dReal groin_r;
	dReal groin_h;

	dReal calf_r;
	dReal calf_h;

	//////////////////////////////////////////////////////////////////

	dReal leg_h;
	dReal bodyCenters[BE_COUNT][3];
	dReal bodyRadiusAndHeight[BE_COUNT][2];
	dReal bodyXyAndHeightForBoxes[BE_COUNT][3];
	dReal bodyWeights[BE_COUNT];
	dReal jointCenters[JE_COUNT][3];
	unsigned jointLinks[JE_COUNT][2];
};

class Biped
{
public:
											Biped(const OdeSpaceContext* osc);
	virtual									~Biped();
	void									initialize(const BipedParameters& bp, ArnVec4 bipedOffset);
	const BipedParameters&					getBipedParameters() const { return m_bp; }
	void									setFsm(const FiniteStateMachine& fsm);
	void									startSimulation() { m_bSimulate = true; }
	void									stopSimulation() { m_bSimulate = false; }
	void									reset();
	bool									isSimulate() const { return m_bSimulate; }
	void									toggleWidenFeet() { m_bWidenFeet = !m_bWidenFeet; if (m_bWidenFeet) m_bNarrowFeet = false; }
	void									toggleBendTrunk() { m_bBendTrunk = !m_bBendTrunk; }
	void									toggleRStepForward() { m_bRStepForward = !m_bRStepForward; if (m_bRStepForward) { m_bLStepForward = false; m_bRStepForward_Secondary = false; } }
	bool									getRStepForward() const { return m_bRStepForward; }
	void									toggleLStepForward() { m_bLStepForward = !m_bLStepForward; if (m_bLStepForward) { m_bRStepForward = false; m_bRStepForward_Secondary = false; } }
	void									toggleRStepForward_Secondary() { m_bRStepForward_Secondary = !m_bRStepForward_Secondary; if (m_bRStepForward_Secondary) { m_bRStepForward = false; m_bLStepForward = false; } }
	void									toggleNarrowFeet() { m_bNarrowFeet = !m_bNarrowFeet; if (m_bNarrowFeet) m_bWidenFeet = false; }
	void									toggleStandOnSingleFoot();
	void									toggleCrouch();
	const OdeSpaceContext*					getOsc() const { return m_osc; }
	GeneralBody*							getBody(int id) const { return m_bodies[id]; }
	AngularJoint*							getJoint(int id) const { return m_joints[id]; }
	double									getFootHeight() const { return m_bp.foot_h; }
	void									pullTrunkUpside();
	void									pullTrunkForward();
	const dReal*							getBodyForce(int id);
	const dReal*							getBodyRotation(int id);
	const dReal*							getBodyQuaternion(int id);
	void									getHipJointsMidpoint(dVector3 point) const;
	void									getFeetMidpoint(dVector3 point) const;
	ArnVec3									getBodyRotationEuler(int id) const;
	const char*								getCurControlStateStr() const;
	void									render();
	bool									getBodyContact(int bodyid) const { return m_bodyContacts[bodyid]; }
	double									getFeetDist() const;
	void									saveBipedStateToFile(const char* fileName);
	void									loadBipedStateFromFile(const char* fileName);
	bool									isBipedNotMoving() const;
	void									setNextControlState(ControlStateEnum cs);
	void									updateFrame(double timeStep, double leftSupportHeight, double leftSupportForward);

	/*! @name Internal use only methods
	These methods are exposed in order to make internal linkage between objects or initialization.
	Clients should aware that these are not for client-side APIs.
	*/
	//@{
	void									clearBodyContact() { memset(m_bodyContacts, 0, sizeof(m_bodyContacts)); }
	void									setBodyContact(int bodyid);
	std::vector<GeneralBody*>&				getBodies() { return m_bodies; }
	std::vector<AngularJoint*>&				getJoints() { return m_joints; }
	//@}
protected:
private:
	void									controlDoNothing();
	void									controlStayStill(double fMax);
	void									controlFirstStepForward();
	void									controlRestAnkles();
	void									controlCorrectTrunkOrientation();
	void									controlSecondStepForward();
	void									controlLeftLeg(double leftSupportHeight, double leftSupportForward);
	void									addLog(const char* msg);

	bool									m_bInitialized;
	const OdeSpaceContext*					m_osc;
	typedef std::vector<GeneralBody*>		GeneralBodyVector;
	typedef std::vector<AngularJoint*>		AngularJointVector;
	GeneralBodyVector						m_bodies;
	AngularJointVector						m_joints;
	bool									m_bSimulate;
	bool									m_bWidenFeet;
	bool									m_bBendTrunk;
	bool									m_bRStepForward;
	bool									m_bLStepForward;
	bool									m_bRStepForward_Secondary;
	bool									m_bNarrowFeet;
	bool									m_bStandOnSingleFoot;
	bool									m_bCrouch;
	bool									m_bStayAllJoints;
	BipedParameters							m_bp;
	bool									m_bodyContacts[BE_COUNT];
	double									m_stance1;
	double									m_stance2;
	double									m_stance3;
	ControlStateEnum						m_curControlState;
	ControlStateEnum						m_nextControlState;
};

#endif // BIPED_H
