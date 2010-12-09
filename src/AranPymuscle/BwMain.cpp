#include "BwPch.h"
#include "BwMain.h"
#include "BwTopWindow.h"
#include "BwOpenGlWindow.h"
#include "BwAppContext.h"
#include "BwWin32Timer.h"
#include "BwDrawingOptionsWindow.h"
#include "BwDebugPrintOptionsWindow.h"
#include "BwRbTrackingOptionsWindow.h"
#include "BwPlaybackSlider.h"
#include "ConvexHullCapi.h"
#include "QuaternionEOM.h"
#include "SliderInput.h"
#include "ArnPathManager.h"

namespace fs = boost::filesystem;

void idle_cb(void* ac);

#if !HAVE_GL
#error OpenGL in FLTK not enabled.
#endif

static int currentSimFrame = 0; // Current sim frame index (can be reset)

struct SceneButtonsHolder
{
  BwAppContext* ac;
  MessageHandleResult mhr;
};

struct cost_term {
  optimization_cost_terms e;
  std::string label;
  double lo, def, hi; /* lower, default, high bound */
};

static cost_term cost_terms[oct_count] = {
  { oct_normal_force, "normal", 0, 0, 10 },
  { oct_contact_point_zpos, "contact z", 0, 0, 10 },
  { oct_normal_force_nonneg_comp, "normal compen", 0, 10, 10 },
  { oct_contact_point_movement, "contact movement", 0, 5, 10},
  { oct_contact_point_zpos_epsilon, "contact z epsilon", 0, 0, 10},
  { oct_rb_reference_deviation, "reference dev.", 0, 10, 10},
  { oct_rb_previous_deviation, "previous dev.", 0, 0, 10},
  { oct_biped_com_deviation, "biped com dev.", 0, 0, 10},
  { oct_torque_around_com, "torque com dev.", 0, 0, 10},
  { oct_ligament_actuation, "lig act", 0, 0, 10},
  { oct_actuated_muscle_actuation, "act act", 0, 0, 10},
  { oct_joint_dislocation, "joint dislocation", 0, 0, 10},
  { oct_uniform_tension_cost, "tension", 0, 0, 10},
  { oct_uniform_actuation_cost, "actuation", 0, 0, 1}
};

void dump_cb(Fl_Widget *o, void *ac_raw)
{
  BwAppContext& ac = *(BwAppContext*)ac_raw;
  pym_config_t *pymCfg = &ac.pymRs->pymCfg;
  FILE *f = fopen("dump.dat", "wb");
  if (!f) {
    cout << "Warn - dump failed due to file opening failure.\n";
    return;
  }
  for (int i = 0; i <= ac.playbackSlider->getAvailableFrames(); ++i)
    fwrite(&ac.rb_history[i][0], sizeof(pym_rb_t), pymCfg->nBody, f);
  fclose(f);
  cout << "Dump file written.\n";
}

void print_rb0_cb(Fl_Widget *o, void *ac_raw)
{
  BwAppContext& ac = *(BwAppContext*)ac_raw;
  pym_config_t *pymCfg = &ac.pymRs->pymCfg;
  stringstream ss;
  pym_print_detailed_rb_state(ss, pymCfg->body[0]);
  string sss = ss.str();
  cout << sss << endl;
}

void load_cb(Fl_Widget *o, void *ac_raw)
{
  BwAppContext& ac = *(BwAppContext*)ac_raw;
  pym_config_t *pymCfg = &ac.pymRs->pymCfg;
  FILE *f = fopen("dump.dat", "rb");
  if (!f) {
    cout << "Error - load failed due to dump file opening.\n";
    return;
  }

  std::cout << "sizeof(intmax_t) is " << sizeof(boost::intmax_t) << '\n';

  fs::path p( "dump.dat" );

  if ( !fs::exists( p ) )
  {
    std::cout << "not found: dump.dat" << std::endl;
    return;
  }

  if ( !fs::is_regular( p ) )
  {
    std::cout << "not a regular file: dump.dat" << std::endl;
    return;
  }

  boost::uintmax_t file_s = fs::file_size( p );
  int nFrames = file_s/(pymCfg->nBody*sizeof(pym_rb_t));
  cout << "file size of dump.dat is " << file_s << " bytes." << endl;
  cout << "File contains " << nFrames << " frames of data." << endl;
  ac.rb_history.clear();
  for (int i = 0; i < nFrames; ++i) {
    vector<pym_rb_t> a(pymCfg->nBody);
    fread(&a[0], sizeof(pym_rb_t), pymCfg->nBody, f);
    ac.rb_history.push_back(a);
  }
  fclose(f);
  cout << "Loaded from dump file.\n";
  ac.playbackSlider->setAvailableFrames(nFrames-1); /* frame index range is 0..nFrames-1 */
  ac.playbackSlider->redraw();
}

static inline double
FootHeight(double t, double stepLength, double maxStepHeight)
{
  if (t <= 0 || t >= stepLength)
    return 0;
  else
    return -4.0*maxStepHeight*t*(t-stepLength)/(stepLength*stepLength);
}

static ArnSceneGraphPtr
ConfigureTestScene(const char* sceneFileName, const ArnViewportData* avd)
{
  ArnSceneGraphPtr ret(ArnSceneGraph::createFrom(sceneFileName));
  if (!ret)
    {
      fprintf(stderr, " *** Scene graph file %s is not loaded correctly.\n",
	      sceneFileName);
      fprintf(stderr, "     Check your input XML scene file.\n");
      return ArnSceneGraphPtr();
    }
  ret->interconnect(ret.get());
  std::cout << "   Scene file " << sceneFileName
	    << " loaded successfully." << std::endl;
  return ret;
}

static ArnSceneGraphPtr
ConfigureNextTestSceneWithRetry(int& curSceneIndex, int nextSceneIndex,
				const std::vector<std::string>& sceneList,
				const ArnViewportData& avd)
{
  assert(nextSceneIndex < (int)sceneList.size());
  curSceneIndex = nextSceneIndex;
  unsigned int retryCount = 0;
  ArnSceneGraphPtr ret;
  while (!ret) {
    ret = ConfigureTestScene(sceneList[curSceneIndex].c_str(), &avd);
    if (ret) {
      return ret;
    } else {
      curSceneIndex = (curSceneIndex + 1) % sceneList.size();
      ++retryCount;
    }
    if (retryCount >= sceneList.size()) {
      std::cerr << " *** All provided scene files have errors." << std::endl;
      return ArnSceneGraphPtr();
    }
  }
  return ret;
}

