/*
 * PymRs.c: Pymuscle Realtime Simulator
 * 2010 Geoyeob Kim
 */

/*
 * HEADERS
 */
//#define GL_GLEXT_PROTOTYPES
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
///#include <GL/glext.h>
///#include <GL/glx.h>
#include <sys/time.h>
#include <pthread.h>

#include <cholmod.h>
#include <umfpack.h>
#include <mosek.h>
#include <libconfig.h>

#include "include/PrsGraphCapi.h"

#include "PymStruct.h"
#include "PymBiped.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "ConvexHullCapi.h"
#include "PymuscleConfig.h"
#include "StateDependents.h"
#include "Config.h"
#include "DebugPrintDef.h"
#include "Optimize.h"
#include "PymJointAnchor.h"
#include "PymDebugMessageFlags.h"
#include "TrajParser.h"
#include "PhysicsThreadMain.h"
#include "PymCmdLineParser.h"
#include "MathUtil.h"

#include "model.h"
#include "common.h"
#include "image.h"
#include "quaternion.h"

#include "pymrscore.h"
#include "pymrsrender.h"

/*
 * Macros
 */
#define NUM_THREADS  3
#define TCOUNT 10
#define COUNT_LIMIT 12

/*
 * Globals
 */
GLboolean	print_fps	 = GL_FALSE;
GLboolean	wireframe	 = GL_FALSE;
GLboolean	transparency	 = GL_FALSE;
GLboolean	skeleton	 = GL_FALSE;
GLboolean	animation	 = GL_FALSE;
GLboolean	selection	 = GL_FALSE;
GLint		width, height;
Cursor		picking;
int		RENDER_WIDTH	 = 700;
int		RENDER_HEIGHT	 = 700;
int		SHADOW_MAP_RATIO = 1;

/* Model variables */
MODEL_PTR	model		     = NULL;
GLuint		texture_id, sel_bone = 0;
GLfloat		t_inc		     = 0.1f;
GLuint		pose		     = 1, old_pose=0;

/* Motion and view variables */
const GLfloat	tStep	      = 100.f;	//Translational step
const GLfloat	aStep	      = 0.001f;	//Rotational step (radians)
const GLfloat	mouse_scale_t = 0.001f;	//Mouse smoothing translations
const GLfloat	mouse_scale_r = 0.2f;	//Mouse smoothing rotations
const GLfloat	mouse_scale_a = 0.002f;	//Mouse smoothing angles
GLfloat		t1	      = 0.0f;
GLfloat		xRot	      = 0.f;
GLfloat		yRot	      = 0.f;
GLfloat		xCam, yCam, zCam, eCam, aCam, lCam, vCam, dCam;
GLfloat		sinE, cosE, sinA, cosA, fScale;
GLint		old_x, old_y;

/* Timer */
struct timeval	tv;
double		etime, dt;
double		t0	 = 0.0f, t2;
GLuint		fps_font;
GLfloat		Timed	 = 0.5f;
GLfloat		fps_mean = 0.0f, fps_count = 0.0f;

/* Threads */
int     count = 0;
int     thread_ids[3] = {0,1,2};
pthread_mutex_t count_mutex, main_mutex;
pthread_cond_t count_threshold_cv, physics_thread_finished;



double		zoomRatio = 1.0;

/* Camera position denoted by spherical coordinate system */
/* r     - distance between center pos */
/* phi   - azimuth direction (angle with -Y axis) */
/* theta - zenith  direction (angle with +Z axis) */
static double	cam_r	   = 10;     
static double	cam_phi	   = 30.0/180*M_PI;
static double	cam_dphi   = 0;
static double	cam_theta  = 60.0/180*M_PI;
static double	cam_dtheta = 0;
//static double	cam_cen[3];
static int	g_mouse_pressed_pos_x;
static int	g_mouse_pressed_pos_y;
static int	g_mouse_dragging;

