#include "PymRsPch.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "PrsGraphCapi.h"

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

#include "pymrscore.h"
#include "pymrsrender.h"

#ifndef M_PI
#define M_PI 3.14
#endif

GLuint		m_vaoID[5];     /* two vertex array objects,
				   one for each drawn object */
GLuint		m_vboID[3];     // three VBOs
GLuint		gndTex;
// Z values will be rendered to this texture when using fboId framebuffer
GLuint		depthTextureId;
// Hold id of the framebuffer for light POV rendering
GLuint		fboId;
// Use to activate/disable shadowShader
GLhandleARB	shadowShaderId;
GLint		shadowMapUniform;
GLint		projMatUniform;
GLint		modelViewMatUniform;
GLint		normalMatUniform;
const int	n_face	  = 45; // Circle shaped ground face number

static inline double Norm3(const double *const v) {
  return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

static void pym_strict_checK_gl() {
  GLenum gl_error = glGetError();
  if( gl_error != GL_NO_ERROR ) {
    fprintf( stderr, "ARAN: OpenGL error: %s\n",
	     gluErrorString(gl_error) );
    abort();
  }
}

GLuint InitTriangleVao() {
  // First simple object
  float vert[9]; // vertex array
  float col[9];  // color array

  vert[0] =-0.3; vert[1] = 0.5; vert[2] =-1.0;
  vert[3] =-0.8; vert[4] =-0.5; vert[5] =-1.0;
  vert[6] = 0.2; vert[7] =-0.5; vert[8]= -1.0;

  col[0] = 1.0; col[1] = 1.0; col[2] = 1.0;
  col[3] = 0.0; col[4] = 1.0; col[5] = 0.0;
  col[6] = 0.0; col[7] = 0.0; col[8] = 1.0;

  GLuint vaoId = 0;
  assert(glGenVertexArrays);
  glGenVertexArrays(1, &vaoId);

  // First VAO setup
  glBindVertexArray(vaoId);

  // 2 VBOs (Vertex Buffer Object) for the first VAO
  GLuint vboId[2] = {0,};
  glGenBuffers(2, vboId);

  glBindBuffer(GL_ARRAY_BUFFER, vboId[0]);
  glBufferData(GL_ARRAY_BUFFER, 9*sizeof(GLfloat), vert, GL_STATIC_DRAW);
  glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, vboId[1]);
  glBufferData(GL_ARRAY_BUFFER, 9*sizeof(GLfloat), col, GL_STATIC_DRAW);
  glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);

  return vaoId;
}

GLuint InitQuadVao() {
  float vert2[9]; // vertex array

  vert2[0] =-0.2; vert2[1] = 0.5; vert2[2] =-1.0;
  vert2[3] = 0.3; vert2[4] =-0.5; vert2[5] =-1.0;
  vert2[6] = 0.8; vert2[7] = 0.5; vert2[8]= -1.0;

  GLuint vaoId = 0;
  glGenVertexArrays(1, &vaoId);
  glBindVertexArray(vaoId);

  // 1 VBO for the second VAO
  GLuint vboId = 0;
  glGenBuffers(1, &vboId);

  glBindBuffer(GL_ARRAY_BUFFER, vboId);
  glBufferData(GL_ARRAY_BUFFER, 9*sizeof(GLfloat), vert2, GL_STATIC_DRAW);
  glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  return vaoId;
}

GLuint InitQuad2Vao() {
  GLuint vaoId = 0;
  glGenVertexArrays(1, &vaoId);
  glBindVertexArray(vaoId);

  // Third simple object
  GLfloat vert3[] = {  15.3f,  5.3f,  0.0f,
		       -5.3f,  5.3f,  0.0f,
		       -5.3f, -5.3f,  0.0f,
                       5.3f, -5.3f,  0.0f  };
  // 1 VBO for the third VAO
  GLuint vboId = 0;
  glGenBuffers(1, &vboId);

  glBindBuffer(GL_ARRAY_BUFFER, vboId);
  glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), vert3, GL_STATIC_DRAW);
  glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
  return vaoId;
}

GLuint InitUnitBoxVao() {
  GLuint vaoId = 0;
  glGenVertexArrays(1, &vaoId);
  glBindVertexArray(vaoId);

  GLfloat vert4[] = { /* Face which has +X normals (x= 0.5) */
    0.5f,  0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,
    /* Face which has -X normals (x=-0.5) */
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    /* Face which has +Y normals (y= 0.5) */
    0.5f, 0.5f,  0.5f,
    0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f,  0.5f,
    /* Face which has -Y normals (y=-0.5) */
    0.5f, -0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,
    -0.5f, -0.5f,-0.5f,
    0.5f, -0.5f,-0.5f,
    /* Face which has +Z normals (z= 0.5) */
    0.5f,  0.5f, 0.5f,
    -0.5f,  0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
    /* Face which has -Z normals (z=-0.5) */
    0.5f,  0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f  };
  GLfloat nor4[] = {
    1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0,
    -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0,
    0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0,
    0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0,
    0,0,1,0,0,1,0,0,1,0,0,1,
    0,0,-1,0,0,-1,0,0,-1,0,0,-1,
  };
  GLfloat col4[3*4*6];
  int i;
  FOR_0(i, 3*4*6) {
    if (i%3 == 0)
      col4[i] = 1;
  }

  GLuint vboId[3] = {0,};
  glGenBuffers(3, vboId);

  glBindBuffer(GL_ARRAY_BUFFER, vboId[0]);
  glBufferData(GL_ARRAY_BUFFER, 3*4*6*sizeof(GLfloat), vert4, GL_STATIC_DRAW);
  glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, vboId[1]);
  glBufferData(GL_ARRAY_BUFFER, 3*4*6*sizeof(GLfloat), col4, GL_STATIC_DRAW);
  glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, vboId[2]);
  glBufferData(GL_ARRAY_BUFFER, 3*4*6*sizeof(GLfloat), nor4, GL_STATIC_DRAW);
  glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(2);
  return vaoId;
}