static ArnSceneGraphPtr
ReloadCurrentScene(int& curSceneIndex,
		   const std::vector<std::string>& sceneList,
		   const ArnViewportData& avd)
{
  return ConfigureNextTestSceneWithRetry(curSceneIndex, curSceneIndex,
					 sceneList, avd);
}

static void
Cleanup()
{
  ArnCleanupXmlParser();
  ArnCleanupImageLibrary();
  ArnCleanupPhysics();
  ArnCleanupGl();
}

static void
GetActiveCamAndLight(ArnCamera*& activeCam, ArnLight*& activeLight,
		     ArnSceneGraph* sg)
{
  activeCam = sg->getFirstCamera();
  assert(activeCam);
  activeCam->recalcLocalXform();
  activeCam->recalcAnimLocalXform();
  //activeCam->printCameraOrientation();
  activeLight = reinterpret_cast<ArnLight*>(sg->findFirstNodeOfType(NDT_RT_LIGHT));
  assert(activeLight);
}

static int
LoadSceneList(std::vector<std::string>& sceneList)
{
  // Load first scene file from SceneList.txt
  assert(sceneList.size() == 0);
  std::ifstream sceneListStream("SceneList.txt");
  std::string sceneFile;
  if (!sceneListStream.is_open()) {
    std::cout << " *** WARN: SceneList.txt file is not available.\n";
    return 0; // No scene loaded.
  }
  int sceneCount = 0;
  while (std::getline(sceneListStream, sceneFile)) {
    sceneList.push_back(sceneFile);
    ++sceneCount;
  }
  return sceneCount;
}

static void
UpdateScene(BwAppContext& ac, float fElapsedTime)
{
  // Physics simulation frequency (Hz)
  // higher --> accurate, stable, slow
  // lower  --> errors, unstable, fast
  static const unsigned int simFreq = 600;
  // Maximum simulation step iteration count for clamping
  // to keep app from advocating all resources to step further.
  static const unsigned int simMaxIteration = 100;

  unsigned int simLoop = (unsigned int)(fElapsedTime * simFreq);
  if (simLoop > simMaxIteration)
    simLoop = simMaxIteration;
  else if (simLoop == 0)
    simLoop = 2;
  simLoop = 2; // TODO: Simulation frequency
  for (unsigned int step = 0; step < simLoop; ++step) {
    //printf("frame duration: %d / simloop original: %d / current simstep %d\n", frameDurationMs, (unsigned int)(frameDurationMs / 1000.0 * simFreq), step);
    if (ac.swPtr)
      ac.swPtr->updateFrame(1.0 / simFreq);
  }
  if (ac.sgPtr) {
    ac.sgPtr->update(ac.timer.getTicks() / 1000.0, fElapsedTime);
  }
  foreach (ArnIkSolver* ikSolver, ac.ikSolvers) {
    ikSolver->update();

    Node* selNode = ikSolver->getSelectedEndeffector();
    if (selNode) {
      /*
	static const double d = 0.1;
	if (ac.bHoldingKeys[SDLK_UP])
	{
	selNode->setTargetDiff(0, 0, d);
	}
	if (ac.bHoldingKeys[SDLK_DOWN])
	{
	selNode->setTargetDiff(0, 0, -d);
	}
	if (ac.bHoldingKeys[SDLK_LEFT])
	{
	selNode->setTargetDiff(0, -d, 0);
	}
	if (ac.bHoldingKeys[SDLK_RIGHT])
	{
	selNode->setTargetDiff(0, d, 0);
	}
	if (ac.bHoldingKeys[SDLK_HOME])
	{
	selNode->setTargetDiff(d, 0, 0);
	}
	if (ac.bHoldingKeys[SDLK_END])
	{
	selNode->setTargetDiff(-d, 0, 0);
	}
      */
    }
  }

  if (ac.bNextCamera) {
    ac.activeCam = ac.sgPtr->getNextCamera(ac.activeCam);
    ac.bNextCamera = false;
  }

  ArnVec3 cameraDiff(0, 0, 0);
  static const float cameraDiffAmount = 0.1f;
  /*
    if (ac.bHoldingKeys[SDLK_a] && !ac.bHoldingKeys[SDLK_d])
    cameraDiff -= ac.activeCam->getRightVec() * cameraDiffAmount;
    else if (!ac.bHoldingKeys[SDLK_a] && ac.bHoldingKeys[SDLK_d])
    cameraDiff += ac.activeCam->getRightVec() * cameraDiffAmount;
    if (ac.bHoldingKeys[SDLK_KP_MINUS] && !ac.bHoldingKeys[SDLK_KP_PLUS])
    cameraDiff -= ac.activeCam->getUpVec() * cameraDiffAmount;
    else if (!ac.bHoldingKeys[SDLK_KP_MINUS] && ac.bHoldingKeys[SDLK_KP_PLUS])
    cameraDiff += ac.activeCam->getUpVec() * cameraDiffAmount;

    if (ac.activeCam->isOrtho())
    {
    float orthoScaleDiff = 0;
    if (ac.bHoldingKeys[SDLK_s] && !ac.bHoldingKeys[SDLK_w])
    orthoScaleDiff += cameraDiffAmount;
    else if (!ac.bHoldingKeys[SDLK_s] && ac.bHoldingKeys[SDLK_w])
    orthoScaleDiff -= cameraDiffAmount;

    ac.activeCam->setOrthoScale(ac.activeCam->getOrthoScale() + orthoScaleDiff);
    }
    else
    {
    if (ac.bHoldingKeys[SDLK_s] && !ac.bHoldingKeys[SDLK_w])
    cameraDiff -= ac.activeCam->getLookVec() * cameraDiffAmount;
    else if (!ac.bHoldingKeys[SDLK_s] && ac.bHoldingKeys[SDLK_w])
    cameraDiff += ac.activeCam->getLookVec() * cameraDiffAmount;
    }
  */
  if (ac.activeCam) {
    ac.activeCam->setLocalXform_Trans( ac.activeCam->getLocalXform_Trans() + cameraDiff );
    ac.activeCam->recalcLocalXform();
  }
  ac.isects.clear();
  ac.verticalLineIsects.clear();
  ac.supportPolygon.clear();
  bool useGroundPlaneBodyIntersection = true;
  if (ac.trunk) {
    // Update COM pos
    ArnVec3 bipedComPos;
    ac.trunk->calculateLumpedComAndMass(&bipedComPos, &ac.bipedMass);
    ac.bipedComPos.push_back(bipedComPos); // Save the trail of COM for visualization

    if (useGroundPlaneBodyIntersection) {
      // Needed for support polygon visualization
      //ac.trunk->calculateLumpedGroundIntersection(ac.isects);
      ac.trunk->calculateLumpedIntersection(ac.isects, ac.contactCheckPlane);
    } else {
      const unsigned int swContactCount = ac.swPtr->getContactCount();
      for (unsigned int i = 0; i < swContactCount; ++i) {
	ArnVec3 contactPos;
	ac.swPtr->getContactPosition(i, &contactPos);
	ac.isects.push_back(contactPos);
      }
    }

    /*
    // Projected mass distribution map texture creation
    ac.trunk->calculateLumpedIntersection(ac.isects, ac.contactCheckPlane);
    for (int mm = 0; mm < AppContext::massMapResolution*2; ++mm)
    std::fill(ac.massMap[mm].begin(), ac.massMap[mm].end(), 0.0f);
    float maxMassMapVal = ac.trunk->calculateLumpedVerticalIntersection(ac.verticalLineIsects, ac.massMap, bipedComPos.x, bipedComPos.y, AppContext::massMapDeviation, AppContext::massMapResolution);

    // Prepare texture
    for (int i = -AppContext::massMapResolution; i < AppContext::massMapResolution; ++i)
    {
    for (int j = -AppContext::massMapResolution; j < AppContext::massMapResolution; ++j)
    {
    int offset = AppContext::massMapResolution*2*(i+AppContext::massMapResolution) + j+AppContext::massMapResolution;

    ac.massMapData[4*offset + 0] = 0;
    ac.massMapData[4*offset + 1] = 0;
    ac.massMapData[4*offset + 2] = 255;
    ac.massMapData[4*offset + 3] = (unsigned char)(ac.massMap[j+AppContext::massMapResolution][i+AppContext::massMapResolution] / maxMassMapVal * 255);
    }
    }
    glBindTexture(GL_TEXTURE_2D, ac.massMapTex);
    //glTexImage2D(GL_TEXTURE_2D, 0, 4, AppContext::massMapResolution*2, AppContext::massMapResolution*2, 0, GL_RGBA, GL_UNSIGNED_BYTE, ac.massMapData);
    gluBuild2DMipmaps( GL_TEXTURE_2D, 4, AppContext::massMapResolution*2, AppContext::massMapResolution*2, GL_RGBA, GL_UNSIGNED_BYTE, ac.massMapData );
    glBindTexture(GL_TEXTURE_2D, 0);
    */
  }

  if (ac.isects.size()) {
    //PYMCORE_API int PymConvexHull(Point_C *P, int n, Point_C *H);

    std::vector<Point_C> isectsCgal(ac.isects.size());
    int a = 0;
    foreach (const ArnVec3& contactPos, ac.isects) {
      isectsCgal[a].x = contactPos.x;
      isectsCgal[a].y = contactPos.y;
      ++a;
    }
    std::vector<Point_C> out(isectsCgal.size() + 1);
    int outEnd;
    outEnd = PymConvexHull(&isectsCgal[0], isectsCgal.size(), &out[0]);

    for (int i = 0; i < outEnd; ++i) {
      ac.supportPolygon.push_back(ArnVec3(out[i].x, out[i].y, 0));
    }
  }
}

