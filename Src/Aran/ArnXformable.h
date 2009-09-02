/*!
 * @file ArnXformable.h
 * @author Geoyeob Kim
 * @date 2009
 */
#pragma once

#include "ArnNode.h"
class ArnAnimationController;
class ArnIpo;
struct ArnJointData;

struct ArnJointData
{
	std::string target;
	ArnVec3 pivot;
	ArnVec3 ax;
	struct ArnJointLimit
	{
		std::string type;
		float minimum;
		float maximum;
	};
	std::list<ArnJointLimit> limits;
};

/*!
 * @brief 위치와 방향 정보를 가진 객체를 나타내는 클래스
 *
 * 주로 렌더링되는 물체(메시)나 카메라 등이 본 클래스를 상속받습니다.
 */
class ARAN_API ArnXformable : public ArnNode
{
public:
	virtual										~ArnXformable(void);
	/*!
	 * @name Local 변환
	 *
	 * 본 객체가 가지고 있는 고유한 local model 변환입니다.
	 * 객체는 local 변환을 위한 행렬과 이를 decompose한 SRT 요소를
	 * 각각 저장하고 있습니다. 사용자는 행렬을 직접 조작할 수 없고
	 * SRT 요소를 개별적으로 수정한 뒤 ArnXformable::recalcLocalXform 을 호출함으로써
	 * local 변환 행렬을 설정할 수 있습니다. 만일 SRT 요소를 설정한 후
	 * ArnXformable::recalcLocalXform 호출을 생략하고 local 변환 행렬을 접근하려 한다면
	 * assertion 오류가 발생합니다.
	 *
	 * 본 객체의 부모가 없는 경우 local 변환이 곧 world 변환이 됩니다.
	 */
	//@{
	/*!
	 * @brief local 변환 행렬을 가져옴
	 * @sa ArnXformable::getFinalLocalXform
	 *
	 * 반드시 dirty bit이 \c false여야 하며, dirty bit이 \c true일 경우에는
	 * ArnXformable::recalcLocalXform 을 먼저 호출해야합니다.
	 */
	inline const ArnMatrix&						getLocalXform() const; // m_localXform will not be updated until recalcLocalXform() is called.
	/*!
	 * @brief local 변환 중 scale 요소를 가져옴
	 */
	inline const ArnVec3&						getLocalXform_Scale() const;
	/*!
	 * @brief local 변환 중 scale 요소를 설정함
	 *
	 * dirty bit을 \c true 로 설정합니다.
	 */
	inline void									setLocalXform_Scale(const ArnVec3& scale);
	/*!
	 * @brief local 변환 중 rotation 요소를 가져옴
	 */
	inline const ArnQuat&						getLocalXform_Rot() const;
	/*!
	 * @brief local 변환 중 rotation 요소를 설정함
	 *
	 * dirty bit을 \c true 로 설정합니다.
	 */
	inline void									setLocalXform_Rot(const ArnQuat& rot);
	/*!
	 * @brief local 변환 중 translation 요소를 가져옴
	 */
	inline const ArnVec3&						getLocalXform_Trans() const;
	/*!
	 * @brief local 변환 중 translation 요소를 설정함
	 *
	 * dirty bit을 \c true 로 설정합니다.
	 */
	inline void									setLocalXform_Trans(const ArnVec3& trans);
	/*!
	 * @brief local 변환의 SRT 요소를 이용해 local 변환 행렬을 재계산함
	 * @remarks dirty bit이 이미 \c false 인 경우에도 재계산을 합니다.
	 *
	 * 재계산 후에 dirty bit을 \c false 로 설정합니다.
	 */
	void										recalcLocalXform();
	/*!
	 * @brief local 변환 행렬이 dirty 상태인지 반환함
	 */
	inline bool									isLocalXformDirty() const;
	//@}
	/*!
	 * @name Animation Local 변환
	 *
	 * IPO에 의해 매 프레임마다 변경될 수 있는 local model 변환입니다.
	 * 만일 본 객체에 IPO가 할당되어 있고, 애니메이션 컨트롤러가 작동하고 있는 경우에는
	 * Local 변환 관련된 값 대신에 Animation Local 변환 관련값을 참조해야 합니다.
	 * 많은 경우 시간 원점(\c t=0)에서의 Animation Local 변환과 Local 변환은 같은 변환이
	 * 되도록 조절하지만 반드시 그래야하는 것은 아닙니다.
	 */
	//@{
	const ArnMatrix& 							getAnimLocalXform() const { return m_animLocalXform; }
	inline const ArnQuat&						getAnimLocalXform_Rot() const;
	inline void									setAnimLocalXform_Rot(const ArnQuat& q);
	inline void									setAnimLocalXform_Scale(const ArnVec3& scale);
	inline void									setAnimLocalXform_Trans(const ArnVec3& trans);
	void										recalcAnimLocalXform();
	bool										isAnimLocalXformDirty() const { return m_bAnimLocalXformDirty; }
	//@}