GLuint InitCircleVao() {
  GLuint vaoId = 0;
  glGenVertexArrays(1, &vaoId);
  glBindVertexArray(vaoId);

  /* center point(1) + vertices around circumference(n_face) + final point(1) */
  const int n_vert_circle = (1+n_face+1)*3;
  GLfloat vert5[n_vert_circle];
  memset(vert5, 0, sizeof(vert5));
  float circle_radius = 10;
  int i;
  for (i=0;i<=n_face;++i) { /* note that '<=' for final point */
    const double rad = 2*M_PI/n_face*i;
    vert5[3 + i*3 + 0] = circle_radius*cos(rad);
    vert5[3 + i*3 + 1] = circle_radius*sin(rad);
  }

  // 1 VBO for the third VAO
  GLuint vboId = 0;
  glGenBuffers(1, &vboId);

  glBindBuffer(GL_ARRAY_BUFFER, vboId);
  glBufferData(GL_ARRAY_BUFFER, n_vert_circle*sizeof(GLfloat),
	       vert5, GL_STATIC_DRAW);
  glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
  return vaoId;
}

void InitVertexArrayObjects(GLuint *m_vaoID)
{
  m_vaoID[0] = InitTriangleVao();
  m_vaoID[1] = InitQuadVao();
  m_vaoID[2] = InitQuad2Vao();
  m_vaoID[3] = InitUnitBoxVao();
  m_vaoID[4] = InitCircleVao();
  glBindVertexArray(0);
}

// Loading shader function
GLhandleARB loadShader(char* filename, unsigned int type)
{
  FILE *pfile;
  GLhandleARB handle;
  const GLcharARB* files[1];

  // shader Compilation variable
  GLint result;				// Compilation code result
  GLint errorLoglength ;
  char* errorLogText;
  GLsizei actualErrorLogLength;

  char buffer[400000];
  memset(buffer,0,400000);

  // This will raise a warning on MS compiler
  printf("Loading shader %s...\n", filename);
  pfile = fopen(filename, "rb");
  if(!pfile)
    {
      printf("Sorry, can't open file: '%s'.\n", filename);
      exit(-1);
    }

  fread(buffer,sizeof(char),400000,pfile);
  //printf("%s\n",buffer);


  fclose(pfile);

  handle = glCreateShaderObjectARB(type);
  if (!handle)
    {
      //We have failed creating the vertex shader object.
      printf("Failed creating vertex shader object from file: %s.",filename);
      exit(-1);
    }

  files[0] = (const GLcharARB*)buffer;
  glShaderSourceARB(
		    handle, //The handle to our shader
		    1, //The number of files.
		    files, //An array of const char * data, which represents the source code of theshaders
		    NULL);

  glCompileShaderARB(handle);

  //Compilation checking.
  glGetObjectParameterivARB(handle, GL_OBJECT_COMPILE_STATUS_ARB, &result);
  //Attempt to get the length of our error and warning log.
  glGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &errorLoglength);
  if (errorLoglength) {
    //Create a buffer to read compilation error message
    errorLogText = (char *)malloc(sizeof(char) * errorLoglength);

    //Used to get the final length of the log.
    glGetInfoLogARB(handle, errorLoglength, &actualErrorLogLength, errorLogText);
    // Display errors.
    printf("%s\n",errorLogText);
    // Free the buffer malloced earlier
    free(errorLogText);
    // In case of error occurred:
    if (!result) {
      //We failed to compile.
      printf("Shader '%s' failed compilation.\n", filename);
      exit(-2);
    }
  }
  return handle;
}

void CheckLinkError(GLhandleARB obj)
{
  int infologLength = 0;
  int charsWritten  = 0;
  char *infoLog;
  GLint result;
  glGetObjectParameterivARB(obj, GL_OBJECT_LINK_STATUS_ARB, &result);
  if (!result) {
    glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
			      &infologLength);
    if (infologLength > 0) {
      infoLog = (char *)malloc(infologLength);
      glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
      printf("%s\n",infoLog);
      free(infoLog);
    }
    exit(-2);
  }
}