static void
AddToSceneGraphList( const ArnNode* node, Fl_Browser* list, int depth )
{
  std::string itemName("@s@f");
  for (int i = 0; i < depth; ++i)
    itemName += "  ";
  if (strlen(node->getName()))
    itemName += node->getName();
  else
    itemName += "<unnamed>";
  list->add(itemName.c_str());

  foreach (const ArnNode* child, node->getChildren()) {
    AddToSceneGraphList(child, list, depth+1);
  }
}

static void
UpdateSceneGraphList( BwAppContext& ac )
{
  if (ac.sceneGraphList) {
    ac.sceneGraphList->clear();
    if (ac.sgPtr)
      AddToSceneGraphList(ac.sgPtr->getSceneRoot(), ac.sceneGraphList, 0);
  }
}

/*!
 * @brief Scene graph가 새로 로드되었을 때 수행되는 초기화 (렌더러와 무관)
 */
static int
InitializeRendererIndependentsFromSg(BwAppContext& ac)
{
  ac.frames = 0;
  assert(ac.sgPtr);
  ac.swPtr.reset(SimWorld::createFrom(ac.sgPtr.get()));
  GetActiveCamAndLight(ac.activeCam, ac.activeLight, ac.sgPtr.get());
  foreach (ArnIkSolver* ikSolver, ac.ikSolvers) {
    delete ikSolver;
  }
  ac.ikSolvers.clear();
  ac.bipedComPos.clear();
  if (ac.swPtr) {
    ac.trunk = ac.swPtr->getBodyByNameFromSet("Trunk");

    // Create ArnSkeleton from rigid body links!
    if (ac.trunk) {
      ac.bipedComPos.clear();
      ArnVec3 comPos;
      ac.trunk->calculateLumpedComAndMass(&comPos, &ac.bipedMass);
      ac.bipedComPos.push_back(comPos);
      std::cout << " - Biped total mass: " << ac.bipedMass << std::endl;

      ArnSkeleton* trunkSkel = ac.trunk->createLumpedArnSkeleton(ac.swPtr);
      trunkSkel->setName("Autogenerated Skeleton");
      ac.sgPtr->attachChildToFront(trunkSkel);
    }
  }
  // TODO: Attaching ArnIkSolver onto specific skeletons makes crash for some cases
  //ArnCreateArnIkSolversOnSceneGraph(ac.ikSolvers, ac.sgPtr);
  UpdateSceneGraphList(ac);
  return 0;
}

