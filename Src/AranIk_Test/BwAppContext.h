#pragma once
#include "BwWin32Timer.h"

enum ViewMode {
  VM_UNKNOWN,
  VM_TOP,			// KeyPad 7
  VM_RIGHT,			// KeyPad 3
  VM_BACK,			// KeyPad 1
  VM_BOTTOM,			// Shift + KeyPad 7
  VM_LEFT,			// Shift + KeyPad 3
  VM_FRONT,			// Shift + KeyPad 1
  VM_CAMERA
};

enum drawing_option {
  do_unused,
  do_grid, // should start from 1
  do_hud,
  do_joint,
  do_endeffector,
  do_joint_axis,
  do_contact,
  do_contact_force,
  do_root_node,
  do_wireframe,
  do_reference,
  do_fiber,
  
  do_count,
};

class	PlaybackSlider;
class BwOpenGlWindow;
typedef std::vector<std::vector<float> > Float2DArray;
class SliderInput;
class Fl_Light_Button;

class BwAppContext : private Uncopyable
{
 public:
  BwAppContext();
  ~BwAppContext();
  static const int			massMapResolution = 8;
  static const float			massMapDeviation;
  int					windowWidth;
  int					windowHeight;
  ArnViewportData			avd;
  std::vector<std::string>		sceneList;
  int					curSceneIndex;
  ArnSceneGraphPtr			sgPtr;
  SimWorldPtr				swPtr;
  ArnCamera*				activeCam;
  ArnLight*				activeLight;
  std::vector<ArnIkSolver*>		ikSolvers;
  GeneralBodyPtr			trunk;
  ArnPlane				contactCheckPlane;
  boost::circular_buffer<ArnVec3>	bipedComPos;	///< Store a trail of biped COM positions
  float					bipedMass;
  float					linVelX;
  float					linVelZ;
  float					torque;
  float					torqueAnkle;
  bool					bHoldingKeys[256];
  bool					bNextCamera;
  BwWin32Timer				timer;
  // Volatile: should be clear()-ed every frame.
  std::vector<ArnVec3>			isects;	///< Foot-ground intersection points
  std::vector<ArnVec3>			supportPolygon;	///< Support polygon
  std::vector<ArnVec3>			verticalLineIsects;
  Float2DArray				massMap;
  GLuint				massMapTex;
  unsigned char*			massMapData;
  ViewMode				viewMode;
  int					orthoViewDistance;
  bool					bPanningButtonDown;
  std::pair<int, int>			panningStartPoint;
  // Drawing options
  bool drawing_options[do_count];
  bool					bSimulate;
  int				frames; // Current frame
  float					panningCenter[3];
  float					dPanningCenter[3];
  Fl_Browser*				sceneGraphList;
  BwOpenGlWindow*				glWindow;
  Fl_Button*				frameLabel;
  PlaybackSlider*			playbackSlider;
  std::vector<SimWorldState>		simWorldHistory;
  Fl_Button *slider;
  SliderInput **cost_coeff_sliders;
  Fl_Light_Button *simulateButton;
  SliderInput *joint_dislocation_slider;
  Fl_Check_Button *joint_dislocation_button;
  Fl_Browser *fiber_browser;
  Fl_Check_Browser *rb_tracking_options;
  static const int MAX_SIMULATION_FRAMES = 10000;
  char frameStr[128];
};
