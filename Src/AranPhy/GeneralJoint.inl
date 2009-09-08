double GeneralJoint::getTargetCurrentDiff(int anum) const
{
	if (m_jcm == JCM_REST)
		return 0;
	else
		return getTargetValue(anum) - getValue(anum);
}