/*!
 * @brief Scene graph가 새로 로드되었을 때 수행되는 초기화 (렌더러 종속)
 */
static int
InitializeRendererDependentsFromSg(BwAppContext& ac)
{
  if (ac.sgPtr.get())
    ArnInitializeRenderableObjectsGl(ac.sgPtr.get());
  return 0;
}

static int
InitializeRendererDependentOnce(BwAppContext& ac)
{
  // OpenGL context available from this line.
  if (!glGetString( GL_VENDOR ) || !glGetString( GL_RENDERER ) || !glGetString( GL_VERSION )) {
    printf("  OpenGL context is not availble yet. Aborting...\n");
    return -1234;
  }

  printf( "Vendor     : %s\n", glGetString( GL_VENDOR ) );
  printf( "Renderer   : %s\n", glGetString( GL_RENDERER ) );
  printf( "Version    : %s\n", glGetString( GL_VERSION ) );
  //printf( "Extensions : %s\n", glGetString( GL_EXTENSIONS ) );
  printf("\n");

  /// OpenGL 플래그를 설정합니다.
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  // Enables Depth Testing
  glShadeModel(GL_SMOOTH);
  // Enable Smooth Shading
  //glShadeModel(GL_FLAT);
  // Enable Smooth Shading
  glClearDepth(1.0f);
  // Depth Buffer Setup
  glDepthFunc(GL_LEQUAL);
  // The Type Of Depth Testing To Do
  // Really Nice Perspective Calculations
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
  glEnable(GL_NORMALIZE);
  for (int lightId = 0; lightId < GL_MAX_LIGHTS; ++lightId) {
    glDisable(GL_LIGHT0 + lightId);
  }

  /// OpenGL 확장 기능을 초기화합니다.
  if (ArnInitGlExtFunctions() < 0) {
    std::cerr << " *** OpenGL extensions needed to run this program are not available." << std::endl;
    std::cerr << "     Check whether you are in the remote control display or have a legacy graphics adapter." << std::endl;
    std::cerr << "     Aborting..." << std::endl;
    return -50;
  }

  /// ARAN OpenGL 패키지를 초기화합니다.
  if (ArnInitializeGl() < 0) {
    return -3;
  }

  /// Mass map 텍스처를 생성합니다.
  glGenTextures(1, &ac.massMapTex);
  glBindTexture( GL_TEXTURE_2D, ac.massMapTex );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glBindTexture(GL_TEXTURE_2D, 0);

  if (InitializeRendererDependentsFromSg(ac) < 0) {
    return -2;
  }

  return 0;
}

int
InitializeRendererIndependentOnce(BwAppContext& ac)
{
  /*!
   * 렌더러 독립적 ARAN 패키지인 ARAN Core, ARAN Physics를 초기화합니다.
   * 초기화가 성공한 이후 프로그램의 치명적인 오류로 인해 실행이 중단될 경우
   * 반드시 Cleanup() 을 호출해야 합니다.
   * 본 초기화가 실패할 경우에는 프로그램이 종료됩니다.
   */
  if (ArnInitializeXmlParser() < 0) {
    Cleanup();
    return -1;
  }
  if (ArnInitializeImageLibrary() < 0) {
    Cleanup();
    return -2;
  }
  if (ArnInitializePhysics() < 0) {
    Cleanup();
    return -3;
  }
  // Notice for 32-bit and 64-bit build: Do not confuse the size of pointers!
  std::cout << " INFO  Raw pointer    size = " << sizeof(ArnSceneGraph*) << std::endl;
  std::cout << " INFO  Shared pointer size = " << sizeof(ArnSceneGraphPtr) << std::endl;
  /// \c SceneList.txt 를 파싱합니다.
  /*if (LoadSceneList(ac.sceneList) < 0) {
    std::cerr << " *** Init failed..." << std::endl;
    return -10;
  }*/
  memset(ac.bHoldingKeys, 0, sizeof(ac.bHoldingKeys));
  // Default viewport init
  ac.avd.X			 = 0;
  ac.avd.Y			 = 0;
  ac.avd.MinZ			 = 0;
  ac.avd.MaxZ			 = 1.0f;
  // View mode
  ac.viewMode			 = VM_UNKNOWN;
  ac.orthoViewDistance		 = 5;
  // Panning by dragging
  ac.bPanningButtonDown		 = false;
  ac.panningCenter[0]		 = 0;
  ac.panningCenter[1]		 = 0;
  ac.panningCenter[2]		 = 0;
  ac.dPanningCenter[0]		 = 0;
  ac.dPanningCenter[1]		 = 0;
  ac.dPanningCenter[2]		 = 0;
  // Drawing options
  for (int i = 0; i < pym_do_count; ++i)
    ac.drawing_options[i] = false;
  // Scene graph UI
  ac.sceneGraphList		 = 0;
  // Rigid body simulation
  ac.bSimulate			 = false;
  /// 다음 카메라로 변경 플래그 초기화
  ac.bNextCamera		 = false;
  // Timer init
  ac.timer.start();
  // SimWorld history
  ac.simWorldHistory.resize(10000);

  ac.contactCheckPlane.setV0(ArnVec3(0, 0, 0.01f));
  ac.contactCheckPlane.setNormal(ArnVec3(0, 0, 1.0f));

  /// (있다면) 첫 장면 파일을 메모리에 로드합니다.
  ac.curSceneIndex = -1;
  if (ac.sceneList.size() > 0) {
    ac.sgPtr = ConfigureNextTestSceneWithRetry(ac.curSceneIndex, 0, ac.sceneList, ac.avd);
    if (!ac.sgPtr) {
      std::cerr << " *** Scene graph loading failed..." << std::endl;
      Cleanup();
      return -20;
    }
    assert(ac.sgPtr);

    /// 처음으로 로드한 모델 파일에 종속적인 데이터를 초기화합니다.
    if (InitializeRendererIndependentsFromSg(ac) < 0) {
      Cleanup();
      return -1;
    }
  } else {
    std::cout << "WARN: No scene file listed on SceneList.txt\n";
  }
  return 0;
}

