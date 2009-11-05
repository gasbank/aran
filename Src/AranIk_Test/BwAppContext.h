#pragma once
#include "BwWin32Timer.h"

enum ViewMode
{
	VM_UNKNOWN,
	VM_TOP,		// KeyPad 7
	VM_RIGHT,	// KeyPad 3
	VM_BACK,	// KeyPad 1
	VM_BOTTOM,	// Shift + KeyPad 7
	VM_LEFT,	// Shift + KeyPad 3
	VM_FRONT,	// Shift + KeyPad 1
	VM_CAMERA
};

class BwAppContext : private Uncopyable
{
public:
	BwAppContext();
	~BwAppContext();
	static const int						massMapResolution	= 8;
	static const float						massMapDeviation;
	int										windowWidth;
	int										windowHeight;
	ArnViewportData							avd;
	std::vector<std::string>				sceneList;
	int										curSceneIndex;
	ArnSceneGraphPtr						sgPtr;
	SimWorldPtr								swPtr;
	ArnCamera*								activeCam;
	ArnLight*								activeLight;
	std::vector<ArnIkSolver*>				ikSolvers;
	GeneralBodyPtr							trunk;
	ArnPlane								contactCheckPlane;
	boost::circular_buffer<ArnVec3>			bipedComPos;			///< Store a trail of biped COM positions
	float									bipedMass;
	float									linVelX;
	float									linVelZ;
	float									torque;
	float									torqueAnkle;
	bool									bHoldingKeys[256];
	bool									bNextCamera;
	BwWin32Timer							timer;

	// Volatile: should be clear()-ed every frame.
	std::vector<ArnVec3>					isects;					///< Foot-ground intersection points
	std::vector<ArnVec3>					supportPolygon;			///< Support polygon

	std::vector<ArnVec3>					verticalLineIsects;
	std::vector<std::vector<float> >		massMap;
	GLuint									massMapTex;
	unsigned char*							massMapData;

	ViewMode								viewMode;
	bool									bRenderHud;
	int										orthoViewDistance;
	bool									bPanningButtonDown;
	std::pair<int, int>						panningStartPoint;

	float									panningCenter[3];
	float									dPanningCenter[3];
};