void loadShadowShader()
{
  GLhandleARB vertexShaderHandle;
  GLhandleARB fragmentShaderHandle;

  vertexShaderHandle   = loadShader("Shadow.vert", GL_VERTEX_SHADER_ARB);
  printf("Vertex   shader handle : %d\n", vertexShaderHandle);
  fragmentShaderHandle = loadShader("Shadow.frag", GL_FRAGMENT_SHADER_ARB);
  printf("Fragment shader handle : %d\n", fragmentShaderHandle);
  shadowShaderId = glCreateProgramObjectARB();
  printf("Program handle         : %d\n", shadowShaderId);
  glAttachObjectARB(shadowShaderId, vertexShaderHandle);
  glAttachObjectARB(shadowShaderId, fragmentShaderHandle);

  glBindAttribLocationARB(shadowShaderId, 0, "in_Position");
  glBindAttribLocationARB(shadowShaderId, 1, "in_Color");
  glBindAttribLocationARB(shadowShaderId, 2, "in_Normal");

  glLinkProgramARB(shadowShaderId);
  CheckLinkError(shadowShaderId);
  glUseProgram(shadowShaderId);

  printf("Shadow shader loaded successfully.\n");

  shadowMapUniform = glGetUniformLocation(shadowShaderId,
					  "ShadowMap");
  projMatUniform   = glGetUniformLocation(shadowShaderId,
					  "projection_matrix");
  modelViewMatUniform = glGetUniformLocation(shadowShaderId,
					     "modelview_matrix");
  normalMatUniform = glGetUniformLocation(shadowShaderId,
					  "normal_matrix");
  assert(shadowMapUniform >= 0);
  assert(projMatUniform >= 0);
  assert(modelViewMatUniform >= 0);
  //assert(normalMatUniform >= 0);
}

void generateShadowFBO() {
  /*
    int shadowMapWidth = RENDER_WIDTH * SHADOW_MAP_RATIO;
    int shadowMapHeight = RENDER_HEIGHT * SHADOW_MAP_RATIO;
  */
  int shadowMapWidth = 1024;
  int shadowMapHeight = 1024;

  //GLfloat borderColor[4] = {0,0,0,0};

  GLenum FBOstatus;

  // Try to use a texture depth component
  glGenTextures(1, &depthTextureId);
  glBindTexture(GL_TEXTURE_2D, depthTextureId);

  // GL_LINEAR does not make sense for depth texture. However, next tutorial shows usage of GL_LINEAR and PCF
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Remove artefact on the edges of the shadowmap
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

  //glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );
  // No need to force GL_DEPTH_COMPONENT24, drivers usually give you the max precision if available
  glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  // create a framebuffer object
  glGenFramebuffersEXT(1, &fboId);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);

  // Instruct openGL that we won't bind a color texture with the currently binded FBO
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  // attach the texture to FBO depth attachment point
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D, depthTextureId, 0);

  // check FBO status
  FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  if(FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
    printf("GL_FRAMEBUFFER_COMPLETE_EXT failed, CANNOT use FBO\n");

  // switch back to window-system-provided framebuffer
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void setupMatrices(float position_x, float position_y, float position_z,
                   float lookAt_x,   float lookAt_y,   float lookAt_z) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  //gluPerspective(45, (double)RENDER_WIDTH/RENDER_HEIGHT, 1, 100);
  glOrtho(-3, 3, -3, 3, 1e-2, 1e5);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  gluLookAt(position_x, position_y, position_z,
	    lookAt_x,   lookAt_y,   lookAt_z,
	    0,0,1);


  GLfloat mat[16];
  glGetFloatv(GL_PROJECTION_MATRIX, mat);
  glUniformMatrix4fvARB(projMatUniform, 1, 0, mat);
  glGetFloatv(GL_MODELVIEW_MATRIX, mat);
  glUniformMatrix4fvARB(modelViewMatUniform, 1, 0, mat);
}

void setTextureMatrix(void) {
  static double modelView[16];
  static double projection[16];

  // This is matrix transform every coordinate x,y,z
  // x = x* 0.5 + 0.5
  // y = y* 0.5 + 0.5
  // z = z* 0.5 + 0.5
  // Moving from unit cube [-1,1] to [0,1]
  const GLdouble bias[16] = {
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0};

  // Grab modelview and transformation matrices
  glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);


  glMatrixMode(GL_TEXTURE);
  glActiveTextureARB(GL_TEXTURE7);

  glLoadIdentity();
  glLoadMatrixd(bias);

  // concatating all matrice into one.
  glMultMatrixd (projection);
  glMultMatrixd (modelView);

  // Go back to normal matrix mode
  glMatrixMode(GL_MODELVIEW);
}

void startXform(double W[4][4]) {
  const GLdouble *const Wa = (const GLdouble *const)W;
  glPushMatrix();
  glMultTransposeMatrixd(Wa);

  glMatrixMode(GL_TEXTURE);
  glActiveTextureARB(GL_TEXTURE7);
  glPushMatrix();
  glMultTransposeMatrixd(Wa);
}
void endXform() {
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void startTranslate(float x, float y, float z) {
  glPushMatrix();
  glTranslatef(x,y,z);

  glMatrixMode(GL_TEXTURE);
  glActiveTextureARB(GL_TEXTURE7);
  glPushMatrix();
  glTranslatef(x,y,z);
}

void endTranslate() {
  endXform();
}

void SetUniforms() {
  GLfloat mat44[4][4];
  /* Projection matrix to vertex shader */
  glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *)mat44);
  glUniformMatrix4fvARB(projMatUniform, 1, GL_FALSE, (GLfloat *)mat44);
  /* Modelview matrix to vertex shader */
  glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)mat44);
  glUniformMatrix4fvARB(modelViewMatUniform, 1, GL_FALSE, (GLfloat *)mat44);
  /* Normal matrix (transpose of inverse of modelview matrix)
   * to vertex shader */
  float mat33[3][3];
  int i, j;
  FOR_0(i, 3) {
    FOR_0(j, 3) {
      mat33[i][j] = mat44[i][j];
    }
  }
  float mat33Inv[3][3];
  Invert3x3Matrixf(mat33Inv, mat33);
  glUniformMatrix3fvARB(normalMatUniform, 1, GL_TRUE, (GLfloat *)mat33Inv);
}

