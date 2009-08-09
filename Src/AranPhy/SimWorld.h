/**
\file SimWorld.h
\author Geoyeob Kim
\date 2009
*/
#ifndef SIMWORLD_H
#define SIMWORLD_H

class GeneralBody;
class BasicObjects;
class SliderJoint;
class Biped;
class GeneralJoint;
class StateController;
class ArnSceneGraph;
struct OdeSpaceContext;

TYPEDEF_SHARED_PTR(GeneralBody);
TYPEDEF_SHARED_PTR(GeneralJoint);

typedef std::vector<GeneralBody*>						GeneralBodyVector;
typedef std::set<GeneralBodyPtr>						GeneralBodyPtrSet;
typedef std::set<GeneralJointPtr>						GeneralJointPtrSet;
typedef std::vector<SliderJoint*>						SliderJointVector;

TYPEDEF_SHARED_PTR(SimWorld);

/**
\brief 독립된 하나의 물리 시뮬레이션 공간
*/
class ARANPHY_API SimWorld
{
public:
	/*!
	\brief		빈 물리 시뮬레이션 공간을 생성
	\return		성공하면 빈 SimWorld 인스턴스, 실패하면 \c NULL
	\remark		새 물리 시뮬레이션 공간을 생성하기 전에 물리 라이브러리가 초기화되어 있어야 합니다.
	\see		ArnInitializePhysics

	새로 생성된 시뮬레이션 공간에는 음의 Z축 방향으로 중력이 작용하고, Z=0인 넓이가 무한한 바닥이 존재합니다.
	*/
	static SimWorld*							createFromEmpty();
	/*!
	@brief		장면 그래프에서 물리 특성을 가진 노드를 찾아 시뮬레이션 공간을 생성
	@return		성공하면 SimWorld 인스턴스, 실패하면 \c NULL
	\remark		새 물리 시뮬레이션 공간을 생성하기 전에 물리 라이브러리가 초기화되어 있어야 합니다.
	\see		ArnInitializePhysics

	새로 생성된 시뮬레이션 공간에는 음의 Z축 방향으로 중력이 작용하고, Z=0인 넓이가 무한한 바닥이 존재합니다.
	*/
	static SimWorld*							createFrom(ArnSceneGraph* sg);
												~SimWorld();
	const OdeSpaceContext*						getOsc() const { return m_osc; }
	bool										registerBody(const GeneralBodyPtr& gbPtr);
	bool										registerJoint(const GeneralJointPtr& gjPtr);
	void										placeGround();
	GeneralBody*								placeBox(const char* name, const ArnVec3& com, const ArnVec3& size, float mass);
	void										placePiston(const char* name, const ArnVec3& com, const ArnVec3& size, float mass);
	void										placeSupport(Biped* biped);
	void										render() const;
	void										updateFrame(double elapsedTime);
	void										reset();
	const SliderJointVector&					getJoints() const { return m_sliderJoints; }
	SliderJoint*								getJointByName(const char* name) const;
	GeneralJointPtr								getGeneralJointByName(const char* name) const;
	GeneralBody*								getBodyByName(const char* name) const;
	const GeneralBodyPtr						getBodyByNameFromSet(const char* name) const;
	//GeneralBody*								getBodyByGeomID(dGeomID g) const;
	void										clearBodyContact();
	void										setBiped(Biped* biped) { m_biped = biped; }
	Biped*										getBiped() const { return m_biped; }
	double										getFootStep() const { return m_footStep; }
	void										setNextFootStep(double v) { m_nextFootStep = v; }
	double										getFootStepMaxHeight() const { return m_footStepMaxHeight; }
	void										setNextFootStepMaxHeight(double v) { m_nextFootStepMaxHeight = v; }
	void										setLeftSupportPosZ_Diff(double d) { m_leftSupportPosZ += d; if (m_leftSupportPosZ < 0) m_leftSupportPosZ = 0; printf("LeftSupportPosZ = %.3f\n", m_leftSupportPosZ); }
	void										setLeftSupportPosY_Diff(double d) { m_leftSupportPosY += d; printf("LeftSupportPosY = %.3f\n", m_leftSupportPosY); }
	void										setTrunkSupportPosZ_Diff(double d) { m_trunkSupportPosZ += d; }
	void										setTrunkSupportPosY_Diff(double d) { m_trunkSupportPosY += d; if (m_trunkSupportPosY < 0) m_trunkSupportPosY = 0;}
	void										setTrunkSupportPosY(double v) { m_trunkSupportPosY = v; if (m_trunkSupportPosY < 0) m_trunkSupportPosY = 0; }
	double										getFootSupportHeight() const { return m_footSupportHeight; }
	void										setRenderSupports(bool v) { m_bRenderSupports = v; }
	void										toggleRenderSupports() { m_bRenderSupports = !m_bRenderSupports; }

protected:
private:
												SimWorld();
	double										m_totalElapsedTime;
	OdeSpaceContext*							m_osc;
	GeneralBodyVector							m_bodies;
	GeneralBodyPtrSet							m_bodiesPtr;
	GeneralJointPtrSet							m_jointsPtr;
	SliderJointVector							m_sliderJoints;
	double										m_leftSupportPosZ;
	double										m_leftSupportPosY;
	double										m_trunkSupportPosZ;
	double										m_trunkSupportPosY;
	Biped*										m_biped;
	double										m_footStep;
	double										m_footStepMaxHeight;
	double										m_nextFootStep;
	double										m_nextFootStepMaxHeight;
	bool										m_bRenderSupports;
	double										m_footSupportHeight;
};

#endif // SIMWORLD_H
