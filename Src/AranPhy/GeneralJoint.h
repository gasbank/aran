/*!
\file GeneralJoint.h
\author Geoyeob Kim
\date 2009
*/
#ifndef GENERALJOINT_H
#define GENERALJOINT_H

class GeneralBody;
class BasicObjects;

static const char* JointControlModeStr[] = { "REST", "TARGET", "STAY", "INCR", "RANGE" };

/*!
\brief ODE 관절 관련 함수에 대한 래퍼 클래스
*/
class GeneralJoint
{
public:
							GeneralJoint(const OdeSpaceContext* osc);
	virtual					~GeneralJoint();

	enum JointControlMode
	{
		JCM_REST,
		JCM_TARGET_VALUE_P,
		JCM_TARGET_VALUE_PD,
		JCM_STAY_STILL,
		JCM_INCREMENTAL_P,
		JCM_INCREMENTAL_PD,
		JCM_WITHIN_RANGE,

		JCM_COUNT
	};

	void						setName(const char* name);
	const char*					getName() const;
	virtual const char*			getJointTypeName() const = 0;
	const char*					getJcmName() const { return JointControlModeStr[m_jcm]; }
	void						attach(GeneralBody& body1, GeneralBody& body2);
	virtual double				getValue(int anum) const = 0;
	virtual double				getVelocity(int anum) const = 0;
	double						getTargetValue(int anum) const { return m_targetValue[anum - 1]; }
	void						reset();

	dJointFeedback*				getSetJointFeedback() { return dJointGetFeedback(m_joint); }
	void						enableJointFeedback() { dJointSetFeedback(m_joint, &m_jointFeedback); }

	/*!
	@name 관절 제어
	*/
	//@{
	JointControlMode			getJointControlMode() const { return m_jcm; }
	void						controlIncr_P(int anum, double incrVal, double proGain, double forceMax);
	void						controlIncr_PD(int anum, double incrVal, double proGain, double derGain, double forceMax);
	void						controlToStayStill_P(int anum, double fMax);
	void						control_P(int anum, double target, double proGain, double forceMax);
	void						control_PD(int anum, double target, double proGain, double derGain, double forceMax);
	void						controlWithinRange_P(int anum, double min, double max, double pGain, double fMax);
	void						rest(int anum);
	void						setProGain(int anum, double v) { m_proGain[anum-1] = v; }
	//@}

	/*!
	@name 관절 파라미터
	*/
	//@{
	virtual void				setParamVelocity(int anum, dReal v) = 0;
	virtual void				setParamFMax(int anum, dReal v) = 0;
	virtual void				setParamLoHiStop(int anum, dReal lo, dReal hi) = 0;
	virtual void				setAxis(int anum, const ArnVec3& pos) = 0;
	virtual void				getAxis(int anum, dVector3 result) const = 0;
	//@}

	// Rendering
	virtual void				render(const BasicObjects& bo) const;
	virtual void				renderJointAxis() const = 0;

	// Update this joint	
	virtual void				updateFrame() = 0;

	double						getTargetCurrentDiff(int anum) const;

protected:
	dJointID					getId() const { return m_joint; }
	void						setId(dJointID id) { m_joint = id; }
	void						updateFrameInternal(int anum);
	virtual void				doReset() = 0;

private:
	void						setTargetValue(int anum, double v) { m_targetValue[anum - 1] = v; }
	void						controlP(int anum, double targetValue, double proGain, double forceMax);
	void						controlPD(int anum, double target, double proGain, double derGain, double forceMax);

	char						m_name[64];
	dJointID					m_joint;
	const OdeSpaceContext*		m_osc;
	JointControlMode			m_jcm;
	double						m_targetValue[3];
	double						m_incrValue[3];
	double						m_forceMax[3];
	double						m_proGain[3]; // Proportional gain in P-controller
	double						m_derGain[3]; // Derivative gain in PD-controller
	double						m_minTargetValue[3];
	double						m_maxTargetValue[3];
	dJointFeedback				m_jointFeedback;
};

inline double GeneralJoint::getTargetCurrentDiff(int anum) const
{
	if (m_jcm == JCM_REST)
		return 0;
	else
		return getTargetValue(anum) - getValue(anum);
}

#endif // GENERALJOINT_H