void DrawBox_chi(const double *chi,
		 const double *const boxSize, int wf) {
  double W[4][4];
  GetWFrom6Dof(W, chi); /* chi only has translation and rotation */
  int j, k;
  /* Scaling added w.r.t. boxSize */
  FOR_0(j, 4)
    FOR_0(k, 3)
    W[j][k] *= boxSize[k];

  /* TODO: Remove scaling factor from the transform matrix W */
  startXform(W);
  glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
  if (wf == 1) {
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else if (wf == 0) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  } else {
    assert("What the...");
  }

  //SetUniforms();

  glBindVertexArray(m_vaoID[3]);      // select second VAO

  /* // set constant color attribute */
  /* glVertexAttrib3f((GLuint)1, 1.0, 1.0, 0.0); */
  /* // set constant color attribute */
  /* glVertexAttrib3f((GLuint)1, 1.0, 1.0, 0.0);  */
  /* glNormalPointer(GL_FLOAT, 0, (void*)(sizeof(float)*3*4*6)); */
  /* glVertexPointer(3, GL_FLOAT, 0, 0); */

  glDrawArrays(GL_QUADS, 0, 4*6);   // draw second object

  //glutWireCube(1.0);

  glPopAttrib();
  endXform();
}

void DrawBox_pq(const double *p, const double *q,
                const double *const boxSize, int wf) {
  double chi[6];
  assert(p);
  memcpy(chi + 0, p, sizeof(double)*3);
  if (q)
    memcpy(chi + 3, q, sizeof(double)*3);
  else {
    chi[3] = 0;
    chi[4] = 0;
    chi[5] = 0;
  }
  DrawBox_chi(chi, boxSize, wf);
}

static void pym_draw_square_ground() {
  float colorBlue[] = { 0.2f, 0.5f, 1.0f, 1.0f };
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorBlue);
  glBegin(GL_QUADS);
  int i, j;
  const int tile_count = 100;
  const double tile_size = 10.0;
  const double tile_unit_size = tile_size/tile_count;
  for (i=-tile_count/2; i<tile_count/2; ++i) {
    for (j=-tile_count/2; j<tile_count/2; ++j) {
      const double tx = tile_unit_size*i;
      const double ty = tile_unit_size*j;
      glVertex3d(tx, ty, 0);
      glVertex3d(tx+tile_unit_size, ty, 0);
      glVertex3d(tx+tile_unit_size, ty+tile_unit_size, 0);
      glVertex3d(tx, ty+tile_unit_size, 0);
    }
  }
  glEnd();
}


void DrawRb(const pym_rb_named_t *rbn,
            const double *const boxSize, int wf) {
  const double *const p = rbn->p;
  const double *const q = rbn->q;
  glColor3f(0.3,0.75,0.3);
  DrawBox_pq(p, q, boxSize, wf);
  /* Draw external force (disturbance) if exists */
  if (rbn->extForce[0] || rbn->extForce[1] || rbn->extForce[2]) {
    const double norm = Norm3(rbn->extForce);
    double chi[6] = { rbn->p[0],
		      rbn->p[1],
		      rbn->p[2],
		      rbn->q[0],
		      rbn->q[1],
		      rbn->q[2] };
    double W[4][4];
    GetWFrom6Dof(W, chi);
    double extForcePos[3];
    TransformPoint(extForcePos, W, rbn->extForcePos);
    glLineWidth(6.0);
    glDisable(GL_LIGHTING);
    glColor3f(1,0,0);
    glBegin(GL_LINES);
    glVertex3dv(extForcePos);
    glVertex3d(extForcePos[0]-rbn->extForce[0]/1000,
	       extForcePos[1]-rbn->extForce[1]/1000,
	       extForcePos[2]-rbn->extForce[2]/1000);
    glEnd();
    glLineWidth(1.0);
    glEnable(GL_LIGHTING);
  }
}

void DrawRbRef(const pym_rb_named_t *rbn,
               const double *const boxSize, int wf) {
  const double *const chi = rbn->chi_ref;
  assert(chi);
  glColor3f(1, 0, 0);
  //printf("%p -- %lf\n", &chi[0], chi[0]);
  DrawBox_chi(chi, boxSize, wf);
}

void DrawRbContacts(const pym_rb_named_t *rbn) {
  int j;
  FOR_0(j, rbn->nContacts_2) {
    const double *conPos   = rbn->contactsPoints_2a[j];
    const double *conForce = rbn->contactsForce_2[j];
    glColor3f(0,1,0);
    startTranslate(conPos[0], conPos[1], conPos[2]);

    /* TODO: No draw call */

    endTranslate();
    static const double scale = 0.001;
    glBegin(GL_LINES);
    glVertex3f(conPos[0], conPos[1], conPos[2]);
    glVertex3f(conPos[0]+conForce[0]*scale,
	       conPos[1]+conForce[1]*scale,
	       conPos[2]+conForce[2]*scale);
    glEnd();
  }
}

