#include "AranPhyPCH.h"
#include "GeneralBody.h"
#include "GeneralJoint.h"

GeneralJoint::GeneralJoint(const OdeSpaceContext* osc)
: m_osc(osc)
, m_jcm(JCM_REST)
, m_joint(0)
{
	//ctor
	m_name[0] = 0;

	// Must not call reset() here...
}

GeneralJoint::~GeneralJoint()
{
	//dtor
	dJointDestroy(m_joint);
}

void GeneralJoint::attach(GeneralBody& body1, GeneralBody& body2)
{
	dJointAttach(m_joint, body1.getBodyId(), body2.getBodyId());
}

void GeneralJoint::rest(int anum)
{
	m_jcm = JCM_REST;
}

void GeneralJoint::controlToStayStill_P(int anum, double fMax)
{
	m_jcm = JCM_STAY_STILL;
	m_forceMax[anum-1] = fMax;
}

void GeneralJoint::controlIncr_P(int anum, double incrVal, double proGain, double forceMax)
{
	m_jcm = JCM_INCREMENTAL_P;
	m_incrValue[anum-1] = incrVal;
	m_proGain[anum-1] = proGain;
	m_forceMax[anum-1] = forceMax;
}

void GeneralJoint::controlIncr_PD(int anum, double incrVal, double proGain, double derGain, double forceMax)
{
	m_jcm = JCM_INCREMENTAL_PD;
	m_incrValue[anum-1] = incrVal;
	m_proGain[anum-1] = proGain;
	m_derGain[anum-1] = derGain;
	m_forceMax[anum-1] = forceMax;
}

void GeneralJoint::control_P(int anum, double target, double gain, double fMax)
{
	m_jcm = JCM_TARGET_VALUE_P;
	m_targetValue[anum-1] = target;
	m_proGain[anum-1] = gain;
	m_forceMax[anum-1] = fMax;
}

void GeneralJoint::control_PD(int anum, double target, double proGain, double derGain, double forceMax)
{
	m_jcm = JCM_TARGET_VALUE_PD;
	m_targetValue[anum-1] = target;
	m_proGain[anum-1] = proGain;
	m_derGain[anum-1] = derGain;
	m_forceMax[anum-1] = forceMax;
}

void GeneralJoint::controlWithinRange_P(int anum, double min, double max, double pGain, double fMax)
{
	m_jcm = JCM_WITHIN_RANGE;
	m_minTargetValue[anum-1] = min;
	m_maxTargetValue[anum-1] = max;
	m_proGain[anum-1] = pGain;
	m_forceMax[anum-1] = fMax;
}

void GeneralJoint::setName(const char* name)
{
	m_name = name;
}
const char* GeneralJoint::getName() const
{
	return m_name.c_str();
}

void GeneralJoint::render(const BasicObjects& bo) const
{
}

void GeneralJoint::updateFrameInternal(int anum)
{
	switch (m_jcm)
	{
	case JCM_REST:
		{
			controlP(anum, 0, 1, 0);
		}
		break;
	case JCM_TARGET_VALUE_P:
		{
			controlP(anum, m_targetValue[anum-1], m_proGain[anum-1], m_forceMax[anum-1]);
		}
		break;
	case JCM_TARGET_VALUE_PD:
		{
			controlPD(anum, m_targetValue[anum-1], m_proGain[anum-1], m_derGain[anum-1], m_forceMax[anum-1]);
		}
		break;
	case JCM_STAY_STILL:
		{
			controlP(anum, getValue(anum), m_proGain[anum-1], m_forceMax[anum-1]);
		}
		break;
	case JCM_WITHIN_RANGE:
		{
			assert(m_minTargetValue[anum-1] <= m_maxTargetValue[anum-1]);
			dReal cur = getValue(anum);
			if (cur < m_minTargetValue[anum-1])
				controlP(anum, m_minTargetValue[anum-1] + 0.001, m_proGain[anum-1], m_forceMax[anum-1]);
			else if (cur > m_maxTargetValue[anum-1])
				controlP(anum, m_maxTargetValue[anum-1] - 0.001, m_proGain[anum-1], m_forceMax[anum-1]);
		}
		break;
	case JCM_INCREMENTAL_P:
		{
			setTargetValue(anum, getTargetValue(anum) + m_incrValue[anum-1]);
			controlP(anum, getTargetValue(anum), m_proGain[anum-1], m_forceMax[anum-1]);
		}
		break;
	case JCM_INCREMENTAL_PD:
		{
			setTargetValue(anum, getTargetValue(anum) + m_incrValue[anum-1]);
			controlPD(anum, getTargetValue(anum), m_proGain[anum-1], m_derGain[anum-1], m_forceMax[anum-1]);
		}
		break;
	default:
		{
			assert(!"Invalid joint control mode!");
		}
		break;
	}
}

void GeneralJoint::controlP(int anum, double targetValue, double proGain, double forceMax)
{
	dReal cur = getValue(anum);
	dReal z1 = getTargetValue(anum) - cur;

	setParamVelocity(anum, proGain * z1);
	setParamFMax(anum, forceMax);
}

void GeneralJoint::controlPD(int anum, double targetValue, double proGain, double derGain, double forceMax)
{
	dReal cur = getValue(anum);
	dReal curVel = getVelocity(anum);
	dReal z1 = getTargetValue(anum) - cur;

	setParamVelocity(anum, proGain * z1 + derGain * curVel);
	setParamFMax(anum, forceMax);
}

void GeneralJoint::reset()
{
	m_targetValue[0] = m_targetValue[1] = m_targetValue[2] = 0;
	m_incrValue[0] = m_incrValue[1] = m_incrValue[2] = 0;
	m_forceMax[0] = m_forceMax[1] = m_forceMax[2] = 0;
	m_proGain[0] = m_proGain[1] = m_proGain[2] = 0;
	m_derGain[0] = m_derGain[1] = m_derGain[2] = 0;
	m_minTargetValue[0] = m_minTargetValue[1] = m_minTargetValue[2] = 0;
	m_maxTargetValue[0] = m_maxTargetValue[1] = m_maxTargetValue[2] = 0;

	doReset();
}

void GeneralJoint::configureOdeContext( const OdeSpaceContext* osc )
{
}
