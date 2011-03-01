#include "BwPch.h"
#include "BwOpenGlWindow.h"
#include "BwAppContext.h"
#include "IL/il.h"
#include "QuaternionEOM.h"

#if _MSC_VER
#define snprintf _snprintf
#endif

static void SelectGraphicObject(BwAppContext& ac, const float mousePx, const float mousePy);
static void RenderScene(const BwAppContext& ac);
static void RenderHud(const BwAppContext& ac);

BwOpenGlWindow::BwOpenGlWindow(int x, int y, int w, int h, const char *l, BwAppContext& ac)
  : Fl_Gl_Window(x, y, w, h, l)
  , m_ac(ac)
  , m_cam_r(10.0)
  , m_cam_phi(0)
  , m_cam_dphi(0)
  , m_cam_theta(0)
  , m_cam_dtheta(0)
  , m_screenshot_fbyf(false)
{
  sides		    = overlay_sides = 3;
  m_ac.windowWidth  = w;
  m_ac.windowHeight = h;
  m_ac.avd.Width    = m_ac.windowWidth;
  m_ac.avd.Height   = m_ac.windowHeight;
  m_cam_cen[0] = 3;
  m_cam_cen[1] = 0;
  m_cam_cen[2] = 1;
  m_cam_dcen[0] = 0;
  m_cam_dcen[1] = 0;
  m_cam_dcen[2] = 0;
}

BwOpenGlWindow::~BwOpenGlWindow()
{
}

void BwOpenGlWindow::setCamCen(double cx, double cy, double cz) {
  m_cam_cen[0] = cx;
  m_cam_cen[1] = cy;
  m_cam_cen[2] = cz;
}

void BwOpenGlWindow::resize( int x, int y, int w, int h )
{
  m_ac.windowWidth = w;
  m_ac.windowHeight = h;
  m_ac.avd.Width	= m_ac.windowWidth;
  m_ac.avd.Height	= m_ac.windowHeight;
  //printf("Resized OpenGL widget size is %d x %d.\n", m_ac.windowWidth, m_ac.windowHeight);
  Fl_Gl_Window::resize(x, y, w, h);
}

static void pym_cross3(double *u, const double *const a,
  const double *const b) {
    u[0] = a[1]*b[2] - a[2]*b[1];
    u[1] = a[2]*b[0] - a[0]*b[2];
    u[2] = a[0]*b[1] - a[1]*b[0];
}