// when you change the data, as in this callback, you must call redraw():
void sides_cb(Fl_Widget *o, void *p)
{
  BwOpenGlWindow *sw = (BwOpenGlWindow *)p;
  //sw->sides = int(((Fl_Slider *)o)->value());
  sw->redraw();
}

void overlay_sides_cb(Fl_Widget *o, void *p)
{
  BwOpenGlWindow *sw = (BwOpenGlWindow *)p;
  //sw->overlay_sides = int(((Fl_Slider *)o)->value());
  sw->redraw_overlay();
}

void update_idle_cb_attachment(BwAppContext &ac)
{
  ac.bSimulate = ac.simulateButton->value() ? true : false;
  if (ac.bSimulate) {
    if (!Fl::has_idle(idle_cb, &ac))
      Fl::add_idle(idle_cb, &ac);
  } else {
    if (Fl::has_idle(idle_cb, &ac))
      Fl::remove_idle(idle_cb, &ac);
  }
}

void step(BwAppContext &ac)
{
  static double start_time		= 0;
  static double frameStartMs		= 0;
  static double frameDurationMs	= 0;
  static double frameEndMs		= 0;
  static int simFrame = 0; // Global sim frame index (no reset)
  
  pym_config_t *pymCfg = &ac.pymRs->pymCfg;
  const int nb = pymCfg->nBody;
  if (!start_time)
    start_time = ac.timer.getTicks();
  frameDurationMs = frameEndMs - frameStartMs;
  //printf("Total %lf FrameDuration %lf\n", frameStartMs - start_time, frameDurationMs);

  frameStartMs = ac.timer.getTicks();
  UpdateScene(ac, (float)(frameDurationMs / 1000.0));

  frameEndMs = ac.timer.getTicks();
  ac.glWindow->redraw();
  
  
  if (ac.swPtr)
    ac.swPtr->getSimWorldState(ac.simWorldHistory[ac.frames]);

  std::stringstream ss;
  ss << setiosflags(ios::fixed) << setprecision(4);

  int frame_move_ret = 0;
  char result_msg[2048] = {0,};
  if (ac.pymRs) {
    //cout << pymCfg->body[0] << endl;
    //printf("FRAME %d\n", simFrame);
    pymCfg->curFrame = simFrame;
    for (int i = 0; i < oct_count; ++i) {
      const double cv = ac.cost_coeff_sliders[ i ]->value();
      pymCfg->opt_cost_coeffs[ cost_terms[i].e ] = cv;
    }
    for (int i = 0; i < nb; ++i) {
      // fltk check browser item index starts at 1.
      pymCfg->body[i].b.track = ac.rb_tracking_options->checked(i+1) == 1 ? true : false;
    }
    pymCfg->joint_dislocation_threshold = ac.joint_dislocation_slider->value();
    pymCfg->joint_dislocation_enabled = ac.joint_dislocation_button->value();
    frame_move_ret = PymRsFrameMove(ac.pymRs, currentSimFrame, result_msg);
    if (frame_move_ret < 0) {
      ac.simulateButton->value(0);
      update_idle_cb_attachment(ac);
    }
    const int nb = ac.pymRs->phyCon.pymCfg->nBody;
    double totConForce[3] = {0,};
    double COM[3] = {0,};
    double totMass = 0;
    int i, j, k;
    FOR_0(i, nb) {
      /* Access data from renderer-accessible area of phyCon */
      const pym_rb_named_t *rbn = &pymCfg->body[i].b;
      FOR_0(j, rbn->nContacts_1) {
        const double *conForce =rbn->contactsForce_2[j];
        FOR_0(k, 3) {
          totConForce[k] += conForce[k];
          COM[k] += rbn->q[k]*rbn->m;
        }
      }
      FOR_0(k, 3) {
        COM[k] += rbn->q[k]*rbn->m;
      }
      totMass += rbn->m;
    }
    const int nf = pymCfg->nFiber;
    double totActAct = 0, totActTen = 0;
    double totLigAct = 0, totLigTen = 0;
    FOR_0(j, nf) {
      pym_mf_named_t *mf = &pymCfg->fiber[j].b;
      if (mf->mType == PMT_ACTUATED_MUSCLE) {
        totActAct += mf->A;
        totActTen += fabs(mf->T);
      } else {
        totLigAct += mf->A;
        totLigTen += fabs(mf->T);
      }
    }
    //printf("totAct  Act %lf  Ten %lf\n", totActAct, totActTen);
    //printf("totLig  Act %lf  Ten %lf\n", totLigAct, totLigTen);
    //printf("totConForce = %lf, %lf, %lf\n", totConForce[0], totConForce[1], totConForce[2]);
    ss << result_msg << endl;
    ss << "Cube omega: " << ac.glWindow->omega() << endl;
    ss << "totConForce: [" << totConForce[0] << ", " << totConForce[1] << ", " << totConForce[2] << "]\n";

    FOR_0(k, 3) {
      COM[k] /= totMass;
    }
    static vector<string> fiber_str_items;
    ac.fiber_browser->clear();
    for (int i = 0; i < pymCfg->nFiber; ++i) {
      pym_mf_named_t *mfi = &pymCfg->fiber[i].b;
      std::stringstream str_s;
      str_s << setiosflags(ios::fixed) << setprecision(6);
      str_s << mfi->name << "\t" << mfi->A << "\t" << mfi->T << "\t" << mfi->kse << "\t"
        << mfi->kpe << "\t" << mfi->b << "\t" << mfi->xrest;
      ac.fiber_browser->add(str_s.str().c_str());
    }

    BwOpenGlWindow *gw = dynamic_cast<BwOpenGlWindow *>(ac.glWindow);
    ++simFrame;
    ++currentSimFrame;
    if (frame_move_ret || simFrame >= pymCfg->nSimFrame) {
      //PymRsResetPhysics(appContext.pymRs);
      simFrame = 0;
    }
  }
  std::string sss(ss.str());
  static char slidersss[1024];
  strncpy(slidersss, sss.c_str(), 1024);
  ac.slider->label(slidersss);

  if (ac.pymRs && !frame_move_ret) {
    if (ac.frames < BwAppContext::MAX_SIMULATION_FRAMES) {
      ++ac.frames;
      sprintf(ac.frameStr, "%d", ac.frames);
      ac.frameLabel->redraw_label();

      ac.playbackSlider->setAvailableFrames(ac.frames);
      ac.playbackSlider->value(ac.frames);
    }

    static vector<pym_rb_t> body_states(pymCfg->nBody);
    memcpy(&body_states[0], pymCfg->body, sizeof(pym_rb_t)*pymCfg->nBody);
    ac.rb_history.erase_end( ac.rb_history.size() - ac.frames );
    ac.rb_history.push_back(body_states);
    //printf("rb_history.size() = %d\n", ac.rb_history.size());
  }
}
static const int aaa = 100;
char bbb[aaa];
void idle_cb(void* ac_raw)
{
  BwAppContext& ac = *(BwAppContext*)ac_raw;
  if (ac.bSimulate) {
    step(ac);
  }
}

