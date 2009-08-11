/*!
@file GeneralBody.h
@author Geoyeob Kim
@date 2009
*/
#ifndef GENERALBODY_H
#define GENERALBODY_H

class ArnXformable;
struct OdeSpaceContext;

struct GeneralBodyState
{
	ArnVec3 pos;
	ArnQuat quat;
	ArnVec3 linVel;
	ArnVec3 angVel;
};

TYPEDEF_SHARED_PTR(GeneralBody);

/*!
@brief ODE 강체 관련 함수에 대한 래퍼 최상위 클래스
*/
class ARANPHY_API GeneralBody
{
public:
	virtual							~GeneralBody();

	const char*						getName() const { return m_name.c_str(); }
	void							setName(const char* val) { m_name = val; }

	/*! @name 강체의 모양과 크기
	모든 강체마다 아래의 값이 정의되어야 합니다.
	- 충돌 체크를 위한 Bounding volume의 모양과 크기
	- 질량 분포를 위한 모양과 크기

	충돌 체크를 위한 모양과 질량 분포를 위한 모양이 반드시 같아야 할 필요는 없습니다.
	두 경우 모두 크기는 ArnVec3 형태를 사용하는데, 모양에 따라 값의 의미가 다릅니다.
	반드시 형태를 먼저 설정 한 후 크기를 설정해야 합니다.

	- 상자 모양: 크기의 X, Y, Z 성분이 각각 가로, 세로, 높이 값이 됩니다.
	- 캡슐 모양: 크기의 X, Y 성분이 각각 캡슐의 반지름, 높이 값이 됩니다. Z 성분은 반드시 0이어야 합니다.
	*/
	//@{
	void							setBoundingBoxType(ArnBoundingBoxType abbt) { m_abbt = abbt; }
	ArnBoundingBoxType				getBoundingBoxType() const { return m_abbt; }
	void							setBoundingBoxSize(const ArnVec3& size);
	void							getGeomSize(dVector3 out) const;
	void							setMassDistributionType(ArnMassDistributionType amdt) { m_amdt = amdt; }
	ArnMassDistributionType			getMassDistributionType() const { return m_amdt; }
	void							setMassDistributionSize(const ArnVec3& size);
	//@}
	/*!
	@brief		강체의 질량을 설정
	@param kg	질량
	*/
	void							setMass(float kg) { m_mass = kg; }
	/*! @name 위치
	강체의 위치를 COM 기준으로 설정하거나 읽어올 수 있습니다.
	위치를 명시적으로 설정하는 것은 물리적으로 봤을 때 물체가 순간이동 하는 것이기
	때문에 초기화하거나 재설정하는 목적 이외에는 쓰지 않는 것이 좋습니다.
	*/
	//@{
	void							setBodyPosition(const ArnVec3& comPos);
	ArnVec3*						getPosition(ArnVec3* pos) const;
	const dReal*					getPosition() const { return dBodyGetPosition(m_body); }
	//@}
	/*! @name 회전
	강체의 회전 상태를 Euler 각도, 사원수 혹은 행렬 형태로 설정하거나 읽어올 수 있습니다.
	회전 상태를 명시적으로 설정하는 것은 물리적으로 봤을 때 물체가 순간이동 하는 것이기
	때문에 초기화하거나 재설정하는 목적 이외에는 쓰지 않는 것이 좋습니다.
	*/
	//@{
	ArnVec3							getRotationEuler() const;
	ArnMatrix*						getRotationMatrix(ArnMatrix* rotMat) const;
	const dReal*					getRotationMatrix() const { return dBodyGetRotation(m_body); }
	ArnQuat*						getQuaternion(ArnQuat* q) const;
	const dReal*					getQuaternion() const { return dBodyGetQuaternion(m_body); }
	void							setInitialQuaternion(const ArnQuat& q) { m_quat0 = q; }
	//@}
	/*! @name 각속도와 선속도
	강체의 각속도와 선속도를 읽어오거나 설정할 수 있습니다.
	Kinematic 물체에 대해서 선속도나 각속도를 설정하는 것은 문제가 없을 수 있으나
	dynamics 물체에 대해 이러한 값을 시뮬레이션 도중에 직접 설정하는 것은 불안정성을 높입니다.
	*/
	//@{
	ArnVec3*						getLinearVel(ArnVec3* linVel) const;
	const dReal*					getLinearVel() const { return dBodyGetLinearVel(m_body); }
	void							setLinearVel(float x, float y, float z);
	ArnVec3*						getAngularVel(ArnVec3* angVel) const;
	const dReal*					getAngularVel() const { return dBodyGetAngularVel(m_body); }
	//@}
	/*! @name 상태
	강체의 물리적인 상태를 나타내는 값을 한꺼번에 읽어오거나 설정할 수 있습니다.
	*/
	//@{
	void							getState(GeneralBodyState& gbs) const;
	void							setState(const GeneralBodyState& gbs);
	//@}
	/*!
	@brief			모든 물리적 상태를 초기화
	선속도, 각속도, 회전 상태, 위치, 작용하는 힘, 토크 값을 초기화시킵니다.
	*/
	virtual void					reset();
	/*!
	@brief ODE 컨텍스트를 이용해 강체의 ODE 인스턴스를 생성
	@param osc ODE 컨텍스트
	@remark 호출하는 전에 ODE 컨텍스트가 \c NULL 로 설정되어 있어야 합니다.
	\c NULL 이 아닌 경우는 이미 객체의 ODE 인스턴스가 생성되었다는 뜻입니다.
	*/
	void							configureOdeContext(const OdeSpaceContext* osc);
	/*!
	@brief ArnXformable의 위치와 회전 값을 갱신
	*/
	void							notify();
	void							setXformableTarget(ArnXformable* xformable) { m_xformable = xformable; }
	void							addExternalForceOnCom(double x, double y, double z);
	bool							isContactGround() const;
	void							setContactGround(bool b) { m_isContactGround = b; }
	void							setBodyData(void* data);
	void							render();
	void							setGeomData(void* data);
	void*							getGeomData() const;
	/*! @name Internal use only methods
	These methods are exposed in order to make internal linkage between objects or initialization.
	Clients should aware that these are not for client-side APIs.
	*/
	//@{
	dGeomID							getGeomID() const { return m_geom; }
	dBodyID							getBodyId() const { return m_body; }
	//@}
	bool							isFixed() const { return m_bFixed; }
protected:
									GeneralBody(const OdeSpaceContext* osc);
	void							setInitialCom(const ArnVec3& com0) { m_com0 = com0; }
	void							setFixed(bool b) { m_bFixed = b; }
private:
	void							createGeomBox(const ArnVec3& dim);
	void							createGeomCapsule(double radius, double height);
	std::string						m_name;					///< 이름
	const OdeSpaceContext*			m_osc;					///< ODE 컨텍스트
	dBodyID							m_body;					///< ODE Body ID
	dGeomID							m_geom;					///< ODE Geom ID
	bool							m_isContactGround;		///< 바닥과 접촉 여부
	float							m_mass;					///< 질량 (kg)
	ArnVec3							m_com;					///< 현재 COM(center of mass) 위치
	ArnVec3							m_com0;					///< 초기 COM 위치
	ArnQuat							m_quat0;				///< 초기 회전 상태
	ArnVec3							m_boundingSize;			///< 충돌 확인 모형의 크기
	ArnVec3							m_massDistSize;			///< 질량 분포 모형의 크기
	ArnBoundingBoxType				m_abbt;					///< bounding volume의 종류
	ArnMassDistributionType			m_amdt;					///< 질량 분포 종류
	ArnXformable*					m_xformable;			///< 강체 시뮬레이션 결과를 적용받을 인스턴스
	bool							m_bFixed;				///< 강체의 고정 여부
};

#endif // GENERALBODY_H