void BwOpenGlWindow::draw()
{ 
  // the valid() property may be used to avoid reinitializing your
  // GL transformation for each redraw:
  if (!valid()) {
    valid(1);
    glLoadIdentity();
    glViewport(0, 0, w(), h());
  }
  glClearColor(0.8,0.8,0.8,1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  if (m_ac.activeCam) {
    m_cam_r = 5;

    double cam_r = 10, cam_phi = 1.0, cam_dphi = 0;
    double cam_theta = 1.0, cam_dtheta = 1.0;
    pym_render_config_t rc;
    rc.cam_r	 = m_cam_r;
    rc.cam_phi	 = m_cam_phi + m_cam_dphi;
    rc.cam_theta = m_cam_theta + m_cam_dtheta;
    rc.vpx	 = 0;
    rc.vpy	 = 0;
    rc.vpw	 = w(); //min(w(), h());
    rc.vph	 = h(); //min(h(), h());
    for (int i = 0; i < 3; ++i)
      rc.cam_cen[i] = m_cam_cen[i] + m_cam_dcen[i];

    assert(glGetError() == GL_NO_ERROR);

    double cam_car[3];            /* Cartesian coordinate of cam */
    double up_dir[3];
    pym_sphere_to_cartesian(cam_car, rc.cam_r, rc.cam_phi, rc.cam_theta);
    //printf("cam_car = %lf %lf %lf\n", cam_car[0], cam_car[1], cam_car[2]);
    pym_up_dir_from_sphere(up_dir, cam_car, rc.cam_r,
      rc.cam_phi, rc.cam_theta);
   

    double right_dir[3];
    double cam_car_nor[3];
    /* printf("cam_car = %lf %lf %lf\n", */
    /* 	 cam_car[0], cam_car[1], cam_car[2]); */
   
    cam_car_nor[0] = cam_car[0] / m_cam_r;
    cam_car_nor[1] = cam_car[1] / m_cam_r;
    cam_car_nor[2] = cam_car[2] / m_cam_r;
    pym_cross3(right_dir, up_dir, cam_car_nor);
    ArnMatrix cam_mat(
      right_dir[0], up_dir[0], cam_car_nor[0],   rc.cam_cen[0]+cam_car[0],
      right_dir[1], up_dir[1], cam_car_nor[1],   rc.cam_cen[1]+cam_car[1],
      right_dir[2], up_dir[2], cam_car_nor[2],   rc.cam_cen[2]+cam_car[2],
      0, 0, 0, 1);
    m_ac.activeCam->setLocalXform(cam_mat);
  }
  
  RenderScene(m_ac);

  double cam_r = 10, cam_phi = 1.0, cam_dphi = 0;
  double cam_theta = 1.0, cam_dtheta = 1.0;
  pym_render_config_t rc;
  rc.cam_r	 = m_cam_r;
  rc.cam_phi	 = m_cam_phi + m_cam_dphi;
  rc.cam_theta = m_cam_theta + m_cam_dtheta;
  rc.vpx	 = 0;
  rc.vpy	 = 0;
  rc.vpw	 = w(); //min(w(), h());
  rc.vph	 = h(); //min(h(), h());
  for (int i = 0; i < 3; ++i)
    rc.cam_cen[i] = m_cam_cen[i] + m_cam_dcen[i];

  assert(glGetError() == GL_NO_ERROR);

  pym_configure_light();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45, double(w()) / h(), 0.01, 1000);

  assert(glGetError() == GL_NO_ERROR);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  double cam_car[3];            /* Cartesian coordinate of cam */
  double up_dir[3];
  pym_sphere_to_cartesian(cam_car, rc.cam_r, rc.cam_phi, rc.cam_theta);
  //printf("cam_car = %lf %lf %lf\n", cam_car[0], cam_car[1], cam_car[2]);
  pym_up_dir_from_sphere(up_dir, cam_car, rc.cam_r,
    rc.cam_phi, rc.cam_theta);
  gluLookAt(rc.cam_cen[0]+cam_car[0],
    rc.cam_cen[1]+cam_car[1],
    rc.cam_cen[2]+cam_car[2],
    rc.cam_cen[0],
    rc.cam_cen[1],
    rc.cam_cen[2],
    up_dir[0],
    up_dir[1],
    up_dir[2]);

  assert(glGetError() == GL_NO_ERROR);

  // Draw the world coordinate system indicator
  glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
  glDisable(GL_LIGHTING);
  glLineWidth(5.0);
  glBegin(GL_LINES);
  glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(1, 0, 0); 
  glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 1, 0);
  glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 1);
  glEnd();
  glPopAttrib();

  
  if (m_ac.pymRs) {
    PymRsRender(m_ac.pymRs, &rc);
  }

  assert(glGetError() == GL_NO_ERROR);

  // Quaternion EOM test box
  static AranMath::Quaternion q0(VectorR3(0,1,0),0.8);
  static VectorR3 omega0(5, 5, 0);
  q0.normalize();
  m_omega = omega0;
  const double h = 0.005;
  AranMath::Quaternion q1;
  VectorR3 omega1;
  quaternion_eom_av_rk4(q1, omega1, q0, omega0, 0, h);
  ArnQuat q1_new(q1.x, q1.y, q1.z, q1.w);
  ArnMatrix q1mat;
  q1_new.getRotationMatrix(&q1mat);
  static const double regularBoxSize[3] = { 1, 1, 1 };
  static const double ZERO[3] = {0,};

  assert(glGetError() == GL_NO_ERROR);
  glPushMatrix();
  {
    glMultMatrixf((const GLfloat *)q1mat.m);
    //DrawBox_pq(ZERO, 0, regularBoxSize, 0);
  }
  q0 = q1;
  omega0 = omega1;
  glPopMatrix();



  assert(glGetError() == GL_NO_ERROR);

  if (m_screenshot_fbyf) {
    /* Take a screenshot */
    //Allocate memory for storing the image
    GLvoid *imageData = malloc(rc.vpw*rc.vph*32);
    //Copy the image to the array imageData
    glReadPixels(0, 0, rc.vpw, rc.vph, GL_RGBA, GL_UNSIGNED_BYTE, imageData); 
    ILuint ImageName; // The image name to return.
    ilGenImages(1, &ImageName); // Grab a new image name.
    //printf("%d\n", ImageName);
    ilBindImage(ImageName);
    ilTexImage(rc.vpw, rc.vph, 0, 4, IL_RGBA, IL_UNSIGNED_BYTE, imageData);
    ilEnable(IL_FILE_OVERWRITE);
    assert(m_ac.pymRs->ssIdx >= 0 && m_ac.pymRs->ssIdx <= 99999);
    char ssName[128];
    std::string ssPath(getenv("WORKING"));
    ssPath += "/pymss/%05d.jpg";
    snprintf(ssName, 128, ssPath.c_str(), m_ac.pymRs->ssIdx);
    ilSaveImage(ssName);
    //printf("%s\n", ssName);
    ilDeleteImages(1, &ImageName);
    free(imageData);
    ++m_ac.pymRs->ssIdx;
  }

  assert(glGetError() == GL_NO_ERROR);

  /* Check for error conditions. */
  GLenum gl_error = glGetError();
  if( gl_error != GL_NO_ERROR ) {
    std::cerr << "ARAN: OpenGL error: " << gluErrorString(gl_error) << std::endl;
    abort();
  }
}