int isExtensionSupported(const char *extension) {
  const GLubyte *extensions = NULL;
  const GLubyte *start;
  GLubyte *where, *terminator;
  /* Extension names should not have spaces. */
  where = (GLubyte *) strchr(extension, ' ');
  if (where || *extension == '\0')
    return 0;
  extensions = glGetString(GL_EXTENSIONS);
  /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings,
     etc. */
  start = extensions;
  for (;;) {
    where = (GLubyte *) strstr((const char *) start, extension);
    if (!where)
      break;
    terminator = where + strlen(extension);
    if (where == start || *(where - 1) == ' ')
      if (*terminator == ' ' || *terminator == '\0')
	return 1;
    start = terminator;
  }
  return 0;
}

void change_size(GLsizei w, GLsizei h)
{
  printf("Size changed to %d x %d.\n", w, h);
  GLfloat m[4][4];
  GLfloat sine, cotangent, deltaZ;
  GLfloat radians, fAspect;
  const GLfloat zNear = 0.1f, zFar = 100.f, fovy = 45.f;

  if (h == 0)
    h = 1;

  width = w;
  height = h;

  glViewport(0, 0, w, h);

  fAspect = (GLfloat)w / (GLfloat)h;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  radians = fovy * PI / 360.f;
  deltaZ = zFar - zNear;
  sine = sinf(radians);
  cotangent = cosf(radians) / sine;

  m[0][0] = cotangent / fAspect;
  m[0][1] = 0.f;
  m[0][2] = 0.f;
  m[0][3] = 0.f;
  m[1][0] = 0.f;
  m[1][1] = cotangent;
  m[1][2] = 0.f;
  m[1][3] = 0.f;
  m[2][0] = 0.f;
  m[2][1] = 0.f;
  m[2][2] = -(zFar + zNear) / deltaZ;
  m[2][3] = -1.f;
  m[3][0] = 0.f;
  m[3][1] = 0.f;
  m[3][2] = -2.f * zNear * zFar / deltaZ;
  m[3][3] = 0.f;

  glMultMatrixf(&m[0][0]);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

/*
 * setup
 */

int setup(const char* filename)
{
  char* fn=NULL;

  //Background color (Ligth blue)
  //glClearColor(0.0f, 0.4f, 0.8f, 1.0f);

  //Set up OpenGL rendering context
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  fn=(char *)malloc(strlen(filename)+6);
  strcpy(fn, filename);
  strcat(fn,".model");
  if(!(model=loadModel(fn)))
    {
      printf("Error: couldn't load model from %s.model\n", filename);
      return 1;
    }

  strcpy(fn, filename);
  strcat(fn,".tga");
  if (!LoadTexture(&texture_id, fn, GL_TRUE, 0.0f))
    {
      printf("Error: couldn't load texture from %s.tga\n", filename);
      return 1;
    }
  free(fn);

  //Make joints look better
  glPointSize(4.0f);

  // Set initial camera position and orientation
  xCam = 7.5f;
  yCam = 7.5f;
  zCam = 7.5f;

  eCam = -PI/8;
  aCam = -PI/4;

  lCam = 0.0f;
  vCam = 0.0f;
  dCam = 0.0f;

  // Initialise timer
  gettimeofday(&tv, NULL);
  t0 = (double)tv.tv_sec + tv.tv_usec / 1000000.0f;
  t2 = t0;

  return 0;
}

/*
 * Draws a model's skeleton
 *
 */

void drawSkeleton(MODEL_PTR model)
{

  GLuint i;

  glDisable(GL_TEXTURE_2D);

  //Draw bones
  glColor3f(1.0f, 0.0f, 0.0f);
  glBegin(GL_LINES);
  for(i=0; i<model->num_bones; i++)
    if(model->bone[i].parent!=-1)
      {
	glVertex3fv(&model->bone[model->bone[i].parent].mva[12]);
	glVertex3fv(&model->bone[i].mva[12]);
      }
  glEnd();

  //Draw selected joint
  if(sel_bone)
    {
      glColor3f(1.0f, 1.0f, 1.0f);
      glBegin(GL_POINTS);
      glVertex3fv(&model->bone[sel_bone-1].mva[12]);
      glEnd();
    }

  glEnable(GL_TEXTURE_2D);

  return;
}

/*
 * Draw a model
 *
 */

void drawModel(MODEL_PTR model)
{

  GLuint i;

  glBegin(GL_TRIANGLES);
  for(i=0; i<model->num_tris*3; i++)
    {

      //Select color
      if(sel_bone==0)
	glColor4f(0.7f, 0.7f, 0.7f, 0.5f); //No bones selected
      else if (model->bone[sel_bone-1].parent==-1)
	glColor4f(1.0f, 1.0f, 1.0f, 0.5f); //All bones selected
      else //One bone selected
	glColor4f(0.7f+0.3f*model->vertex[model->tris[i]].w[sel_bone-1],
		  0.7f+0.3f*model->vertex[model->tris[i]].w[sel_bone-1],
		  0.7f+0.3f*model->vertex[model->tris[i]].w[sel_bone-1],
		  0.5f);

      glTexCoord2fv(model->texcoord[i].M);
      glVertex3fv(model->vertex[model->tris[i]].pos.M);
    }
  glEnd();

  return;
}

/*
 * Compute modelview matrix for selected bone and given euler angles (1-2-3)
 */
void compute_mv(GLuint bone, GLfloat roll, GLfloat pitch, GLfloat yaw)
{

  GLfloat cosR=cosf(roll);
  GLfloat sinR=sinf(roll);
  GLfloat cosP=cosf(pitch);
  GLfloat sinP=sinf(pitch);
  GLfloat cosY=cosf(yaw);
  GLfloat sinY=sinf(yaw);

  model->bone[bone].mvr[ 0]=cosY*cosP;
  model->bone[bone].mvr[ 1]=-sinY*cosP;
  model->bone[bone].mvr[ 2]=sinP;
  model->bone[bone].mvr[ 4]=cosY*sinR*sinP+sinY*cosR;
  model->bone[bone].mvr[ 5]=-sinY*sinR*sinP+cosY*cosR;
  model->bone[bone].mvr[ 6]=-sinR*cosP;
  model->bone[bone].mvr[ 8]=-cosY*cosR*sinP+sinY*sinR;
  model->bone[bone].mvr[ 9]=sinY*cosR*sinP+cosY*sinR;
  model->bone[bone].mvr[10]=cosR*cosP;

}

void handle_mouse_button( int button, int state, int x, int y ) {
  /* state == 0 : pressed */
  /* state == 1 : released */
  if (state == 0) {
    printf("Pressed at (%d,%d).\n", x, y);
    g_mouse_pressed_pos_x = x;
    g_mouse_pressed_pos_y = y;
    g_mouse_dragging = 1;
  } else {
    printf("Release at (%d,%d).\n", x, y);
    g_mouse_dragging = 0;
    cam_phi += cam_dphi;
    cam_theta += cam_dtheta;
    cam_dphi = 0;
    cam_dtheta = 0;
  }
  if (state)
    return;
  switch (button) {
  case 1:
    break;
  case 2:
    old_x = x;
    old_y = y;
    break;
  case 3:
    old_x = x;
    old_y = y;
    break;
  case 4: /* rotate wheel upward */
    zoomRatio += 0.1;
    break;
  case 5: /* rotate wheel downward */
    zoomRatio -= 0.1;
    break;
  }
}

/*
 * handle_mouse_motion
 */
void handle_mouse_motion(int button, int x, int y ) {
  if (g_mouse_dragging) {
    const int dx = x - g_mouse_pressed_pos_x;
    const int dy = y - g_mouse_pressed_pos_y;
    cam_dphi = -(double)dx/200;
    cam_dtheta = -(double)dy/200;
  }
  switch (button) {
  case 1:
    break;
  case 2:
    break;
  case 3:
    break;
  }
}

void simplerender(GLuint *m_vaoID) {
  glPointSize(20);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //    glBindVertexArray(m_vaoID[0]);      // select first VAO
  //    glDrawArrays(GL_TRIANGLES, 0, 3);   // draw first object

  //    glBindVertexArray(m_vaoID[1]);      // select second VAO
  //    glVertexAttrib3f((GLuint)1, 0.0, 1.0, 1.0); // set constant color attribute
  //    glDrawArrays(GL_TRIANGLES, 0, 3);   // draw second object

  //    glBindVertexArray(m_vaoID[2]);      // select second VAO
  //    glVertexAttrib3f((GLuint)1, 1.0, 0.0, 1.0); // set constant color attribute
  //    glDrawArrays(GL_QUADS, 0, 4);   // draw second object
  //    GLenum err = glGetError();
  //    switch (err) {
  //        case GL_INVALID_ENUM: printf("GL_INVALID_ENUM\n"); break;
  //        case GL_INVALID_VALUE: printf("GL_INVALID_VALUE\n"); break;
  //        case GL_INVALID_OPERATION: printf("GL_INVALID_OPERATION\n"); break;
  //        default: printf("OK.\n"); break;
  //    }
  //    glBindVertexArray(0);


  //GLenum err;

  glColor3f(1,0,0);


  //    glEnableClientState(GL_VERTEX_ARRAY);
  //    glVertexPointer(3, GL_FLOAT, 0, vert3);
  //
  //
  //    glDrawArrays(GL_POINTS, 0, 1);
  //    err = glGetError();
  //        switch (err) {
  //        case GL_INVALID_ENUM: printf("GL_INVALID_ENUM\n"); break;
  //        case GL_INVALID_VALUE: printf("GL_INVALID_VALUE\n"); break;
  //        case GL_INVALID_OPERATION: printf("GL_INVALID_OPERATION\n"); break;
  //        default: printf("OK.\n"); break;
  //    }
  //
  //
  //    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
}

void EventLoop(Display* display, Window window, Atom wmDeleteMessage,
               PYMRS rs) {
  int exertExternalForce = 0;
  while (1) {
    static GLint iFrames = 0;
    static GLfloat fps = 0.0f, DeltaT;
    static char cBuffer[64];
    struct timeval tv;
    // Update timer
    gettimeofday(&tv, NULL);
    etime = (double)tv.tv_sec + tv.tv_usec / 1000000.0f;
    dt = etime - t0;
    t0 = etime;

    while (XPending(display) > 0) {
      XEvent event;
      int code;
      XNextEvent( display, &event );
      if (event.type == ClientMessage &&
	  event.xclient.data.l[0] == wmDeleteMessage) {
	printf("Shutting down now!!!\n");
	return;
      }
      switch (event.type) {
      case Expose:
	break;
      case ConfigureNotify:
	RENDER_WIDTH  = event.xconfigure.width;
	RENDER_HEIGHT = event.xconfigure.height;
	change_size(RENDER_WIDTH, RENDER_HEIGHT);
	break;
      case KeyPress:
	code = XLookupKeysym( &event.xkey, 0);

	if ((code == XK_Escape)||(code == XK_q))
	  return;
	if(code == XK_a)
	  exertExternalForce = 1;
	if(code == XK_r)
	  pthread_cond_signal(&count_threshold_cv);
	if(code == XK_w)
	  dCam = tStep;
	if(code == XK_s)
	  dCam = -tStep;
	if(code == XK_d)
	  lCam = tStep;
	if(code == XK_a)
	  lCam = -tStep;
	if(code == XK_Up)
	  model->bone[0].mvr[13]+=0.2;
	if(code == XK_Down)
	  model->bone[0].mvr[13]-=0.2;
	if(code == XK_Left)
	  model->bone[0].mvr[12]-=0.2;
	if(code == XK_Right)
	  model->bone[0].mvr[12]+=0.2;
	if(code == XK_f)
	  print_fps = !print_fps;
	if(code == XK_t)
	  transparency=!transparency;
	if(code == XK_b)
	  skeleton=!skeleton;
	if(code == XK_l)
	  if(model->num_poses>1) {
	    animation=!animation;
	    pose = 1;
	    old_pose = 0;
	    t1 = 0.0f;
	  }
	if(code == XK_p) {

	  //Dump quaternions to STDOUT and add them as a pose

	  GLuint i;

	  model->num_poses++;
	  model->pose=(POSE_PTR)realloc(model->pose,
					sizeof(POSE)*model->num_poses);
	  model->pose[model->num_poses-1].Q=
	    (VECTOR4D_PTR)malloc(sizeof(VECTOR4D)*model->num_bones);
	  model->pose[model->num_poses-1].t=2.0f;

	  printf("Dumping quaternions\n");
	  for(i=0; i<model->num_bones; i++) {
	    mv2quat(&model->bone[i].Q, model->bone[i].mvr);
	    model->pose[model->num_poses-1].Q[i].x=model->bone[i].Q.x;
	    model->pose[model->num_poses-1].Q[i].y=model->bone[i].Q.y;
	    model->pose[model->num_poses-1].Q[i].z=model->bone[i].Q.z;
	    model->pose[model->num_poses-1].Q[i].w=model->bone[i].Q.w;
	    printf("%3.6f %3.6f %3.6f %3.6f\n",
		   model->bone[i].Q.x,
		   model->bone[i].Q.y,
		   model->bone[i].Q.z,
		   model->bone[i].Q.w);
	  }
	}
	if (code == XK_r) {
	  wireframe = !wireframe;
	  if (wireframe)
	    glPolygonMode(GL_FRONT, GL_LINE);
	  else
	    glPolygonMode(GL_FRONT, GL_FILL);
	}
	if (code == XK_Shift_L) {
	  selection=GL_TRUE;
	  XDefineCursor(display, window, picking);
	}
	if (code == XK_bracketleft) {
	  if( (model->pose[model->num_poses-1].t -= t_inc) < t_inc )
	    model->pose[model->num_poses-1].t=t_inc;
	}
	if (code == XK_bracketright)
	  model->pose[model->num_poses-1].t += t_inc;
	break;
      case KeyRelease:
	code = XLookupKeysym( &event.xkey, 0);
	if (code == XK_Shift_L) {
	  selection=GL_FALSE;
	  XUndefineCursor(display, window);
	}
	break;
      case ButtonPress:
	handle_mouse_button(event.xbutton.button,
			    0,
			    event.xbutton.x,
			    event.xbutton.y);
	break;
      case ButtonRelease:
	handle_mouse_button(event.xbutton.button,
			    1,
			    event.xbutton.x,
			    event.xbutton.y);
	break;
      case MotionNotify:
	if (event.xmotion.state & Button1Mask) {
	  handle_mouse_motion(1,
			      event.xmotion.x,
			      event.xmotion.y);
	  break;
	}
	if (event.xmotion.state & Button2Mask) {
	  handle_mouse_motion(2,
			      event.xmotion.x,
			      event.xmotion.y);
	  break;
	}
	if (event.xmotion.state & Button3Mask)
	  handle_mouse_motion(3,
			      event.xmotion.x,
			      event.xmotion.y);
	break;
      }
    }
    pym_render_config_t rc;
    rc.cam_r	 = cam_r;
    rc.cam_phi	 = cam_phi + cam_dphi;
    rc.cam_theta = cam_theta + cam_dtheta;
    rc.vpx	 = 0;
    rc.vpy	 = 0;
    rc.vpw	 = RENDER_WIDTH;
    rc.vph	 = RENDER_HEIGHT;

    pthread_mutex_lock(&main_mutex); {
      PymRsRender(rs, &rc);
    } pthread_mutex_unlock(&main_mutex);
    //simplerender(m_vaoID);

    iFrames++;
    DeltaT = (GLfloat)(etime-t2);
    if( DeltaT >= Timed ) {
      fps = (GLfloat)(iFrames)/DeltaT;
      fps_count++;
      fps_mean = ((fps_count - 1.0f) * fps_mean + fps ) / fps_count;
      iFrames = 0;
      t2 = etime;
    }
    if (print_fps) {
      sprintf(cBuffer, "FPS: %.1f", fps);
      glColor3f(1.0f, 1.0f, 1.0f);
      glPushAttrib(GL_LIST_BIT);
      glListBase(fps_font - ' ');
      glWindowPos2i(0,2);
      glCallLists(strlen(cBuffer), GL_UNSIGNED_BYTE, cBuffer);
      glPopAttrib();
    }
    lCam = 0.0f;
    vCam = 0.0f;
    dCam = 0.0f;
  
    glXSwapBuffers( display, window );
    if (exertExternalForce) {
      pthread_mutex_lock(&main_mutex);
      {
	rs->phyCon.trunkExternalForce[0] = 5000;
	rs->phyCon.trunkExternalForce[1] = 4000;
      }
      pthread_mutex_unlock(&main_mutex);
      exertExternalForce = 0;
    }
  }
}

/* A simple function that will read a file into an allocated char pointer buffer */
char* filetobuf(char *file)
{
  FILE *fptr;
  long length;
  char *buf;

  fptr = fopen(file, "r"); /* Open file for reading */
  if (!fptr) /* Return NULL on failure */
    return NULL;
  fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
  length = ftell(fptr); /* Find out how many bytes into the file we are */
  buf = (char*)malloc(length+1); /* Allocate a buffer for the entire length of the file and a null terminator */
  fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
  fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
  fclose(fptr); /* Close the file */
  buf[length] = 0; /* Null terminator */

  return buf; /* Return the buffer */
}
void preparedata(void)
{
  assert(0);
  //int i; /* Simple iterator */
  //GLuint vao, vbo[2]; /* Create handles for our Vertex Array Object and two Vertex Buffer Objects */

  ///* We're going to create a simple diamond made from lines */
  //const GLfloat diamond[4][2] = {
  //{  0.0,  1.0  }, /* Top point */
  //{  1.0,  0.0  }, /* Right point */
  //{  0.0, -1.0  }, /* Bottom point */
  //{ -1.0,  0.0  } }; /* Left point */

  //const GLfloat colors[4][3] = {
  //{  1.0,  0.0,  0.0  }, /* Red */
  //{  0.0,  1.0,  0.0  }, /* Green */
  //{  0.0,  0.0,  1.0  }, /* Blue */
  //{  1.0,  1.0,  1.0  } }; /* White */

  /* These pointers will receive the contents of our shader source code files */
  GLchar *vertexsource, *fragmentsource;

  /* These are handles used to reference the shaders */
  GLuint vertexshader, fragmentshader;

  /* This is a handle to the shader program */
  GLuint shaderprogram;

  ///* Allocate and assign a Vertex Array Object to our handle */
  //glGenVertexArrays(1, &vao);

  ///* Bind our Vertex Array Object as the current used object */
  //glBindVertexArray(vao);

  ///* Allocate and assign two Vertex Buffer Objects to our handle */
  //glGenBuffers(2, vbo);

  ///* Bind our first VBO as being the active buffer and storing vertex attributes (coordinates) */
  //glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

  ///* Copy the vertex data from diamond to our buffer */
  ///* 8 * sizeof(GLfloat) is the size of the diamond array, since it contains 8 GLfloat values */
  //glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), diamond, GL_STATIC_DRAW);

  ///* Specify that our coordinate data is going into attribute index 0, and contains two floats per vertex */
  //glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);

  ///* Enable attribute index 0 as being used */
  //glEnableVertexAttribArray(0);

  ///* Bind our second VBO as being the active buffer and storing vertex attributes (colors) */
  //glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

  ///* Copy the color data from colors to our buffer */
  ///* 12 * sizeof(GLfloat) is the size of the colors array, since it contains 12 GLfloat values */
  //glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), colors, GL_STATIC_DRAW);

  ///* Specify that our color data is going into attribute index 1, and contains three floats per vertex */
  //glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  ///* Enable attribute index 1 as being used */
  //glEnableVertexAttribArray(1);

  /* Read our shaders into the appropriate buffers */
  vertexsource = filetobuf("minimal.vert");
  fragmentsource = filetobuf("minimal.frag");
  assert(vertexsource && fragmentsource);

  /* Assign our handles a "name" to new shader objects */
  vertexshader = glCreateShader(GL_VERTEX_SHADER);
  fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);

  /* Associate the source code buffers with each handle */
  glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);
  glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);

  /* Compile our shader objects */
  glCompileShader(vertexshader);
  glCompileShader(fragmentshader);

  /* Assign our program handle a "name" */
  shaderprogram = glCreateProgram();

  /* Attach our shaders to our program */
  glAttachShader(shaderprogram, vertexshader);
  glAttachShader(shaderprogram, fragmentshader);

  /* Bind attribute index 0 (coordinates) to in_Position and attribute index 1 (color) to in_Color */
  glBindAttribLocation(shaderprogram, 0, "in_Position");
  glBindAttribLocation(shaderprogram, 1, "in_Color");

  /* Link our program, and set it as being actively used */
  glLinkProgram(shaderprogram);
  glUseProgram(shaderprogram);

}