	/*!
	 * @name World 변환
	 */
	//@{
	/*!
	 * @brief World 변환을 계산
	 * @sa ArnXformable::getAutoLocalXform
	 *
	 * 자신과 부모의 (Animation) Local 변환을 재귀적으로 따라 올라가며
	 * 최종 변환인 World 변환을 계산합니다.
	 */
	ArnMatrix									computeWorldXform() const;
	//@}

	/*!
	 * @brief IPO 여부에 따라 적절한 local 변환 행렬을 반환
	 *
	 * IPO가 할당되고 애니메이션 컨트롤러가 작동 중인 경우에는 Animation Local 행렬을 반환하고
	 * 그렇지 않은 경우에는 Local 행렬을 반환합니다.
	 */
	virtual const ArnMatrix&					getAutoLocalXform() const;

	/*!
	 * @brief 보이는 물체(렌더링 되는 물체)라면 \c true 반환
	 *
	 * 부모의 visibility와는 관계 없이 이 물체의 값을 반환합니다.
	 * 즉, 부모의 ArnXformable::isVisible() 값과 이 값과는 관계가 없습니다.
	 * 그러나 렌더링 할 때에는 부모의 visibility가 \c false 이면
	 * 자식 노드는 렌더링하지 않습니다.
	 */
	bool										isVisible() const;
	/*!
	 * @brief 보이는 물체(렌더링 되는 물체)인지 아닌지 설정
	 */
	inline void									setVisible(bool val);

	/*!
	 * @name 애니메이션과 IPO
	 */
	//@{
	inline const std::string&					getIpoName() const;
	inline ArnIpo*								getIpo() const;
	inline void									setIpoName(const char* ipoName);
	void										setIpo(ArnIpo* val);
	void										setIpo(const std::string& ipoName);
	void										configureIpo();
	inline void									resetAnimSeqTime();
	inline bool									isAnimSeqEnded() const;
	double										getAnimCtrlTime() const;
	void										setAnimCtrlTime( double dTime );
	void										setDoAnim( bool bDoAnim );
	void										advanceTime( float fTime );
	inline ArnAnimationController*				getAnimCtrl();
	//@}

	const std::vector<ArnJointData>&			getJointData() const { return m_jointData; }
	/*!
	 * @brief local 변환 행렬에 관련한 정보를 출력 (디버그)
	 */
	void										printXformData() const;
protected:
												ArnXformable(NODE_DATA_TYPE ndt);
	void										setLocalXform(const ArnMatrix& localXform);
	virtual void								update(double fTime, float fElapsedTime);
	inline void									setAnimCtrl(ArnAnimationController* animCtrl);
	void										configureAnimCtrl();
	void										addJointData(const ArnJointData& data);

private:
	inline void									setAnimSeqEnded(bool val);
	bool										m_bVisible;
	ArnMatrix  									m_localXform;
	ArnVec3 									m_localXform_Scale;
	ArnQuat     								m_localXform_Rot;
	ArnVec3	    								m_localXform_Trans;
	bool										m_bLocalXformDirty;
	ArnMatrix  									m_animLocalXform;
	ArnVec3										m_animLocalXform_Scale;
	ArnQuat										m_animLocalXform_Rot;
	ArnVec3										m_animLocalXform_Trans;
	bool										m_bAnimLocalXformDirty;
	ArnAnimationController*						m_animCtrl;
	bool										m_bDoAnim;
	bool										m_bAnimSeqEnded;
	std::vector<ArnJointData>					m_jointData;
	std::string									m_ipoName;
	ArnIpo*										m_ipo;
};

#include "ArnXformable.inl"