void BwOpenGlWindow::handle_push() {
  const int mouseX = Fl::event_x();
  const int mouseY = Fl::event_y();
  m_dragX = mouseX;
  m_dragY = mouseY;
  SelectGraphicObject(m_ac,
		      float(mouseX),
		      float(m_ac.avd.Height - mouseY) // Note that Y-coord flipped.
		      );

  if (m_ac.sgPtr) {
    ArnMatrix modelview, projection;
    glGetFloatv(GL_MODELVIEW_MATRIX, reinterpret_cast<GLfloat*>(modelview.m));
    modelview = modelview.transpose();
    glGetFloatv(GL_PROJECTION_MATRIX, reinterpret_cast<GLfloat*>(projection.m));
    projection = projection.transpose();
    ArnVec3 origin, direction;
    ArnMakePickRay(&origin, &direction,
		   float(mouseX), float(m_ac.avd.Height - mouseY),
		   &modelview, &projection, &m_ac.avd);
    ArnNode* firstNode = m_ac.sgPtr->findFirstNodeOfType(NDT_RT_MESH);
    ArnMesh* mesh = reinterpret_cast<ArnMesh*>(firstNode);
    if (mesh) {
      bool bHit = false;
      unsigned int faceIdx = 0;
      ArnIntersectGl(mesh, &origin, &direction, &bHit, &faceIdx, 0, 0, 0, 0, 0);
      if (bHit)
	printf("Hit on Face %u of mesh %s\n", faceIdx, mesh->getName());
    }
  }
}

void BwOpenGlWindow::handle_release() {
  m_cam_phi    += m_cam_dphi;
  m_cam_theta  += m_cam_dtheta;
  m_cam_dphi	= 0;
  m_cam_dtheta  = 0;
  for (int i = 0; i < 3; ++i) {
    m_cam_cen[i] += m_cam_dcen[i];
    m_cam_dcen[i] = 0;
  }
}

void BwOpenGlWindow::handle_drag() {
  const int dx	 = Fl::event_x() - m_dragX;
  const int dy	 = Fl::event_y() - m_dragY;
  if (Fl::event_key() == FL_Button + FL_LEFT_MOUSE) {
    // Left mouse button dragging - trackball rotating
    m_cam_dphi	 = -(double)dx/200;
    m_cam_dtheta	 = -(double)dy/200;
    redraw();
  } else if (Fl::event_key() == FL_Button + FL_RIGHT_MOUSE) {
    // Right mouse button dragging - panning through XY plane
    double look[3], up[3], left[3];
    PymLookUpLeft(look, up, left, m_cam_r, m_cam_phi, m_cam_theta);
    up[2] = 0;
    const double upProjectedNorm = PymNorm(2, up);
    double upProjected[2];
    for (int k = 0; k < 2; ++k)
      upProjected[k] = up[k] / upProjectedNorm;
    const double pro = 0.01;
    m_cam_dcen[0] = left[0]*pro*dx + upProjected[0]*pro*dy;
    m_cam_dcen[1] = left[1]*pro*dx + upProjected[1]*pro*dy;
    m_cam_dcen[2] = 0;
    redraw();
  }
}

void BwOpenGlWindow::handle_move() {
  mousePosition.first  = Fl::event_x();
  mousePosition.second = Fl::event_y();
  //    printf("%d %d\n", mousePosition.first, mousePosition.second);
  if (m_ac.bPanningButtonDown) {
    int dx = mousePosition.first - m_ac.panningStartPoint.first;
    int dy = mousePosition.second - m_ac.panningStartPoint.second;
    const float aspectRatio = (float)m_ac.windowWidth / m_ac.windowHeight;
    const float d1 =
      -(2.0f * m_ac.orthoViewDistance * aspectRatio / m_ac.windowWidth) * dx;
    const float d2 =
      -(-2.0f * m_ac.orthoViewDistance / m_ac.windowHeight) * dy;
    if (m_ac.viewMode == VM_TOP) {
      m_ac.dPanningCenter[0] = d1;
      m_ac.dPanningCenter[1] = d2;
    } else if (m_ac.viewMode == VM_RIGHT) {
      m_ac.dPanningCenter[1] = d1;
      m_ac.dPanningCenter[2] = d2;
    } else if (m_ac.viewMode == VM_BACK) {
      m_ac.dPanningCenter[0] = d1;
      m_ac.dPanningCenter[2] = d2;
    }
    //printf("%f   %f\n", m_ac.dPanningCenter.first, m_ac.dPanningCenter.second);
    redraw();
  }
}