void simulate_button_cb(Fl_Widget *o, void *p)
{
  Fl_Light_Button* widget = (Fl_Light_Button*)o;
  BwAppContext& ac = *(BwAppContext*)p;
  update_idle_cb_attachment(ac);
}

void real_muscle_cb(Fl_Widget *o, void *p)
{
  Fl_Light_Button* widget = (Fl_Light_Button*)o;
  BwAppContext& ac = *(BwAppContext*)p;
  ac.pymRs->pymCfg.real_muscle = widget->value() == 1 ? true : false;
}

void set_vel_zero_cb(Fl_Widget *o, void *p)
{
  Fl_Light_Button* widget = (Fl_Light_Button*)o;
  BwAppContext& ac = *(BwAppContext*)p;
  //ac.pymRs->pymCfg.real_muscle = widget->value() == 1 ? true : false;
  for (int i = 0; i < ac.pymRs->pymCfg.nBody; ++i) {
    ac.pymRs->pymCfg.body[i].b.pd[0] = 0;
    ac.pymRs->pymCfg.body[i].b.pd[1] = 0;
    ac.pymRs->pymCfg.body[i].b.pd[2] = 0;
    ac.pymRs->pymCfg.body[i].b.p0[0] = ac.pymRs->pymCfg.body[i].b.p[0];
    ac.pymRs->pymCfg.body[i].b.p0[1] = ac.pymRs->pymCfg.body[i].b.p[1];
    ac.pymRs->pymCfg.body[i].b.p0[2] = ac.pymRs->pymCfg.body[i].b.p[2];

    ac.pymRs->pymCfg.body[i].b.qd[0] = 0;
    ac.pymRs->pymCfg.body[i].b.qd[1] = 0;
    ac.pymRs->pymCfg.body[i].b.qd[2] = 0;
    ac.pymRs->pymCfg.body[i].b.qd[3] = 0;
    ac.pymRs->pymCfg.body[i].b.q0[0] = ac.pymRs->pymCfg.body[i].b.q[0];
    ac.pymRs->pymCfg.body[i].b.q0[1] = ac.pymRs->pymCfg.body[i].b.q[1];
    ac.pymRs->pymCfg.body[i].b.q0[2] = ac.pymRs->pymCfg.body[i].b.q[2];
    ac.pymRs->pymCfg.body[i].b.q0[3] = ac.pymRs->pymCfg.body[i].b.q[3];
  }
}

void scene_buttons_cb(Fl_Widget* o, void* p)
{
  SceneButtonsHolder* sbh = (SceneButtonsHolder*)p;
  BwAppContext& ac = *sbh->ac;
  BwOpenGlWindow& openGlWindow = *sbh->ac->glWindow;
  MessageHandleResult done = sbh->mhr;

  int reconfigScene = false;
  if (ac.sceneList.size() && (done == MHR_NEXT_SCENE || done == MHR_PREV_SCENE)) {
    int nextSceneIndex;
    if (done == MHR_NEXT_SCENE) {
      nextSceneIndex = (ac.curSceneIndex + 1) % ac.sceneList.size();
    } else {
      if (ac.curSceneIndex == 0)
        nextSceneIndex = ac.sceneList.size() - 1;
      else
        nextSceneIndex = ac.curSceneIndex - 1;
    }
    ac.sgPtr = ConfigureNextTestSceneWithRetry(ac.curSceneIndex, nextSceneIndex, ac.sceneList, ac.avd);
    if (!ac.sgPtr)	{
      std::cerr << " *** Aborting..." << std::endl;
      done = MHR_EXIT_APP;
    } else {
      reconfigScene = true;
    }
  } else if (done == MHR_RELOAD_SCENE) {
    if (ac.sceneList.size()) {
      ac.sgPtr = ReloadCurrentScene(ac.curSceneIndex, ac.sceneList, ac.avd);
      if (!ac.sgPtr) {
        std::cerr << " *** Aborting..." << std::endl;
        done = MHR_EXIT_APP;
      }
    }
    PymRsResetPhysics(ac.pymRs);
    currentSimFrame = 0;
    reconfigScene = true;
  } else if (done == MHR_STEP_SIMULATION) {
    step(ac);
  }

  if (reconfigScene) {
    if (ac.sceneList.size()) {
      InitializeRendererIndependentsFromSg(ac);
      InitializeRendererDependentsFromSg(ac);
    }
    openGlWindow.redraw();
  }
}