static int pym_init_xwindow(Display **_display, Window *_window,
			    Atom *_wmDeleteMessage,
			    XFontStruct **_font) {
  int             ret=0;
  char*           display_name;
  Display*        display;
  Window          window;
  int		  scrnum;
  Window	  root;
  int		  x, y;
  GLXContext      context;
  XVisualInfo*    vinfo;
  XSetWindowAttributes	winattrs;
  XFontStruct*		font;
  unsigned long		winmask;
  static int		attributes[] = {
    GLX_RGBA,
    GLX_DOUBLEBUFFER,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    0
  };

  /* Open the connection to the X server */
  display_name = getenv("DISPLAY");
  display = XOpenDisplay(display_name);
  if (!display) {
    printf("Error: couldn't open display %s\n", display_name);
    return 1;
  }
  /* Find the proper visual */
  scrnum = DefaultScreen( display );
  root = RootWindow( display, scrnum );
  x = 0;
  y = 0;
  //    width = DisplayWidth( display, scrnum )*3/4;
  //    height = DisplayHeight( display, scrnum )*3/4;
  width = 700;
  height = 700;
  vinfo = glXChooseVisual(display, scrnum, attributes);
  if (!vinfo) {
    printf("Error: couldn't get an RGB, Double-buffered visual\n");
    return 1;
  }
  /* Create the window */
  winattrs.event_mask =	ExposureMask | StructureNotifyMask |
    ButtonPressMask | ButtonReleaseMask |
    PointerMotionMask | KeyPressMask | KeyReleaseMask;
  winattrs.background_pixel = BlackPixel(display, scrnum);
  winattrs.border_pixel = BlackPixel(display, scrnum);
  winattrs.bit_gravity = StaticGravity;
  winattrs.colormap = XCreateColormap( display, root, vinfo->visual, AllocNone);
  winmask = CWBitGravity | CWBackPixel | CWBorderPixel |
    CWColormap | CWEventMask;
  window = XCreateWindow( display, root,
			  x, y, width, height, 0, vinfo->depth, InputOutput,
			  vinfo->visual, winmask, &winattrs );
  if (!window) {
    printf("Error: couldn't create window\n");
    return 1;
  }
  const char *const fullAppName = "Pymuscle Realtime Simulator";
  const char *const taskbarAppName = "Pymuscle";
  XChangeProperty(display, window, XA_WM_NAME, XA_STRING, 8, 0,
		  (unsigned char*)fullAppName, strlen(fullAppName));
  XChangeProperty(display, window, XA_WM_ICON_NAME, XA_STRING, 8, 0,
		  (unsigned char*)taskbarAppName, strlen(taskbarAppName));

  Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wmDeleteMessage, 1);
  /* Create the picking cursor */
  picking = XCreateFontCursor(display, 34);
  /* Create the OpenGL context */
  context = glXCreateContext(display, vinfo, 0, True);

  if (!context) {
    printf("Error: glXCreateContext failed\n");
    return 1;
  }
  XFree(vinfo);
  XMapWindow( display, window );
  glXMakeCurrent( display, window, context );
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    printf("Error: glewInit() failed\n");
    return 1;
  }
  assert(glXChooseFBConfig);
  static int visual_attribs[] = {
    GLX_X_RENDERABLE    , True,
    GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
    GLX_RENDER_TYPE     , GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
    GLX_RED_SIZE        , 8,
    GLX_GREEN_SIZE      , 8,
    GLX_BLUE_SIZE       , 8,
    GLX_ALPHA_SIZE      , 8,
    GLX_DEPTH_SIZE      , 24,
    GLX_STENCIL_SIZE    , 8,
    GLX_DOUBLEBUFFER    , True,
    //GLX_SAMPLE_BUFFERS  , 1,
    //GLX_SAMPLES         , 4,
    None
  };
  printf( "Getting matching framebuffer configs\n" );
  int fbcount;
  GLXFBConfig *fbc = glXChooseFBConfig( display, DefaultScreen( display ),
					visual_attribs, &fbcount );
  if ( !fbc ) {
    printf( "Failed to retrieve a framebuffer config\n" );
    exit(1);
  }
  printf( "Found %d matching FB configs.\n", fbcount );
  // Pick the FB config/visual with the most samples per pixel
  printf( "Getting XVisualInfos\n" );
  int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
  int i = 0;
  for ( i = 0; i < fbcount; i++ ) {
    XVisualInfo *vi = glXGetVisualFromFBConfig( display, fbc[i] );
    if ( vi ) {
      int samp_buf, samples;
      glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
      glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLES       , &samples  );

      printf( "  Matching fbconfig %d, visual ID 0x%2x: SAMPLE_BUFFERS = %d,"
	      " SAMPLES = %d\n",
	      i, (int)(vi -> visualid), samp_buf, samples );

      if ( (best_fbc < 0) || (samp_buf && (samples > best_num_samp)) )
	best_fbc = i, best_num_samp = samples;
      if ( (worst_fbc < 0) || !samp_buf || (samples < worst_num_samp) )
	worst_fbc = i, worst_num_samp = samples;
    }
    XFree( vi );
  }
  // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
  XFree( fbc );
  //  GLXContext ctx = 0;
  /* Setup fonts */
  font = XLoadQueryFont(display,
			"-*-courier-bold-r-normal--14-*-*-*-*-*-*-*");
  fps_font = glGenLists(96);
  glXUseXFont(font->fid, ' ', 96, fps_font);
  *_display = display;
  *_window = window;
  *_wmDeleteMessage = wmDeleteMessage;
  *_font = font;
  return ret;
}