void BwOpenGlWindow::handle_mousewheel() {
  if (m_ac.viewMode == VM_TOP || m_ac.viewMode == VM_RIGHT || m_ac.viewMode == VM_BACK) {
    if (Fl::event_dy() > 0) {
      ++m_ac.orthoViewDistance;
      redraw();
    } else if (Fl::event_dy() < 0) {
      if (m_ac.orthoViewDistance > 1)
	--m_ac.orthoViewDistance;
      redraw();
    }
  }
}

int BwOpenGlWindow::handle_keydown() {
  int key = Fl::event_key();
  if (key == FL_Escape) // ESC key
    return 0;
  if (key == ' ') { // SPACE key
    if (!m_ac.bPanningButtonDown) {
      m_ac.bPanningButtonDown = true;
      m_ac.panningStartPoint = mousePosition;
    }
  }
 
  pym_rb_named_t *rbnTrunk = 0;
  int j;
  const int nb = m_ac.pymRs->pymCfg.nBody;
  FOR_0(j, nb) {
    pym_rb_named_t *rbn = &m_ac.pymRs->pymCfg.body[j].b;
    if (strcmp(rbn->name, "trunk") == 0) {
      rbnTrunk = rbn;
    }
  }
  srand(time(0));
  double look[3], up[3], left[3];
  PymLookUpLeft(look, up, left, m_cam_r, m_cam_phi, m_cam_theta);
  up[2] = 0;
  const double upNorm = sqrt(up[0]*up[0]+up[1]*up[1]+up[2]*up[2]);
  for (int k = 0; k < 3; ++k)
    up[k] /= upNorm;
  if (key == FL_Left) {
    for (int k = 0; k < 3; ++k)
      m_cam_cen[k] += left[k]*0.1;
    return 1;
  } else if (key == FL_Right) { // RIGHT key
    for (int k = 0; k < 3; ++k)
      m_cam_cen[k] -= left[k]*0.1;
    return 1;
  } else if (key == FL_Up) { // UP key
    for (int k = 0; k < 3; ++k)
      m_cam_cen[k] += up[k]*0.1;
    return 1;
  } else if (key == FL_Down) { // DOWN key
    for (int k = 0; k < 3; ++k)
      m_cam_cen[k] -= up[k]*0.1;
    return 1;
  } else if (key == 'f') {
    m_ac.pymRs->pymCfg.renderFibers = (m_ac.pymRs->pymCfg.renderFibers) ? 0 : 1;
    return 1;
  } else if (key == 'x') {
    int ti1 = rand()%360;
    int ti2 = rand()%35 + 55;
    double r = 3000;
    double phi = ti1/360.0*2*M_PI;
    double theta = ti2/360.0*2*M_PI;
    double car[3];
    pym_sphere_to_cartesian(car, r, phi, theta);
    if (rbnTrunk) {
      rbnTrunk->extForce[0] = -car[0];
      rbnTrunk->extForce[1] = -car[1];
      rbnTrunk->extForce[2] = -car[2];
      printf("%lf, %lf\n", rbnTrunk->extForce[0], rbnTrunk->extForce[1]);
      double tx = (rand()%100-50)/50.0;
      double ty = (rand()%100-50)/50.0;
      double tz = (rand()%100-50)/50.0;
      if (rbnTrunk) {
        rbnTrunk->extForcePos[0] = tx/3;
        rbnTrunk->extForcePos[1] = ty/3;
        rbnTrunk->extForcePos[2] = tz/3;
      }
    }
    return 1;
  }
  redraw();
  //printf("key=%d\n", key);
  return 0;
}