void DrawAxisOnWorldOrigin() {
  glBegin(GL_LINES);
  glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(1,0,0);
  glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,1,0);
  glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,1);
  glEnd();
}

void DrawGround(pym_ground_type_t gndType,
		const GLuint *const m_vaoID) {
  if (gndType == PYM_SQUARE_GROUND)
    glBindVertexArray(m_vaoID[2]);
  else if (gndType == PYM_CIRCLE_GROUND)
    glBindVertexArray(m_vaoID[4]);
  else
    return;
  glPushAttrib(GL_POLYGON_BIT);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // in_Color  (vertex shader)
  glVertexAttrib3f((GLuint)1, 0.3, 0.3, 0.3); 
  // in_Normal (vertex shader)
  glVertexAttrib3f((GLuint)2, 0.0, 0.0, 1.0); 
  pym_strict_checK_gl();

  //SetUniforms();
  pym_strict_checK_gl();

  if (gndType == PYM_SQUARE_GROUND)
    // draw second object
    glDrawArrays(GL_QUADS, 0, 4);
  else if (gndType == PYM_CIRCLE_GROUND)
    // draw second object
    glDrawArrays(GL_TRIANGLE_FAN, 0, (1+n_face+1)*3);
  glPopAttrib();
  pym_strict_checK_gl();
}


void XRotPoint(float pr[3], float p[3], float th) {
  pr[0] = p[0];
  pr[1] = cos(th)*p[1] - sin(th)*p[2];
  pr[2] = sin(th)*p[1] + cos(th)*p[2];
}

void YRotPoint(float pr[3], float p[3], float th) {
  pr[0] = cos(th)*p[0] + sin(th)*p[2];
  pr[1] = p[1];
  pr[2] = -sin(th)*p[0] + cos(th)*p[2];
}


void RenderFootContactStatus
(const pym_physics_thread_context_t *const phyCon) {
  static const double pointBoxSize[3] = { 3e-2, 3e-2, 3e-2 };
  static const double pzero[3] = {0,};
  //static const double q1[3] = {1,1,1};

  int i, j, k;
  FOR_0(i, phyCon->pymCfg->nBody) {
    /* Access data from renderer-accessable area of phyCon */
    const pym_rb_named_t *rbn = &phyCon->renBody[i].b;
    const char *footParts[] = { "soleL", "soleR", "toeL", "toeR" };
    const double pos[4][2] = { { -0.7, 0 },
			       {0.7, 0},
			       {-0.7, 0.2},
			       {0.7, 0.2} };
    FOR_0(k, 4) {
      if (strcmp(rbn->name, footParts[k]) == 0) {
	const double *const boxSize = rbn->boxSize;

	glPushMatrix(); /* Stack A */
	glTranslated(pos[k][0], pos[k][1], 0);
	glRotatef(30, 1, 0, 0);
	glRotatef(45, 0, 1, 0);
	glColor3f(1,1,1);
	DrawBox_pq(pzero, 0, boxSize, 1);
	glColor3f(1,0,0);
	FOR_0(j, phyCon->sd[i].nContacts_2) {
	  const int ci = phyCon->sd[i].contactIndices_2[j];
	  glColor3f(1,0,0);
	  DrawBox_pq(rbn->corners[ ci ], 0, pointBoxSize, 0);
	}
	glPopMatrix(); /* Stack A */
	break;
      }
    }
  }
}

void RenderFootContactFixPosition
(const pym_physics_thread_context_t *const phyCon) {
  static const double pointBoxSize[3] = { 5e-2, 5e-2, 5e-2 };
  int i, j, k;
  FOR_0(i, phyCon->pymCfg->nBody) {
    /* Access data from renderer-accessable area of phyCon */
    const pym_rb_named_t *rbn = &phyCon->renBody[i].b;
    const char *footParts[] = { "soleL", "soleR", "toeL", "toeR" };
    FOR_0(k, 4) {
      if (strcmp(rbn->name, footParts[k]) == 0) {
	glColor3f(1, 0, 0);
	//printf("nContacts_2 = %d\n", phyCon->sd[i].nContacts_2);
	assert(phyCon->sd[i].nContacts_2 >= 0);
	FOR_0(j, phyCon->sd[i].nContacts_2) {
	  DrawBox_pq(phyCon->sd[i].contactsFix_2[j], 0, pointBoxSize, 1);
	}
      }
    }
  }
}

void pym_sphere_to_cartesian(double *c, double r,
			     double phi, double theta) {
  c[0] = r*sin(theta)*sin(phi);
  c[1] = -r*sin(theta)*cos(phi);
  c[2] = r*cos(theta);
}

static void pym_cross3(double *u, const double *const a,
		       const double *const b) {
  u[0] = a[1]*b[2] - a[2]*b[1];
  u[1] = a[2]*b[0] - a[0]*b[2];
  u[2] = a[0]*b[1] - a[1]*b[0];
}

static void pym_up_dir_from_sphere(double *u,
				   const double *const cam_car,
				   double r, double phi, double theta) {
  const double left[3] = { -cos(phi), -sin(phi), 0 };
  const double look_dir[3] = { -cam_car[0]/r,
			       -cam_car[1]/r,
			       -cam_car[2]/r };
  pym_cross3(u, look_dir, left);
}