static void pym_init_opengl() {
  glClearColor(0.8, 0.8, 0.8, 0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_TEXTURE_2D);
  //glCullFace(GL_BACK);
  glEnable(GL_NORMALIZE);
  glEnable(GL_ALPHA_TEST);
  glShadeModel(GL_FLAT);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45, 1.0, 0.1, 100.0);
}

int main(int argc, char *argv[]) {
  Display*      display;
  Window        window;
  Atom          wmDeleteMessage;
  XFontStruct*	font;
  PYMRS rs = PymRsInitContext(argc, argv);
  if (!rs) {
    /* Realtime simulator context acquisition failed. */
    return -1;
  }
  pym_init_xwindow(&display, &window, &wmDeleteMessage, &font);
  pym_init_opengl();
  PymRsInitRender();
  pthread_t thPhysics;
  pthread_attr_t attr;
  /* Initialize mutex and condition variable objects */
  pthread_mutex_init(&count_mutex, NULL);
  pthread_mutex_init(&main_mutex, NULL);
  pthread_cond_init (&count_threshold_cv, NULL);
  pthread_cond_init (&physics_thread_finished, NULL);
  /* For portability, explicitly create threads in a joinable state */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&thPhysics, &attr, PhysicsThreadMain, (void *)rs);
  //pthread_create(&threads[1], &attr, inc_count, (void *)t2);
  //pthread_create(&threads[2], &attr, inc_count, (void *)t3);

  EventLoop( display, window, wmDeleteMessage, rs );

  pthread_mutex_lock(&main_mutex);
  rs->phyCon.stop = 1;
  pthread_mutex_unlock(&main_mutex);

  /* Wait for the physics thread to complete */
  puts("Wait for the physics thread to complete...\n");
  pthread_cond_signal(&count_threshold_cv);
  //pthread_join(thPhysics, NULL);

  /* TODO: Force-canceling physics thread.
   *       Is there way to exit gracefully? */
  pthread_cancel(thPhysics);

  /* Print average fps */
  printf("\nAverage FPS: %.1f\n", fps_mean);

  /* Clean up */
  PymRsDestroyContext(rs);
  rs = 0;
  PymRsDestroyRender();
  if(model)
    deleteModel(model);
  if(texture_id)
    glDeleteTextures(1, &texture_id);
  XFreeFont(display, font);
  //glXDestroyContext( display, context );
  //  glXDestroyContext( display, ctx );
  XDestroyWindow( display, window );
  XCloseDisplay( display );

  /* pthread Clean up and exit */
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&count_mutex);
  pthread_mutex_destroy(&main_mutex);
  pthread_cond_destroy(&count_threshold_cv);
  pthread_cond_destroy(&physics_thread_finished);
  //pthread_exit(ret);
  return 0;
}