int BwOpenGlWindow::handle_keyup() {
  int key = Fl::event_key();
  if (key == 65307) // ESC key
    return 0;
  if (key < 256)
    m_ac.bHoldingKeys[key] = true;

  if (key == 32) {// SPACE key
    if (m_ac.bPanningButtonDown) {
      m_ac.bPanningButtonDown = false;

      m_ac.panningCenter[0] += m_ac.dPanningCenter[0];
      m_ac.panningCenter[1] += m_ac.dPanningCenter[1];
      m_ac.panningCenter[2] += m_ac.dPanningCenter[2];

      m_ac.dPanningCenter[0] = 0;
      m_ac.dPanningCenter[1] = 0;
      m_ac.dPanningCenter[2] = 0;
      redraw();
    }
  } else if (key == FL_KP + '7') {
    m_ac.viewMode = VM_TOP;
    //printf("  View mode set to top.\n");
    redraw();
    return 1;
  } else if (key == FL_KP + '3') {
    m_ac.viewMode = VM_RIGHT;
    //printf("  View mode set to left.\n");
    redraw();
    return 1;
  } else if (key == FL_KP + '1') {
    m_ac.viewMode = VM_BACK;
    //printf("  View mode set to front.\n");
    redraw();
    return 1;
  } else if (key == FL_KP + '4') {
    m_ac.viewMode = VM_CAMERA;
    //printf("  View mode set to camera.\n");
    redraw();
    return 1;
  }
  return 0;
}

int BwOpenGlWindow::handle( int eventType )
{
  if (eventType == FL_PUSH) {
    handle_push();
    return 1;
  } else if (eventType == FL_RELEASE) {
    handle_release();
    return 1;
  } else if (eventType == FL_DRAG) {
    handle_drag();
    return 1;
  } else if (eventType == FL_ENTER) {
    //take_focus();
    return 1;
  } else if (eventType == FL_FOCUS) {
    return 1;
  } else if (eventType == FL_UNFOCUS) {
    return 1;
  } else if (eventType == FL_MOVE) {
    handle_move();
    return 1;
  } else if (eventType == FL_MOUSEWHEEL) {
    handle_mousewheel();
    return 1;
  } else if (eventType == FL_KEYDOWN) {
    return handle_keydown();
  } else if (eventType == FL_KEYUP) {
    return handle_keyup();
  }
  return Fl_Gl_Window::handle(eventType);
}

//////////////////////////////////////////////////////////////////////////


static void
SelectGraphicObject(BwAppContext& ac, const float mousePx, const float mousePy)
{
  if (!ac.sgPtr || !ac.activeCam)
    return;

  HitRecord buff[16];
  GLint hits, view[4];

  /* This choose the buffer where store the values for the selection data */
  glSelectBuffer(4*16, reinterpret_cast<GLuint*>(buff));

  /* This retrieve info about the viewport */
  glGetIntegerv(GL_VIEWPORT, view);

  /* Switching in selecton mode */
  glRenderMode(GL_SELECT);

  /* Clearing the name's stack. This stack contains all the info about the objects */
  glInitNames();

  /* Now fill the stack with one element (or glLoadName will generate an error) */
  glPushName(0);

  /* Now modify the vieving volume, restricting selection area around the cursor */
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  float origProjMat[16];
  glGetFloatv(GL_PROJECTION_MATRIX, origProjMat);
  glLoadIdentity();

  /* restrict the draw to an area around the cursor */
  const GLdouble pickingAroundFactor = 1.0;
  gluPickMatrix(mousePx, mousePy, pickingAroundFactor, pickingAroundFactor, view);

  /* your original projection matrix */
  glMultMatrixf(origProjMat);

  /* Draw the objects onto the screen */
  glMatrixMode(GL_MODELVIEW);
  /* draw only the names in the stack, and fill the array */
  glFlush();
	
  // Rendering routine START
  ArnSceneGraphRenderGl(ac.sgPtr.get(), true);
  foreach (ArnIkSolver* ikSolver, ac.ikSolvers)
    {
      TreeDraw(*ikSolver->getTree(), ac.drawing_options[pym_do_joint],
        ac.drawing_options[pym_do_endeffector],
        ac.drawing_options[pym_do_joint_axis],
        ac.drawing_options[pym_do_root_node]);
    }
  // Rendering routine END

  /* Do you remeber? We do pushMatrix in PROJECTION mode */
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  /* get number of objects drawed in that area and return to render mode */
  hits = glRenderMode(GL_RENDER);

  glMatrixMode(GL_MODELVIEW);

  /* Print a list of the objects */
  printf("---------------------\n");
  for (GLint h = 0; h < hits; ++h)
    {
      if (buff[h].contents) // Zero means that ray hit on bounding box area.
	{
	  const ArnNode* node = ac.sgPtr->getConstNodeById(buff[h].contents);
	  if (node)
	    {
	      const ArnNode* parentNode = node->getParent();
	      const char* name = node->getName();
	      if (strlen(name) == 0)
		name = "<Unnamed>";
	      if (parentNode)
		{
		  const char* parentName = parentNode->getName();
		  if (strlen(parentName) == 0)
		    parentName = "<Unnamed>";
		  printf("[Object 0x%p ID %d %s (Parent Object 0x%p ID %d %s)]\n",
			 node, node->getObjectId(), name, parentNode, parentNode->getObjectId(), parentName);
		}
	      else
		{
		  printf("[Object 0x%p ID %d %s]\n",
			 node, node->getObjectId(), name);
		}

	      const ArnMesh* mesh = dynamic_cast<const ArnMesh*>(parentNode);
	      if (mesh)
		{
		  ArnVec3 dim;
		  mesh->getBoundingBoxDimension(&dim, true);
		  printf("Mesh Dimension: "); dim.printFormatString();
		}
	    }

	  foreach (ArnIkSolver* ikSolver, ac.ikSolvers)
	    {
	      Node* node = ikSolver->getNodeByObjectId(buff[h].contents);
	      if (node)
		{
		  printf("[Object 0x%p ID %d %s] Endeffector=%d\n",
			 node, node->getObjectId(), node->getName(), node->isEndeffector());
		  /*
		    if (ac.bHoldingKeys[SDLK_LSHIFT])
		    {
		    ikSolver->reconfigureRoot(node);
		    }
		    else
		    {
		    if (node->isEndeffector())
		    {
		    ikSolver->setSelectedEndeffector(node);
		    }
		    }
		  */
		}
	    }
	}
    }
}