int doMain(int argc, char **argv)
{
  int ret = 0;
  BwAppContext ac;
  if (InitializeRendererIndependentOnce(ac) < 0) {
    printf("Critical error during initialization of "
	   "application context. Aborting...\n");
    abort();
  }
  ac.pymRs = PymRsInitContext(argc, argv);
  BwTopWindow topWindow(800, 600, "aran", ac);

  std::ifstream windowPosAndSizeInput("BwWindow.txt");
  if (windowPosAndSizeInput.is_open()) {
    int x, y, w, h;
    windowPosAndSizeInput >> x >> y >> w >> h;
    topWindow.resize(x, y, w, h);
  }

  BwOpenGlWindow openGlWindow(210, 75,
			      topWindow.w()-20-400, topWindow.h()-290,
			      0, ac);
  //openGlWindow.mode(FL_RGB | FL_DOUBLE | FL_DEPTH);
  topWindow.setShapeWindow(&openGlWindow);
  //sw.mode(FL_RGB);
  topWindow.resizable(&openGlWindow);

  SceneButtonsHolder sbh[4] = {
    {&ac, MHR_RELOAD_SCENE},
    {&ac, MHR_PREV_SCENE},
    {&ac, MHR_NEXT_SCENE},
    {&ac, MHR_STEP_SIMULATION},
  };
  Fl_Button reloadSceneButton(10, 5, 70, 30, "Reload");
  reloadSceneButton.callback(scene_buttons_cb, &sbh[0]);
  Fl_Button nextSceneButton(10+75, 5, 50, 30, "Prev");
  nextSceneButton.callback(scene_buttons_cb, &sbh[1]);
  Fl_Button prevSceneButton(10+75+55, 5, 50, 30, "Next");
  prevSceneButton.callback(scene_buttons_cb, &sbh[2]);
  Fl_Light_Button simulateButton(10, 40, 100, 30, "@> Simulate");
  simulateButton.callback(simulate_button_cb, &ac);
  ac.simulateButton = &simulateButton;
  Fl_Button stepButton(115, 40, 40, 30, "Step");
  stepButton.callback(scene_buttons_cb, &sbh[3]);
  Fl_Button frameLabel(10+100+60, 40, 100, 30, "Frame");
  frameLabel.box(FL_NO_BOX);
  frameLabel.align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
  frameLabel.label(ac.frameStr);
  ac.frameLabel = &frameLabel;
  Fl_Button load(300, 5, 100, 30, "Load");
  load.callback(load_cb, &ac);
  Fl_Button dump(415, 5, 100, 30, "Dump");
  dump.callback(dump_cb, &ac);
  
  Fl_Hor_Slider slider(510, 5, topWindow.w()-520, 30, "Sides");
  slider.align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
  slider.callback(sides_cb, &openGlWindow);
  slider.step(1);
  slider.bounds(3,40);
  PlaybackSlider playbackSlider(300, 40, topWindow.w()-310, 30, 0, ac);
  ac.playbackSlider = &playbackSlider;
  playbackSlider.bounds(0, ac.MAX_SIMULATION_FRAMES-1); // Maximum 10000 frames can be simulated.
  playbackSlider.align(FL_ALIGN_LEFT);
  playbackSlider.step(1);
  BwDrawingOptionsWindow drawingOptions(topWindow.w()-200, 75,
					190, 200, "Drawings", ac, openGlWindow);
  BwDebugPrintOptionsWindow debugMsgOptions(topWindow.w()-200, 300,
    190, 200, "Debug messages", ac);
  BwRbTrackingOptionsWindow rbTrackingOptions(topWindow.w()-200, 520,
    190, 200, "Rigid body tracking", ac);
  ac.rb_tracking_options = &rbTrackingOptions;
  Fl_Button print_rb0_btn(topWindow.w()-200, 740, 190, 25, "Print RB[0] State");
  print_rb0_btn.callback(print_rb0_cb, &ac);
  
  Fl_Browser sceneList(10000+topWindow.w()-200, 75+110+100, 190, 100);
  Fl_Browser sceneGraphList(10000+topWindow.w()-200, 75+110+110+100,
			    190, topWindow.h()-90-110-110-200);
  ac.sceneGraphList = &sceneGraphList;
  ac.glWindow = &openGlWindow;
  Fl_Button omega_label(0, 75+110+110 + (topWindow.h()-90-110-110-200), 600, 300, "text info");
  omega_label.box(FL_NO_BOX);
  omega_label.align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_TOP);
  ac.slider = &omega_label;

  
  Fl_Button *cost_labels[oct_count];
  //Fl_Value_Slider *cost_coefficients[oct_count];
  SliderInput *cost_coeff_sliders[oct_count];
  for (int i = 0; i < oct_count; ++i) {
    assert(cost_terms[i].lo <= cost_terms[i].def && cost_terms[i].def <= cost_terms[i].hi);
    cost_labels[i] = new Fl_Button(0, 75+40*i, 200, 20, cost_terms[i].label.c_str());
    cost_labels[i]->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    cost_labels[i]->labelsize(12);
    cost_labels[i]->box(FL_NO_BOX);

    cost_coeff_sliders[i] = new SliderInput(0, 75+40*i+20, 200, 20);
    cost_coeff_sliders[i]->type(FL_HOR_SLIDER);
    cost_coeff_sliders[i]->bounds(cost_terms[i].lo, cost_terms[i].hi);
    cost_coeff_sliders[i]->value(cost_terms[i].def);
  }
  ac.cost_coeff_sliders = cost_coeff_sliders;

  Fl_Check_Button joint_dislocation_button(0, 75+20+40*oct_count, 200, 20, "Joint dislocation");
  joint_dislocation_button.align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
  joint_dislocation_button.labelsize(12);

  SliderInput joint_dislocation_si = SliderInput(0, 75+20+40*oct_count+20, 200, 20);
  joint_dislocation_si.type(FL_HOR_SLIDER);
  joint_dislocation_si.bounds(0.0, 1.0);
  joint_dislocation_si.value(0.05);
  ac.joint_dislocation_slider = &joint_dislocation_si;
  ac.joint_dislocation_button = &joint_dislocation_button;

  Fl_Light_Button real_muscle(0, 75+20+40*oct_count+20+100, 100, 30, "Real muscle");
  real_muscle.callback(real_muscle_cb, &ac);

  Fl_Button set_vel_zero(0, 75+20+40*oct_count+20+100+40, 100, 30, "Set vel 0");
  set_vel_zero.callback(set_vel_zero_cb, &ac);

  // Style table
  Fl_Text_Display::Style_Table_Entry stable[] = {
    // FONT COLOR      FONT FACE   FONT SIZE
    // --------------- ----------- --------------
    {  FL_RED,         FL_COURIER, 10 }, // A - Red
    {  FL_DARK_YELLOW, FL_COURIER, 10 }, // B - Yellow
    {  FL_DARK_GREEN,  FL_COURIER, 10 }, // C - Green
    {  FL_BLUE,        FL_COURIER, 10 }, // D - Blue
  };
  Fl_Text_Display *disp = new Fl_Text_Display(0, 75+20+40*oct_count+20+100+40+40, 200, 200, "Display");
  Fl_Text_Buffer *tbuff = new Fl_Text_Buffer();      // text buffer
  Fl_Text_Buffer *sbuff = new Fl_Text_Buffer();      // style buffer
  disp->buffer(tbuff);
  int stable_size = sizeof(stable)/sizeof(stable[0]);
  disp->highlight_data(sbuff, stable, stable_size, 'A', 0, 0);
  // Text
  tbuff->text("Red Line 1\nYel Line 2\nGrn Line 3\nBlu Line 4\n"
    "Red Line 5\nYel Line 6\nGrn Line 7\nBlu Line 8\n");
  // Style for text
  sbuff->text("AAAAAAAAAA\nBBBBBBBBBB\nCCCCCCCCCC\nDDDDDDDDDD\n"
    "AAAAAAAAAA\nBBBBBBBBBB\nCCCCCCCCCC\nDDDDDDDDDD\n");

  Fl_Browser *b = new Fl_Browser(500,75+110+110 + (topWindow.h()-90-110-110-200),500,200);
  int widths[] = { 100, 100, 100, 70, 70, 40, 40, 70, 70, 50, 0 };               // widths for each column
  b->textsize(9);
  b->column_widths(widths);
  b->column_char('\t');                                                       // tabs as column delimiters
  b->type(FL_MULTI_BROWSER);
  ac.fiber_browser = b;

  

  topWindow.setSceneList(&sceneList);
  topWindow.end();
  //topWindow.show(argc,argv);
  topWindow.show();
  openGlWindow.show();
  openGlWindow.make_current();
  openGlWindow.redraw_overlay();
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    printf("Error: glewInit() failed\n");
    return 1;
  }
  InitializeRendererDependentOnce(ac);
  UpdateSceneGraphList(ac);
  // scene list available now
  foreach (const std::string& scene, ac.sceneList) {
    sceneList.add(scene.c_str());
  }

  if (ac.pymRs) {
    PymRsInitRender();
    PymRsInitPhysics(ac.pymRs);
    PymRsResetPhysics(ac.pymRs);
    ac.pymRs->drawing_options = ac.drawing_options;
    rbTrackingOptions.init_items();
    pym_config_t *pymCfg = &ac.pymRs->pymCfg;
    vector<pym_rb_t> body_states(pymCfg->nBody);
    memcpy(&body_states[0], ac.pymRs->pymCfg.body, sizeof(pym_rb_t)*pymCfg->nBody);
    ac.rb_history.push_back(body_states); // First frame
    assert(ac.rb_history.size() == 1);

    /* Query latest(newest) screen shot file name
       saved in /home/johnu/pymss. */
    std::string ssPathStr(getenv("WORKING"));
    ssPathStr += "/pymss/";
    fs::path pathSS(ssPathStr);
    if ( !exists( pathSS ) ) {
      assert(!"Error: Screenshot path does not exist!\n");
      abort();
    }
    fs::directory_iterator end_itr; // default construction yields past-the-end
    int ssIdx = 0;
    for ( fs::directory_iterator itr( pathSS ); itr != end_itr; ++itr ) {
      if ( fs::is_directory(itr->status()) ) {
        /* Do nothing */
      } else {
	      std::string fn(itr->path().filename().generic_string());
	      fn = fn.substr(0, fn.find('.'));
	      int idx = boost::lexical_cast<int>(fn);
	      //std::cout << fn << " " << idx << std::endl;
	      if (idx > ssIdx)
	        ssIdx = idx;
      }
    }
    ac.pymRs->ssIdx = ssIdx + 1; /* Screenshot file name continues... */
  }

  ret = Fl::run();

  if (ac.pymRs) {
    PymRsDestroyPhysics(ac.pymRs);
    PymRsDestroyRender();
    PymRsDestroyContext(ac.pymRs);
    ac.pymRs = 0;
  }
  std::ofstream windowPosAndSize("BwWindow.txt");
  windowPosAndSize << topWindow.x() << " " << topWindow.y() << " "
		   << topWindow.w() << " " << topWindow.h() << std::endl;
  windowPosAndSize.close();
  return ret;
}

