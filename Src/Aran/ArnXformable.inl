const std::string& ArnXformable::getIpoName() const
{
	return m_ipoName;
}

const ArnVec3& ArnXformable::getLocalXform_Scale() const
{
	return m_localXform_Scale;
}

void ArnXformable::setLocalXform_Scale( const ArnVec3& scale )
{
	m_bLocalXformDirty = true;
	m_localXform_Scale = scale;
}

const ArnQuat& ArnXformable::getLocalXform_Rot() const
{
	return m_localXform_Rot;
}

void ArnXformable::setLocalXform_Rot( const ArnQuat& rot )
{
	m_bLocalXformDirty = true;
	m_localXform_Rot = rot;
}

const ArnVec3& ArnXformable::getLocalXform_Trans() const
{
	return m_localXform_Trans;
}

void ArnXformable::setLocalXform_Trans( const ArnVec3& trans )
{
	m_bLocalXformDirty = true;
	m_localXform_Trans = trans;
}

bool ArnXformable::isAnimSeqEnded() const
{
	return m_bAnimSeqEnded;
}

const ArnMatrix& ArnXformable::getLocalXform() const
{
	assert(!m_bLocalXformDirty);
	return m_localXform;
}

void ArnXformable::resetAnimSeqTime()
{
	m_bAnimSeqEnded = false; setAnimCtrlTime(0);
}

ArnIpo* ArnXformable::getIpo() const
{
	return m_ipo;
}

void ArnXformable::setIpoName( const char* ipoName )
{
	m_ipoName = ipoName;
}

void ArnXformable::setAnimSeqEnded( bool val )
{
	m_bAnimSeqEnded = val;
}

const ArnQuat& ArnXformable::getAnimLocalXform_Rot() const
{
	return m_animLocalXform_Rot;
}

ArnAnimationController* ArnXformable::getAnimCtrl()
{
	return m_animCtrl;
}

void ArnXformable::setAnimCtrl(ArnAnimationController* animCtrl)
{
	m_animCtrl = animCtrl;
}

bool ArnXformable::isLocalXformDirty() const
{
	return m_bLocalXformDirty;
}

void ArnXformable::setAnimLocalXform_Rot( const ArnQuat& q )
{
	m_animLocalXform_Rot = q;
}

void ArnXformable::setAnimLocalXform_Scale( const ArnVec3& scale )
{
	m_animLocalXform_Scale = scale;
}

void ArnXformable::setAnimLocalXform_Trans( const ArnVec3& trans )
{
	m_animLocalXform_Trans = trans;
}