static void RenderGrid(const BwAppContext& ac, const float gridCellSize, const int gridCellCount, const float gridColor[3], const float thickness)
{
  glColor3fv(gridColor);
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_LIGHTING);

  // subgrid
  glLineWidth(thickness);
  glBegin(GL_LINES);
  const float v1 = gridCellCount * gridCellSize;
  for (int i = -gridCellCount; i <= gridCellCount; ++i)
    {		
      const float v2 = gridCellSize * i;

      switch (ac.viewMode)
	{
	case VM_UNKNOWN:
	case VM_CAMERA:
	case VM_TOP:
	  // X direction
	  glVertex3f(-v1, v2, 0);
	  glVertex3f( v1, v2, 0);
	  // Y direction
	  glVertex3f(v2, -v1, 0);
	  glVertex3f(v2,  v1, 0);
	  break;
	case VM_RIGHT:
	  // Y direction
	  glVertex3f(0, -v1, v2);
	  glVertex3f(0,  v1, v2);
	  // Z direction
	  glVertex3f(0, v2, -v1);
	  glVertex3f(0, v2,  v1);
	  break;
	case VM_BACK:
	  // X direction
	  glVertex3f(-v1, 0, v2);
	  glVertex3f( v1, 0, v2);
	  // Z direction
	  glVertex3f(v2, 0, -v1);
	  glVertex3f(v2, 0,  v1);
	  break;
	default:
	  break;
	}
    }
  glEnd();

  glPopAttrib();
}