void PymLookUpLeft(double *look, double *up, double *left,
		   double r, double phi, double theta) {
  double cam_car[3];
  pym_sphere_to_cartesian(cam_car, r, phi, theta);
  left[0] = -cos(phi);
  left[1] = -sin(phi);
  left[2] = 0;
  look[0] = -cam_car[0]/r;
  look[1] = -cam_car[1]/r;
  look[2] = -cam_car[2]/r;
  pym_cross3(up, look, left);
}

static void pym_configure_light() {
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  const float cutoff = 90;
  glLightfv(GL_LIGHT0, GL_SPOT_CUTOFF, &cutoff);
  const float noAmbient[] = {0.5f, 0.5f, 0.5f, 1.0f};
  const float whiteDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
  float lightVector[4] = { 0, 0, 10, 1 };
  float spot_direction[4] = {0, 0, -1};
  glLightfv(GL_LIGHT0, GL_AMBIENT, noAmbient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteDiffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, lightVector);
  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);
}

static void pym_draw_all(pym_physics_thread_context_t *phyCon,
			 int forShadow,
			 GLuint *m_vaoID) {
  glPushMatrix();
  if (!forShadow)
    DrawAxisOnWorldOrigin();
  pym_strict_checK_gl();

  DrawGround(PYM_CIRCLE_GROUND, m_vaoID);
  pym_strict_checK_gl();
  //pym_draw_square_ground();
  pym_strict_checK_gl();
  const int nb = phyCon->pymCfg->nBody;
  int i;
  FOR_0(i, nb) {
    /* Access data from renderer-accessable area of phyCon */
    const pym_rb_named_t *rbn = &phyCon->renBody[i].b;
    const pym_rb_named_t *rbn2 = &phyCon->pymCfg->body[i].b;
    const double *const boxSize = rbn->boxSize;
    pym_strict_checK_gl();
    DrawRb(rbn, boxSize, 0);
    //printf("%lf,%lf,%lf\n", rbn->q[0],rbn->q[1],rbn->q[2]);
    pym_strict_checK_gl();
    DrawRbRef(rbn2, boxSize, 1);
    //DrawRbContacts(rbn);
  }
  if (phyCon->pymCfg->renderFibers) {
    glDisable(GL_LIGHTING);
    const int nf = phyCon->pymCfg->nFiber;
    FOR_0(i, nf) {
      const pym_mf_named_t *mfn = &phyCon->pymCfg->fiber[i].b;
      double W_org[4][4], W_ins[4][4];
      const pym_rb_named_t *rbn_org = &phyCon->renBody[mfn->org].b;
      const pym_rb_named_t *rbn_ins = &phyCon->renBody[mfn->ins].b;
      double chi_org[6] = { rbn_org->p[0],
			    rbn_org->p[1],
			    rbn_org->p[2],
			    rbn_org->q[0],
			    rbn_org->q[1],
			    rbn_org->q[2] };
      double chi_ins[6] = { rbn_ins->p[0],
			    rbn_ins->p[1],
			    rbn_ins->p[2],
			    rbn_ins->q[0],
			    rbn_ins->q[1],
			    rbn_ins->q[2] };
      GetWFrom6Dof(W_org, chi_org);
      GetWFrom6Dof(W_ins, chi_ins);
      //mfn->fibb_org
      double org[3], ins[3];
      TransformPoint(org, W_org, mfn->fibb_org);
      TransformPoint(ins, W_ins, mfn->fibb_ins);
      glLineWidth(2.0);
      if (mfn->mType == PMT_ACTUATED_MUSCLE) {
	if (mfn->T * mfn->A < 0) {
	  glLineWidth(3.0);
	  glColor3f(1.0, 0, 0);
	} else {
	  glColor3f(0.8, 0.6, 0.6);
	}
      
      } else {
	glColor3f(0.5, 0.5, 0.5);
      }
      glBegin(GL_LINES);
      glVertex3dv(org);
      glVertex3dv(ins);
      glEnd();
    }
    glEnable(GL_LIGHTING);
  }
  static const double pointBoxSize[3] = { 1e-1, 1e-1, 1e-1 };
  glColor3f(1,1,1);
  pym_strict_checK_gl();
  DrawBox_pq(phyCon->bipCom, 0, pointBoxSize, 0);
  DrawBox_pq(phyCon->pymCfg->bipRefCom, 0, pointBoxSize, 0);
  printf("pymCfg address2 = %p\n", phyCon->pymCfg);
  if (strcmp(phyCon->pymCfg->trajName, "Walk1") == 0) {
    static const double s[3][3] = { { 1.557, 0.580, 0.210 },
				    { 1.557, 0.580, 0.413 },
				    { 1.557, 0.580, 0.630 } };	    
    static const double p[3][3] = { { 0.973, 0.298, 0.104 },
				    { 0.973, 0.891, 0.207 },	    
				    { 0.973, 1.478, 0.316 } };
    glColor3f(1,1,1);
    pym_strict_checK_gl();
    FOR_0(i, 3) {
      DrawBox_pq(p[i], 0, s[i], 0);
    }
  }
  glPopMatrix();
}

