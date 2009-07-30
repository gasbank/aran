#pragma once

#include "ArnNode.h"
class ArnAnimationController;
class ArnIpo;

/**
@brief A superclass of renderer objects which have position and orientation.
*/
class ARAN_API ArnXformable : public ArnNode
{
public:
	virtual										~ArnXformable(void);
	inline const STRING&						getIpoName() const;
	inline const ArnMatrix&						getFinalLocalXform() const;
	inline const ArnVec3&						getLocalXform_Scale() const;
	inline void									setLocalXform_Scale(const ArnVec3& scale);
	inline const ArnQuat&						getLocalXform_Rot() const;
	inline void									setLocalXform_Rot(const ArnQuat& rot);
	inline const ArnVec3&						getLocalXform_Trans() const;
	inline void									setLocalXform_Trans(const ArnVec3& trans);
	inline const ArnQuat&						getAnimLocalXform_Rot() const;
	inline void									setAnimLocalXform_Rot(const ArnQuat& q);
	inline void									setAnimLocalXform_Scale(const ArnVec3& scale);
	inline void									setAnimLocalXform_Trans(const ArnVec3& trans);
	inline bool									isAnimSeqEnded() const;
	inline const ArnMatrix&						getLocalXform() const; // m_localXform will not be updated until recalcLocalXform() is called.
	inline void									resetAnimSeqTime();
	ArnMatrix									getFinalXform();
	void										recalcLocalXform();
	void										recalcAnimLocalXform();
	double										getAnimCtrlTime() const;
	void										setAnimCtrlTime( double dTime );
	void										setDoAnim( bool bDoAnim );
	void										advanceTime( float fTime );
	void										printXformData() const;
	void										configureIpo();
	inline ArnIpo*								getIpo() const;
	inline void									setIpoName(const char* ipoName);
	void										setIpo(ArnIpo* val);
	void										setIpo(const STRING& ipoName);
	inline ArnAnimationController*				getAnimCtrl();
	inline bool									isLocalXformDirty() const;

protected:
												ArnXformable(NODE_DATA_TYPE ndt);
	void										setLocalXform(const ArnMatrix& localXform);
	virtual void								update(double fTime, float fElapsedTime);
	inline void									setAnimCtrl(ArnAnimationController* animCtrl);
	void										configureAnimCtrl();

private:
	inline void									setAnimSeqEnded(bool val);
	ArnIpo*										m_ipo;
	STRING										m_ipoName;
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
	ArnAnimationController*						m_aniimCtrl;
	bool										m_bDoAnim;
	bool										m_bAnimSeqEnded;
	
	ArnMatrix  									m_finalLocalXform;		// TODO: A variable's usage is not clear
	ArnMatrix  									m_localXformIpo;		// TODO: A variable's usage is not clear	
};

#include "ArnXformable.inl"