static void
RenderScene(const BwAppContext& ac)
{
  // DO NOT CLEAR THE OPENGL FRAMEBUFFER HERE
	
  // Set modelview and projection matrices here
  if (ac.viewMode == VM_CAMERA || ac.viewMode == VM_UNKNOWN) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (ac.activeCam) {
	    ArnConfigureViewportProjectionMatrixGl(&ac.avd, ac.activeCam);
	    ArnConfigureViewMatrixGl(ac.activeCam);
	  }
  } else if (ac.viewMode == VM_TOP || ac.viewMode == VM_RIGHT || ac.viewMode == VM_BACK) {
    static float eye[3], at[3], up[3];
    if (ac.viewMode == VM_TOP) {
	    eye[0] = ac.panningCenter[0] + ac.dPanningCenter[0];
	    eye[1] = ac.panningCenter[1] + ac.dPanningCenter[1];
	    eye[2] = 100.0f;

	    at[0] = eye[0];
	    at[1] = eye[1];
	    at[2] = 0;
			
	    up[0] = 0;
	    up[1] = 1.0f;
	    up[2] = 0;
	  } else if (ac.viewMode == VM_RIGHT) {
	    eye[0] = 100.0f;
	    eye[1] = ac.panningCenter[1] + ac.dPanningCenter[1];
	    eye[2] = ac.panningCenter[2] + ac.dPanningCenter[2];

	    at[0] = 0;
	    at[1] = eye[1];
	    at[2] = eye[2];

	    up[0] = 0;
	    up[1] = 0;
	    up[2] = 1.0f;
	  } else if (ac.viewMode == VM_BACK) {
	    eye[0] = ac.panningCenter[0] + ac.dPanningCenter[0];
	    eye[1] = -100.0f;
	    eye[2] = ac.panningCenter[2] + ac.dPanningCenter[2];

	    at[0] = eye[0];
	    at[1] = 0;
	    at[2] = eye[2];

	    up[0] = 0;
	    up[1] = 0;
	    up[2] = 1.0f;
	  }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eye[0], eye[1], eye[2], at[0], at[1], at[2], up[0], up[1], up[2]);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const float aspectRatio = (float)ac.windowWidth / ac.windowHeight;
    const float viewDistance = (float)ac.orthoViewDistance;
    glOrtho(-viewDistance*aspectRatio, viewDistance*aspectRatio, -viewDistance, viewDistance, 0, 10000);
    glMatrixMode(GL_MODELVIEW);
  }
  
  if (ac.activeLight) {
    ArnConfigureLightGl(0, ac.activeLight);
  }

  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc(GL_ONE_MINUS_DST_ALPHA,GL_DST_ALPHA);
  //glBindTexture(GL_TEXTURE_2D, 0);

  if (ac.drawing_options[pym_do_grid]) {
    const static float gridColor[3] = { 0.4f, 0.4f, 0.4f };
    RenderGrid(ac, 0.5f, 10, gridColor, 0.5f);
    RenderGrid(ac, 2.5f, 2, gridColor, 1.0f);
  }
	
  // Render skeletons under control of IK solver
  foreach (ArnIkSolver* ikSolver, ac.ikSolvers) {
    glPushMatrix();
    TreeDraw(*ikSolver->getTree(), ac.drawing_options[pym_do_joint],
      ac.drawing_options[pym_do_endeffector],
      ac.drawing_options[pym_do_joint_axis],
      ac.drawing_options[pym_do_root_node]);
    glPopMatrix();
  }

  // Render the main scene graph
  glPushMatrix();
  {
    if (ac.sgPtr) {
	    ArnSceneGraphRenderGl(ac.sgPtr.get(), true);
    }
  }
  glPopMatrix();

  // Render shadow
  // light vector. LIGHTZ is implicitly 1
  /*
    static const float LIGHTX = 1.0f;
    static const float LIGHTY = 1.0f;
    static const float SHADOW_INTENSITY = 0.65f;
    static const float GROUND_R = 0.5f; 	// ground color for when there's no texture
    static const float GROUND_G = 0.5f;
    static const float GROUND_B = 0.5f;
    glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT);
    glDisable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(GROUND_R*SHADOW_INTENSITY, GROUND_G*SHADOW_INTENSITY, GROUND_B*SHADOW_INTENSITY);
    glDepthRange(0, 0.9999);
    GLfloat matrix[16];
    for (int i = 0; i < 16; i++)
    matrix[i] = 0;
    matrix[ 0] = 1;
    matrix[ 5] = 1;
    matrix[ 8] = -LIGHTX;
    matrix[ 9] = -LIGHTY;
    matrix[15] = 1;
    glPushMatrix();
    glMultMatrixf(matrix);
    if (ac.sgPtr)
    {
    ArnSceneGraphRenderGl(ac.sgPtr.get(), false);
    }
    glPopMatrix();
    glPopAttrib();
  */

  // Render COM indicator and contact points of a biped.
  glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT); // Push Attrib A
  glDisable(GL_DEPTH_TEST);
  {
    if (ac.trunk) {
	    ArnVec3 netContactForce;
	    const unsigned int contactCount = ac.swPtr->getContactCount();
	    // Calculate the net contact force and render the individual contact force
	    for (unsigned int i = 0; i < contactCount; ++i) {
	      ArnVec3 contactPos, contactForce;
	      ac.swPtr->getContactPosition(i, &contactPos);
	      ac.swPtr->getContactForce1(i, &contactForce);
	      netContactForce += contactForce; // Accumulate contact forces

	      // Render the individual contact force and contact point
	      glPushMatrix();
	      {
	        glTranslatef(contactPos.x, contactPos.y, contactPos.z);
	        if (ac.drawing_options[pym_do_contact]) {
		        ArnSetupBasicMaterialGl(&ArnConsts::ARNCOLOR_YELLOW);
		        ArnRenderSphereGl(0.025, 16, 16);
		      }

	        if (ac.drawing_options[pym_do_contact_force]) {
		        glEnable(GL_COLOR_MATERIAL);
		        glBegin(GL_LINES);
		        glColor3f(1, 0, 0); glVertex3f(0, 0, 0);
		        glColor3f(1, 0, 0); glVertex3f(contactForce.x, contactForce.y, contactForce.z);
		        glEnd();
		        glDisable(GL_COLOR_MATERIAL);
		      }

	        // TODO: Contact forces in the second direction. Should be zero.
	        ac.swPtr->getContactForce2(i, &contactForce);
	        //assert(contactForce == ArnConsts::ARNVEC3_ZERO);
	      }
	      glPopMatrix();
	    }
	    glPushMatrix();
	    const ArnVec3& bipedComPos = *ac.bipedComPos.rbegin();
	    glTranslatef(bipedComPos.x, bipedComPos.y, bipedComPos.z);
	    ArnSetupBasicMaterialGl(&ArnConsts::ARNCOLOR_GREEN);
	    ArnRenderSphereGl(0.025, 16, 16); // COM indicator
	    glEnable(GL_COLOR_MATERIAL);
	    //float netContactForceSize = ArnVec3Length(netContactForce);
	    /*
	      glBegin(GL_LINES);
	      glColor3f(0, 0, 1); glVertex3f(0, 0, 0);
	      glColor3f(0, 0, 1); glVertex3f(netContactForce.x, netContactForce.y, netContactForce.z);
	      glEnd();
	    */
	    glDisable(GL_COLOR_MATERIAL);
	    //printf("%.2f, %.2f, %.2f\n", netContactForce.x, netContactForce.y, netContactForce.z);
	    glPopMatrix();
    }
    /*
      foreach (const ArnVec3& isect, ac.isects)
      {
      glPushMatrix();
      glTranslatef(isect.x, isect.y, isect.z);
      ArnSetupBasicMaterialGl(&ArnConsts::ARNCOLOR_WHITE);
      ArnRenderSphereGl(0.025, 16, 16);
      glPopMatrix();
      }
    */
  }
  glPopAttrib();  // Pop Attrib A
}