static int CreateGridPatternGroundTexture(void) {
  GLuint gndTex;
  glGenTextures(1, &gndTex);
  assert(gndTex > 0);
  glBindTexture( GL_TEXTURE_2D, gndTex );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		   GL_LINEAR_MIPMAP_LINEAR );
  // when texture area is large, bilinear filter the original
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  // the texture wraps over at the edges (repeat)
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  // allocate buffer
  int gndTexSize = 512;
  unsigned char *data = (unsigned char *)malloc( gndTexSize * gndTexSize * 3 );
  memset(data, 0, gndTexSize * gndTexSize * 3);

  unsigned char xAxisColor[] = { 120, 200, 220 };
  unsigned char yAxisColor[] = { 120, 200, 220 };
  int i, j;
  FOR_0(i, gndTexSize) {
    for (j=-1; j<=1; ++j) {
      data[3*(gndTexSize*(gndTexSize/2+j) + i) + 0] = xAxisColor[0];
      data[3*(gndTexSize*(gndTexSize/2+j) + i) + 1] = xAxisColor[1];
      data[3*(gndTexSize*(gndTexSize/2+j) + i) + 2] = xAxisColor[2];

      data[3*(gndTexSize*i + (gndTexSize/2+j)) + 0] = yAxisColor[0];
      data[3*(gndTexSize*i + (gndTexSize/2+j)) + 1] = yAxisColor[1];
      data[3*(gndTexSize*i + (gndTexSize/2+j)) + 2] = yAxisColor[2];
    }

    for (j=-gndTexSize/2; j<gndTexSize/2-1; j+=32) {
      data[3*(gndTexSize*(gndTexSize/2+j) + i) + 0] = xAxisColor[0];
      data[3*(gndTexSize*(gndTexSize/2+j) + i) + 1] = xAxisColor[1];
      data[3*(gndTexSize*(gndTexSize/2+j) + i) + 2] = xAxisColor[2];

      //            data[3*(gndTexSize*(gndTexSize/2+j+1) + i) + 0] = xAxisColor[0];
      //            data[3*(gndTexSize*(gndTexSize/2+j+1) + i) + 1] = xAxisColor[1];
      //            data[3*(gndTexSize*(gndTexSize/2+j+1) + i) + 2] = xAxisColor[2];
    }

    for (j=-gndTexSize/2; j<gndTexSize/2-1; j+=32) {
      data[3*(gndTexSize*i + (gndTexSize/2+j)) + 0] = yAxisColor[0];
      data[3*(gndTexSize*i + (gndTexSize/2+j)) + 1] = yAxisColor[1];
      data[3*(gndTexSize*i + (gndTexSize/2+j)) + 2] = yAxisColor[2];

      //            data[3*(gndTexSize*i + (gndTexSize/2+j+1)) + 0] = yAxisColor[0];
      //            data[3*(gndTexSize*i + (gndTexSize/2+j+1)) + 1] = yAxisColor[1];
      //            data[3*(gndTexSize*i + (gndTexSize/2+j+1)) + 2] = yAxisColor[2];
    }
  }


  // open and read texture data

  // build our texture mipmaps
  gluBuild2DMipmaps( GL_TEXTURE_2D, 3, gndTexSize, gndTexSize,
		     GL_RGB, GL_UNSIGNED_BYTE, data );
  glBindTexture(GL_TEXTURE_2D, 0);
  free(data);
  return gndTex;
}
static void RenderGraph(PRSGRAPH g, int slotid, pym_render_config_t *rc) {
  glPushMatrix(); /* stack A */
  const double margin = 0.05;
  double graphW, graphH;
  double graphGapX, graphGapY;
  const int width = rc->vpw;
  const int height = rc->vph;
  if (width > height) {
    graphGapX = margin*height/width;
    graphGapY = margin;
    graphW = 0.5*height/width;
    graphH = 0.5;
  } else {
    graphGapX = margin;
    graphGapY = margin*width/height;
    graphW = 0.5;
    graphH = 0.5*width/height;
  }
  const double graphX = -1 + graphGapX + slotid*(graphW + graphGapX);
  const double graphY = -1 + graphGapY;
  glTranslated(graphX, graphY, 0);
  glScaled(graphW, graphH, 1);
  glPushAttrib(GL_LIST_BIT | GL_DEPTH_BUFFER_BIT); /* stack B */
  glDisable(GL_DEPTH_TEST);
  PrsGraphRender(g);
  /* Graph title */
  /* glColor3f(1.0f, 1.0f, 1.0f); */
  /* glListBase(fps_font - ' '); */
  /* static const double fontHeight = 14.0; */
  /* glWindowPos2d((graphX + 1)*width/2, */
  /* 		(graphY + 1)*height/2 - fontHeight); */
  /* const char *test = PrsGraphTitle(g); */
  /* glCallLists(strlen(test), GL_UNSIGNED_BYTE, test); */
  glPopAttrib(); /* stack B */
  glPopMatrix(); /* stack A */
}