int main4()
{
  // Quaternion EOM test box
  unsigned long long i = 0;
  while (true) {
    static AranMath::Quaternion q0(VectorR3(1,2,3),0.8);
    static AranMath::Quaternion q1(VectorR3(1,2,3),0.800005);
    const double h = 0.01;
    q0.normalize();
    q1.normalize();
    AranMath::Quaternion q2 = quaternion_eom(q0, q1, h);
    VectorR3 omega = calc_angular_velocity(q1, q0, h);
    if (i%100000 == 0)
      std::cout << i << " : " << omega << std::endl;
    q0 = q1;
    q1 = q2;
    ++i;
  }
  return 0;
  
}

int main3()
{
  // Quaternion EOM test box
  unsigned long long i = 0;
  while (true) {
    static AranMath::Quaternion q0(VectorR3(0,1,0),0.8);
    static VectorR3 omega0(0.3, 0.2, 0.1);
    q0.normalize();
    if (i%100000 == 0)
      std::cout << i << " : " << omega0 << std::endl;

    const double h = 0.01;
    AranMath::Quaternion q1;
    VectorR3 omega1;
    quaternion_eom_av_rkn4(q1, omega1, q0, omega0, 0, h);
    q0 = q1;
    omega0 = omega1;
    ++i;
  }
  return 0;
}

int main(int argc, char **argv)
{
  static aran::core::PathManager pm;
  pm.set_shader_dir("resources/shaders/");

  int ret = doMain(argc, argv);
  if (ret) {
    std::cout << "Error detected.\n";
  }
#ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
  // Simple check for the memory leak of ArnObjects.
  std::cout << "ArnObject ctor count: " << ArnObject::getCtorCount() << std::endl;
  std::cout << "ArnObject dtor count: " << ArnObject::getDtorCount() << std::endl;
  ArnObject::printInstances();
#endif // #ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
  return ret;
}