static void
RenderHud(const BwAppContext& ac)
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  const double aspect = (double)ac.windowWidth / ac.windowHeight;
  glOrtho(-0.5 * aspect, 0.5 * aspect, -0.5, 0.5, 0, 1);
  glMatrixMode(GL_MODELVIEW);

  glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT | GL_LINE_BIT | GL_POINT_BIT);
  glLineWidth(1);
  glPointSize(4);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(0.1f, 0, 0);
  glScalef(0.2, 0.2, 1);

  // Origin indicator
  ArnDrawAxesGl(0.5f);

  // Support polygon
  if (ac.supportPolygon.size())
    {
      glBegin(GL_LINE_LOOP);
      foreach (const ArnVec3& v, ac.supportPolygon)
	{
	  glColor3f(0, 0, 0); glVertex2f(v.x, v.y);
	}
      glEnd();

      glBegin(GL_POINTS);
      foreach (const ArnVec3& contactPos, ac.isects)
	{
	  glVertex2f(contactPos.x, contactPos.y);
	}
      glEnd();
    }

  // Mass map
  if (ac.bipedComPos.size())
    {
      const float devi = BwAppContext::massMapDeviation;
      const ArnVec3& comPos = *ac.bipedComPos.rbegin();
      glEnable(GL_DEPTH_TEST);
      glBindTexture(GL_TEXTURE_2D, ac.massMapTex);
      glBegin(GL_QUADS);
      glTexCoord2f(0, 0); glColor4f(1, 1, 1, 1); glVertex2f(comPos.x - devi, comPos.y - devi);
      glTexCoord2f(1, 0); glColor4f(1, 1, 1, 1); glVertex2f(comPos.x + devi, comPos.y - devi);
      glTexCoord2f(1, 1); glColor4f(1, 1, 1, 1); glVertex2f(comPos.x + devi, comPos.y + devi);
      glTexCoord2f(0, 1); glColor4f(1, 1, 1, 1); glVertex2f(comPos.x - devi, comPos.y + devi);
      glEnd();
      glBindTexture(GL_TEXTURE_2D, 0);

      glDisable(GL_DEPTH_TEST);
      glBegin(GL_LINE_LOOP);
      glColor4f(1, 1, 1, 1); glVertex2f(comPos.x - devi, comPos.y - devi);
      glColor4f(1, 1, 1, 1); glVertex2f(comPos.x + devi, comPos.y - devi);
      glColor4f(1, 1, 1, 1); glVertex2f(comPos.x + devi, comPos.y + devi);
      glColor4f(1, 1, 1, 1); glVertex2f(comPos.x - devi, comPos.y + devi);
      glEnd();
    }

  // COM
  glDisable(GL_DEPTH_TEST);
  glBegin(GL_POINTS);
  float trail = 0;
  const float trailDiff = 1.0f / ac.bipedComPos.size();
  foreach (const ArnVec3& comPos, ac.bipedComPos)
    {
      glColor4f(1, 0, 0, trail); glVertex2f(comPos.x, comPos.y);
      trail += trailDiff;
    }
  glEnd();


  glPopMatrix();
  glPopAttrib();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}