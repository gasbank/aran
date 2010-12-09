#include "BwPch.h"
#include "BwMain.h"
#include "BwTopWindow.h"
#include "BwOpenGlWindow.h"
#include "BwAppContext.h"
#include "BwWin32Timer.h"
#include "BwDrawingOptionsWindow.h"
#include "BwPlaybackSlider.h"
#include "SliderInput.h"

namespace fs = boost::filesystem;

void idle_cb(void* ac);

#if !HAVE_GL
#error OpenGL in FLTK not enabled.
#endif

struct SceneButtonsHolder
{
  BwAppContext* ac;
  MessageHandleResult mhr;
};

void dump_cb(Fl_Widget *o, void *ac_raw)
{
}

void print_rb0_cb(Fl_Widget *o, void *ac_raw)
{
}

void load_cb(Fl_Widget *o, void *ac_raw)
{
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
    }
  }

  if (ac.bNextCamera) {
    ac.activeCam = ac.sgPtr->getNextCamera(ac.activeCam);
    ac.bNextCamera = false;
  }

  ArnVec3 cameraDiff(0, 0, 0);
  static const float cameraDiffAmount = 0.1f;

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
  /// 이러한 전역 OpenGL 스테이트를 변경하는 것은 최대한 여기서
  /// 초기화 단계에 한번만 하는 것으로 해야 합니다.
  /// 일부분 코드에 대해서만 스테이트를 잠시 변경할 필요가 있을 때는
  /// 반드시 glAttribPush/Pop() 함수를 이용하여
  /// 전역 설정이 완전히 변경되는 경우가 없도록 해야 합니다.
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
  //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  glEnable(GL_NORMALIZE);
  for (int lightId = 0; lightId < 8; ++lightId)
  {
    glDisable(GL_LIGHT0 + lightId);
  }

  /// OpenGL 확장 기능을 초기화합니다.
  if (ArnInitGlExtFunctions() < 0)
  {
    std::cerr << " *** OpenGL extensions needed to run this program are not available." << std::endl;
    std::cerr << "     Check whether you are in the remote control display or have a legacy graphics adapter." << std::endl;
    std::cerr << "     Aborting..." << std::endl;
    return -50;
  }

  /// ARAN OpenGL 패키지를 초기화합니다.
  if (ArnInitializeGl() < 0)
  {
    return -3;
  }

  /// Mass map 텍스처를 생성합니다.
  glGenTextures(1, &ac.massMapTex);
  glBindTexture( GL_TEXTURE_2D, ac.massMapTex );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glBindTexture(GL_TEXTURE_2D, 0);

  if (InitializeRendererDependentsFromSg(ac) < 0)
  {
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
  if (LoadSceneList(ac.sceneList) < 0) {
    std::cerr << " *** Init failed..." << std::endl;
    return -10;
  }
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
  for (int i = 0; i < do_count; ++i)
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
  static int simFrame = 0;
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

  /*int frame_move_ret = 0;
  char result_msg[2048] = {0,};
  std::string sss(ss.str());
  static char slidersss[1024];
  strncpy(slidersss, sss.c_str(), 1024);
  ac.slider->label(slidersss);*/
}

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
}

void set_vel_zero_cb(Fl_Widget *o, void *p)
{
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

  // Style table
  Fl_Text_Display::Style_Table_Entry stable[] = {
    // FONT COLOR      FONT FACE   FONT SIZE
    // --------------- ----------- --------------
    {  FL_RED,         FL_COURIER, 10 }, // A - Red
    {  FL_DARK_YELLOW, FL_COURIER, 10 }, // B - Yellow
    {  FL_DARK_GREEN,  FL_COURIER, 10 }, // C - Green
    {  FL_BLUE,        FL_COURIER, 10 }, // D - Blue
  };

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
  
  ret = Fl::run();

  std::ofstream windowPosAndSize("BwWindow.txt");
  windowPosAndSize << topWindow.x() << " " << topWindow.y() << " "
    << topWindow.w() << " " << topWindow.h() << std::endl;
  windowPosAndSize.close();
  return ret;
}

int main(int argc, char **argv)
{
  aran::core::PathManager pm;
  pm.set_shader_dir("resources/shaders/");
  pm.set_model_dir("resources/models/");

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
