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

TYPEDEF_SHARED_PTR(GeneralBody)
TYPEDEF_SHARED_PTR(GeneralJoint)
/*!
\brief ODE 관절 관련 함수에 대한 래퍼 최상위 클래스
*/
class ARANPHY_API GeneralJoint
{
public:
	virtual						~GeneralJoint();
	/*! @name 관절 이름 및 종류
	사용자가 읽을 수 있는 관절 이름을 정합니다. 고유한 이름이라는 보장은 없습니다.
	*/
	//@{
	void						setName(const char* name);
	const char*					getName() const;
	virtual const char*			getJointTypeName() const = 0;
	//@}
	/*! @brief 두 강체를 현재 관절을 이용해 붙임 */
	virtual void				attach(GeneralBody& body1, GeneralBody& body2);
	/* @brief 현재 관절 각도를 반환 */
	virtual double				getValue(int anum) const = 0;
	/* @brief 현재 관절 속도를 반환 */
	virtual double				getVelocity(int anum) const = 0;
	/* @brief 현재 관절이 이루고자 하는 목표값을 반환 */
	double						getTargetValue(int anum) const { return m_targetValue[anum - 1]; }
	/* @brief 현재 관절 각도와 목표값과의 차이를 반환 */
	inline double				getTargetCurrentDiff(int anum) const;
	/*! @name 관절 제어
	관절을 제어할 수 있는 여러 가지 방법을 설정할 수 있습니다.
	*/
	//@{
	/*!	@brief 관절 제어 방법	*/
	enum JointControlMode
	{
		JCM_REST,					///< 전혀 제어를 하지 않습니다. (힘을 가하지 않음)
		JCM_TARGET_VALUE_P,			///< 특정한 관절 각도를 P제어로 다다르기 위해 힘을 가합니다.
		JCM_TARGET_VALUE_PD,		///< 특정한 관절 각도를 PD제어로 다다르기 위해 힘을 가합니다.
		JCM_STAY_STILL,				///< 현재 관절 각도를 유지하기 위해 P제어합니다.
		JCM_INCREMENTAL_P,			///< 원하는 관절 각도를 P제어로 이루도록 하되 목표치를 조금씩 바꿔가며 힘을 가합니다.
		JCM_INCREMENTAL_PD,			///< 원하는 관절 각도를 PD제어로 이루도록 하되 목표치를 조금씩 바꿔가며 힘을 가합니다.
		JCM_WITHIN_RANGE,			///< 관절 각도가 일정한 범위 내에 있도록 제어합니다.

		JCM_COUNT					///< 관절 제어 방법 종류 개수
	};
	JointControlMode			getJointControlMode() const { return m_jcm; }
	void						rest(int anum);
	void						control_P(int anum, double target, double proGain, double forceMax);
	void						control_PD(int anum, double target, double proGain, double derGain, double forceMax);
	void						controlToStayStill_P(int anum, double fMax);
	void						controlIncr_P(int anum, double incrVal, double proGain, double forceMax);
	void						controlIncr_PD(int anum, double incrVal, double proGain, double derGain, double forceMax);
	void						controlWithinRange_P(int anum, double min, double max, double pGain, double fMax);
	/*! @brief proportional 게인 값을 설정 */
	void						setProGain(int anum, double v) { m_proGain[anum-1] = v; }
	/*! @brief 현재 관절 제어 방법을 문자열로 반환 */
	const char*					getJcmName() const { return JointControlModeStr[m_jcm]; }
	//@}
	/*!
	@brief ODE 컨텍스트를 이용해 강체의 ODE 인스턴스를 생성
	@param osc ODE 컨텍스트
	@remark 호출하는 전에 ODE 컨텍스트가 \c NULL 로 설정되어 있어야 합니다.
	\c NULL 이 아닌 경우는 이미 객체의 ODE 인스턴스가 생성되었다는 뜻입니다.
	*/
	virtual void				configureOdeContext(const OdeSpaceContext* osc);
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
	void						reset();
	dJointFeedback*				getSetJointFeedback() { return dJointGetFeedback(m_joint); }
	void						enableJointFeedback() { dJointSetFeedback(m_joint, &m_jointFeedback); }
	virtual void				addTorque(AxisEnum anum, float torque);
protected:
								GeneralJoint(const OdeSpaceContext* osc);
	dJointID					getId() const { return m_joint; }
	void						setId(dJointID id) { m_joint = id; }
	void						updateFrameInternal(int anum);
	void						setBody1(const GeneralBodyPtr b) { m_body1 = b; }
	void						setBody2(const GeneralBodyPtr b) { m_body2 = b; }
	const GeneralBodyPtr		getBody1() const { return m_body1; }
	const GeneralBodyPtr		getBody2() const { return m_body2; }
	virtual void				doReset() = 0;
private:
	void						setTargetValue(int anum, double v) { m_targetValue[anum - 1] = v; }
	void						controlP(int anum, double targetValue, double proGain, double forceMax);
	void						controlPD(int anum, double target, double proGain, double derGain, double forceMax);
	std::string					m_name;
	const OdeSpaceContext*		m_osc;
	dJointID					m_joint;
	JointControlMode			m_jcm;
	double						m_targetValue[3];
	double						m_incrValue[3];
	double						m_forceMax[3];
	double						m_proGain[3]; // Proportional gain in P-controller
	double						m_derGain[3]; // Derivative gain in PD-controller
	double						m_minTargetValue[3];
	double						m_maxTargetValue[3];
	dJointFeedback				m_jointFeedback;
	GeneralBodyPtr				m_body1;
	GeneralBodyPtr				m_body2;
};

#include "GeneralJoint.inl"

#endif // GENERALJOINT_H