static void RenderSupportPolygon
(const pym_physics_thread_context_t *const phyCon,
 pym_render_config_t *rc) {
  int i;
  const int chInputLen = phyCon->pymCfg->renChInputLen;
  const int chOutputLen = phyCon->pymCfg->renChOutputLen;
  const int width = rc->vpw;
  const int height = rc->vph;
  if (!(chInputLen+1 >= chOutputLen)) {
    printf("chInputLen = %d, chOutputLen = %d\n",
	   chInputLen, chOutputLen);
    assert(chInputLen+1 >= chOutputLen);
  }
  if (chOutputLen) {
    /* PAIR A */
    glPushAttrib(GL_LINE_BIT | GL_CURRENT_BIT | GL_POINT_BIT);
    glPushMatrix(); /* PAIR B */

    const double margin = 0.05;
    /* size on normalized coordinates */
    const double sizeW = 0.5, sizeH = 0.5;
    double graphW, graphH;
    double graphGapX, graphGapY;
    if (width > height) {
      graphGapX = margin*height/width;
      graphGapY = margin;
      graphW = sizeW*height/width;
      graphH = sizeH;
    } else {
      graphGapX = margin;
      graphGapY = margin*width/height;
      graphW = sizeW;
      graphH = sizeH*width/height;
    }
    const int slotid = 0;
    const double graphX = -1 + graphW/2 + graphGapX
      + slotid*(graphW/2 + graphGapX);
    const double graphY = 1 - graphGapY - graphH/2;
    glTranslated(graphX, graphY, 0);
    glScaled(graphW, graphH, 1);

    double chSumX = 0, chSumY = 0;
    FOR_0(i, chOutputLen) {
      chSumX += phyCon->pymCfg->renChOutput[i].x;
      chSumY += phyCon->pymCfg->renChOutput[i].y;
    }
    const double chMeanX = chSumX / chOutputLen;
    const double chMeanY = chSumY / chOutputLen;
    glColor3f(0,1,0);
    glBegin(GL_LINE_LOOP);
    FOR_0(i, chOutputLen) {
      glVertex3d(phyCon->pymCfg->renChOutput[i].x - chMeanX,
		 phyCon->pymCfg->renChOutput[i].y - chMeanY,
		 0);
    }
    glEnd();

    assert(chInputLen > 0);
    glPointSize(3);
    glColor3f(0,1,0);
    glBegin(GL_POINTS);
    FOR_0(i, chInputLen) {
      glVertex3d(phyCon->pymCfg->renChInput[i].x - chMeanX,
		 phyCon->pymCfg->renChInput[i].y - chMeanY,
		 0);
    }
    glEnd();

    glPointSize(5);
    glTranslated(phyCon->pymCfg->bipCom[0] - chMeanX,
		 phyCon->pymCfg->bipCom[1] - chMeanY,
		 0);
    glScaled(0.015,0.015,0.015);
    glBegin(GL_LINE_LOOP);
    glVertex3d(1,1,0);
    glVertex3d(-1,1,0);
    glVertex3d(-1,-1,0);
    glVertex3d(1,-1,0);
    glEnd();

    glPopMatrix(); /* PAIR B */
    glPopAttrib(); /* PAIR A */
  }
}

void PymRsInitRender() {
  gndTex = CreateGridPatternGroundTexture();
  InitVertexArrayObjects(m_vaoID);
  //preparedata();
  generateShadowFBO();
  loadShadowShader();
}

void PymRsDestroyRender() {
  glDeleteTextures(1, &gndTex);
}

void PymRsRender(pym_rs_t *rs, pym_render_config_t *rc) {
  pym_strict_checK_gl();
  glUseProgramObjectARB(0);
  glViewport(rc->vpx, rc->vpy, rc->vpw, rc->vph);
  glClearColor(0.3,0.3,0.3,1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //  glCullFace(GL_BACK);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45, 1, 0.01, 100);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  double cam_car[3];            /* Cartesian coordinate of cam */
  double up_dir[3];
  pym_sphere_to_cartesian(cam_car, rc->cam_r, rc->cam_phi, rc->cam_theta);
  /* printf("cam_car = %lf %lf %lf\n", */
  /* 	 cam_car[0], cam_car[1], cam_car[2]); */
  pym_up_dir_from_sphere(up_dir, cam_car, rc->cam_r,
			 rc->cam_phi, rc->cam_theta);
  gluLookAt(rc->cam_cen[0]+cam_car[0],
	    rc->cam_cen[1]+cam_car[1],
	    rc->cam_cen[2]+cam_car[2],
	    rc->cam_cen[0],
	    rc->cam_cen[1],
	    rc->cam_cen[2],
	    up_dir[0],
	    up_dir[1],
	    up_dir[2]);

  pym_configure_light();
  pym_strict_checK_gl();

  pym_draw_all(&rs->phyCon, 0, m_vaoID);
  pym_strict_checK_gl();

  RenderFootContactFixPosition(&rs->phyCon);

  /* Head-up Display
   * --------------------------
   * Turn off shader and use fixed pipeline.
   * Also make sure we have identities
   * on projection and modelview matrix.
   */
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glDisable(GL_DEPTH_TEST);

  RenderGraph(rs->phyCon.comZGraph, 0, rc);
  //RenderGraph(rs->phyCon.comDevGraph, 1, rc);
  //RenderGraph(rs->phyCon.actGraph, 2, rc);
  //RenderGraph(rs->phyCon.ligGraph, 3, rc);

  pym_strict_checK_gl();
  RenderSupportPolygon(&rs->phyCon, rc);
  //RenderFootContactStatus(&rs->phyCon);
  pym_strict_checK_gl();

  glEnable(GL_DEPTH_TEST);
}

