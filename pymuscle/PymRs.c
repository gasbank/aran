/*
 * PymRs.c: Pymuscle Realtime Simulator
 * 2010 Geoyeob Kim
 */
/*
 * Simple test program for OpenGL and glX
 * Copyright (c) 2008 Cesare Tirabassi <norsetto@ubuntu.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
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
//#include <GL/glext.h>
//#include <GL/glx.h>
#include <GL/glut.h>
#include <sys/time.h>
#include <pthread.h>

#include <cholmod.h>
#include <umfpack.h>
#include <mosek.h>
#include <libconfig.h>

#include "PymStruct.h"
#include "Biped.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "PymuscleConfig.h"
#include "StateDependents.h"
#include "Config.h"
#include "DebugPrintDef.h"
#include "Optimize.h"
#include "PymJointAnchor.h"
#include "PymDebugMessageFlags.h"
#include "PhysicsThreadMain.h"
#include "TrajParser.h"
#include "PymCmdLineParser.h"
#include "MathUtil.h"

#include "model.h"
#include "common.h"
#include "camera.h"
#include "image.h"
#include "quaternion.h"

/*
 * Globals
 */

GLboolean print_fps=GL_FALSE;
GLboolean wireframe=GL_FALSE;
GLboolean transparency=GL_FALSE;
GLboolean skeleton=GL_FALSE;
GLboolean animation=GL_FALSE;
GLboolean selection=GL_FALSE;

GLint width, height;
Cursor picking;

/* Model variables */
MODEL_PTR model=NULL;
GLfloat roll=0.0f, pitch=0.0f, yaw=0.0f;
GLuint texture_id, sel_bone=0;
GLfloat t_inc=0.1f;
GLuint pose=1, old_pose=0;

/* Motion and view variables */
const GLfloat tStep = 100.f;	//Translational step
const GLfloat aStep = 0.001f;	//Rotational step (radians)
const GLfloat mouse_scale_t  = 0.001f; //Mouse smoothing translations
const GLfloat mouse_scale_r  = 0.2f;   //Mouse smoothing rotations
const GLfloat mouse_scale_a  = 0.002f; //Mouse smoothing angles
GLfloat t1 = 0.0f;

GLfloat sinE, cosE, sinA, cosA, fScale;

GLint old_x, old_y;

GLfloat xRot = 0.f, yRot = 0.f;
GLfloat xCam, yCam, zCam, eCam, aCam, lCam, vCam, dCam;

/* Timer */
struct timeval tv;
double etime, dt;
double t0 = 0.0f, t2;
GLuint fps_font;
GLfloat Timed = 0.5f;
GLfloat fps_mean = 0.0f, fps_count = 0.0f;

#define NUM_THREADS  3
#define TCOUNT 10
#define COUNT_LIMIT 12

int     count = 0;
int     thread_ids[3] = {0,1,2};
pthread_mutex_t count_mutex, main_mutex;
pthread_cond_t count_threshold_cv, physics_thread_finished;

GLuint gndTex;
// Z values will be rendered to this texture when using fboId framebuffer
GLuint depthTextureId;
// Hold id of the framebuffer for light POV rendering
GLuint fboId;
// Use to activate/disable shadowShader
GLhandleARB shadowShaderId;
GLuint shadowMapUniform;

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
	pfile = fopen(filename, "rb");
	if(!pfile)
	{
		printf("Sorry, can't open file: '%s'.\n", filename);
		exit(0);
	}

	fread(buffer,sizeof(char),400000,pfile);
	//printf("%s\n",buffer);


	fclose(pfile);

	handle = glCreateShaderObjectARB(type);
	if (!handle)
	{
		//We have failed creating the vertex shader object.
		printf("Failed creating vertex shader object from file: %s.",filename);
		exit(0);
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

	// If an error was detected.
	if (!result)
	{
		//We failed to compile.
		printf("Shader '%s' failed compilation.\n",filename);

		//Attempt to get the length of our error log.
		glGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &errorLoglength);

		//Create a buffer to read compilation error message
		errorLogText = malloc(sizeof(char) * errorLoglength);

		//Used to get the final length of the log.
		glGetInfoLogARB(handle, errorLoglength, &actualErrorLogLength, errorLogText);

		// Display errors.
		printf("%s\n",errorLogText);

		// Free the buffer malloced earlier
		free(errorLogText);
	}

	return handle;
}

void loadShadowShader()
{
	GLhandleARB vertexShaderHandle;
	GLhandleARB fragmentShaderHandle;

	vertexShaderHandle   = loadShader("VertexShader.c",GL_VERTEX_SHADER);
	fragmentShaderHandle = loadShader("FragmentShader.c",GL_FRAGMENT_SHADER);

	shadowShaderId = glCreateProgramObjectARB();

	glAttachObjectARB(shadowShaderId,vertexShaderHandle);
	glAttachObjectARB(shadowShaderId,fragmentShaderHandle);
	glLinkProgramARB(shadowShaderId);

	shadowMapUniform = glGetUniformLocationARB(shadowShaderId,"ShadowMap");
}

void change_size(GLsizei w, GLsizei h)
{
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
 *
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

/*
 * render
 */

void generateShadowFBO()
{
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

void setupMatrices(float position_x,float position_y,float position_z,float lookAt_x,float lookAt_y,float lookAt_z)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45,1.0,1,100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(position_x,position_y,position_z,lookAt_x,lookAt_y,lookAt_z,0,0,1);
}

void setTextureMatrix(void)
{
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

void startXform(double W[4][4])
{
    glPushMatrix();
	glMultTransposeMatrixd(W);

	glMatrixMode(GL_TEXTURE);
	glActiveTextureARB(GL_TEXTURE7);
	glPushMatrix();
	glMultTransposeMatrixd(W);
}
void endXform() {
    glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void startTranslate(float x,float y,float z)
{
	glPushMatrix();
	glTranslatef(x,y,z);

	glMatrixMode(GL_TEXTURE);
	glActiveTextureARB(GL_TEXTURE7);
	glPushMatrix();
	glTranslatef(x,y,z);
}

void endTranslate()
{
	endXform();
}

void DrawRb(const pym_rb_named_t *rbn,
            const double *const boxSize) {
    const double *const p = rbn->p;
    const double *const q = rbn->q;

    /* Translation and Rotation */
    double W[4][4];
    double chi[6];
    memcpy(chi + 0, p, sizeof(double)*3);
    memcpy(chi + 3, q, sizeof(double)*3);
    GetWFrom6Dof(W, chi);
    int j, k;
    FOR_0(j, 4)
        FOR_0(k, 3)
            W[j][k] *= boxSize[k];

    /* TODO: Remove scaling factor from the transform matrix W */
    glColor3f(0.3,0.75,0.3);
    startXform(W);
    glutSolidCube(1.0);
    endXform();
}

void DrawRbContacts(const pym_rb_named_t *rbn) {
    int j;
    FOR_0(j, rbn->nContacts_2) {
        const double *conPos   = rbn->contactsPoints_2a[j];
        const double *conForce = rbn->contactsForce_2[j];
        glColor3f(1,0,0);
        startTranslate(conPos[0], conPos[1], conPos[2]);
        glutSolidCube(0.05);
        endTranslate();
        const double scale = 0.001;
        glBegin(GL_LINES);
        glVertex3f(conPos[0], conPos[1], conPos[2]);
        glVertex3f(conPos[0]+conForce[0]*scale,
                   conPos[1]+conForce[1]*scale,
                   conPos[2]+conForce[2]*scale);
        glEnd();
    }
}

void drawObjects(pym_physics_thread_context_t *phyCxt, int forShadow, GLuint *m_vaoID) {
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();
//    gluLookAt(8, -8, 8, 0, 0, 0, 0, 0, 1);
//

    glPushMatrix();
    //Rotate scene
    //glRotatef(xRot, 1.0f, 0.0f, 0.0f);
    //glRotatef(yRot, 0.0f, 1.0f, 0.0f);


    //NormalizeVectorf(4, sunPos);
    //glLightfv(GL_LIGHT0, GL_POSITION, sunPos);

    //glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(1,0,0);
    glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,1,0);
    glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,1);
    glEnd();
    //glEnable(GL_LIGHTING);
    const float gndSize = 20.0f;
    const int gndTexRepeat = 4;
    const float gndTexOffset = 0.5;
//    if (!forShadow) {
//        glPushAttrib(GL_TEXTURE_BIT);
//        glBindTexture(GL_TEXTURE_2D, gndTex);
//    }


//    glColor3f(0.3,0.3,0.3);
//    glBegin(GL_QUADS);
//    glTexCoord2f(gndTexRepeat+gndTexOffset, gndTexRepeat+gndTexOffset); glNormal3f(0,0,1); glVertex3f( gndSize/2, gndSize/2, 0);
//    glTexCoord2f(0+gndTexOffset,            gndTexRepeat+gndTexOffset); glNormal3f(0,0,1); glVertex3f(-gndSize/2, gndSize/2, 0);
//    glTexCoord2f(0+gndTexOffset,                       0+gndTexOffset); glNormal3f(0,0,1); glVertex3f(-gndSize/2,-gndSize/2, 0);
//    glTexCoord2f(gndTexRepeat+gndTexOffset,            0+gndTexOffset); glNormal3f(0,0,1); glVertex3f( gndSize/2,-gndSize/2, 0);
//    glEnd();

//    glBindVertexArray(m_vaoID[0]);      // select first VAO
//    glDrawArrays(GL_TRIANGLES, 0, 3);   // draw first object
//    glBindVertexArray(0);

    glColor3f(0.4,0.4,0.4);
    glBindVertexArray(m_vaoID[2]);      // select second VAO
    glVertexAttrib3f((GLuint)1, 1.0, 1.0, 0.0); // set constant color attribute
    glDrawArrays(GL_QUADS, 0, 4);   // draw second object


//    if (!forShadow) {
//        glPopAttrib();
//    }

    const int nb = phyCxt->pymCfg->nBody;
    pthread_mutex_lock(&main_mutex);
    int i, j, k;
    FOR_0(i, nb) {
        const pym_rb_named_t *rbn = &phyCxt->pymCfg->body[i].b;
        const double *const boxSize = phyCxt->pymCfg->body[i].b.boxSize;
        DrawRb(rbn, boxSize);
        DrawRbContacts(rbn);
    }
    glEnd();
    pthread_mutex_unlock(&main_mutex);

    glPopMatrix();
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

void render(pym_physics_thread_context_t *phyCxt, GLuint *m_vaoID)
{
    static GLint iFrames = 0;
    static GLfloat fps = 0.0f, DeltaT, t;
    static char cBuffer[64];
    struct timeval tv;
    GLuint i;

    // Update timer
    gettimeofday(&tv, NULL);
    etime = (double)tv.tv_sec + tv.tv_usec / 1000000.0f;

    dt = etime - t0;
    t0 = etime;

    // Clear the window with current clearing color
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Set up camera location and parameters
    //setup_camera((float)dt);

    //We are in animation mode
//	if(animation) {
//	  t1 += (float)dt;
//
//	  //End of this keyframe
//	  if(t1>model->pose[pose].t) {
//	    t1=t1-model->pose[pose].t;
//	    old_pose=pose;
//	    if(pose<model->num_poses-1)
//	      pose++;
//	    else
//	      pose=0;
//	  }
//
//	  //Interpolate start and stop quaternion
//	  t = t1 / model->pose[pose].t;
//	  for(i=0; i<model->num_bones; i++) {
//	    slerp(
//		  &model->bone[i].Q,
//		  &model->pose[old_pose].Q[i],
//		  &model->pose[pose].Q[i],
//		  t);
//	    quat2mv(model->bone[i].mvr,&model->bone[i].Q);
//	  }
//
//	  //Update the model
//	  updateModel(model);
//	}
//	//We are in pose mode
//	else if(sel_bone) {
//
//	  //compute modelview matrix for selected bone
//	  compute_mv(sel_bone-1, roll, pitch, yaw);
//
//	  //Update the model
//	  updateModel(model);
//	}

//		//Draw skeleton
//		if(skeleton)
//		  drawSkeleton(model);
//
//		//Draw model
//		if(transparency)
//		  glEnable(GL_BLEND);
//
//		drawModel(model);



    //First step: Render from the light POV to a FBO, story depth values only
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fboId);	//Rendering offscreen

	//Using the fixed pipeline to render to the depthbuffer
	glUseProgramObjectARB(0);

	// In the case we render the shadowmap to a higher resolution, the viewport must be modified accordingly.
	//glViewport(0,0,RENDER_WIDTH * SHADOW_MAP_RATIO,RENDER_HEIGHT* SHADOW_MAP_RATIO);
	glViewport(0, 0, 1024, 1024);

	// Clear previous frame values
	glClear(GL_DEPTH_BUFFER_BIT);

	//Disable color rendering, we only want to write to the Z-Buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// Directional light (e.g. sunlight)
    float sunPos[4] = { 0.2, -0.8, 1, 0 };
	float p_light[] = {sunPos[0]*10, sunPos[1]*10, sunPos[2]*10};
	float l_light[] = {0, 0, 0};

	setupMatrices(p_light[0],p_light[1],p_light[2],
               l_light[0],l_light[1],l_light[2]);

	// Culling switching, rendering only backface, this is done to avoid self-shadowing
	glCullFace(GL_FRONT);
	drawObjects(phyCxt, 1, m_vaoID);

	//Save modelview/projection matrice into texture7, also add a biais
	setTextureMatrix();


	// Now rendering from the camera POV, using the FBO to generate shadows
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);

	glViewport(0, 0, 700, 700);

	//Enabling color write (previously disabled for light POV z-buffer rendering)
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// Clear previous frame values
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Using the shadow shader
	glUseProgramObjectARB(shadowShaderId);
	glUniform1iARB(shadowMapUniform,7);
	glActiveTextureARB(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D,depthTextureId);




	float p_camera[] = {8, -8, 8};
	float l_camera[] = {0, 0, 0};
	float pRotX[3], pRotXY[3];
	XRotPoint(pRotX, p_camera, xRot/180.0f*M_PI);
	YRotPoint(pRotXY, pRotX, yRot/180.0f*M_PI);
	setupMatrices(pRotXY[0], pRotXY[1], pRotXY[2],
               l_camera[0],l_camera[1],l_camera[2]);

	glCullFace(GL_BACK);
	drawObjects(phyCxt, 0, m_vaoID);

//    glUseProgramObjectARB(0);
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glOrtho(-350,350,-350,350,1,20);
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();
//    glColor4f(1,1,1,1);
//    glActiveTextureARB(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D,depthTextureId);
//    glEnable(GL_TEXTURE_2D);
//    glTranslated(0,0,-1);
//    glBegin(GL_QUADS);
//    glTexCoord2d(0,0);glVertex3f(0,0,0);
//    glTexCoord2d(1,0);glVertex3f(700/2,0,0);
//    glTexCoord2d(1,1);glVertex3f(700/2,700/2,0);
//    glTexCoord2d(0,1);glVertex3f(0,700/2,0);
//    glEnd();
//    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(0.2,0.2,0.9);
    glBegin(GL_LINES);
    glVertex3f(0,0,-1);
    glVertex3f(1,0,-1);
    glVertex3f(0,0,-1);
    glVertex3f(0,1,-1);
    glEnd();
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);
    glPushAttrib(GL_LIST_BIT);
    glListBase(fps_font - ' ');
    glWindowPos2i(0,100);
    glCallLists(strlen("abc"), GL_UNSIGNED_BYTE, "abc");
    glPopAttrib();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);


    iFrames++;
    DeltaT = (GLfloat)(etime-t2);
    if( DeltaT >= Timed )
    {
        fps = (GLfloat)(iFrames)/DeltaT;
        fps_count++;
        fps_mean = ((fps_count - 1.0f) * fps_mean + fps ) / fps_count;

        iFrames = 0;
        t2 = etime;
    }
    if (print_fps)
    {
        sprintf(cBuffer, "FPS: %.1f", fps);
//	  if(animation)
//	    sprintf(cBuffer, "FPS: %.1f Pose: %d T: %2.3f",
//		    fps, pose, t1);
//	  else
//	    sprintf(cBuffer, "FPS: %.1f ID: %d Pose: %d T: %2.3f Roll: %2.3f Pitch %2.3f Yaw %2.3f",
//		    fps, sel_bone, model->num_poses-1,
//		    model->pose[model->num_poses-1].t,
//		    roll, pitch, yaw);

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
}

/*
 * Retrieve attitude angles
 *
 */
inline void retrieve_angles(void)
{

    roll=atan2f(-model->bone[sel_bone-1].mvr[6],
                model->bone[sel_bone-1].mvr[10]);
    yaw =atan2f(-model->bone[sel_bone-1].mvr[1],
                model->bone[sel_bone-1].mvr[0]);
    pitch=asinf(model->bone[sel_bone-1].mvr[2]);
}

/*
 * Process the selection triggered by left mouse click while in selection mode
 *
 */

void processSelection(int x, int y)
{
    GLuint i, vp[4];
    GLubyte pix[3];
    GLfloat ps;

    glDisable(GL_DITHER);
    glDisable(GL_TEXTURE_2D);
    glGetFloatv(GL_POINT_SIZE, &ps);
    glPointSize(20.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();

    setup_camera((float)dt);
    glRotatef(xRot, 1.0f, 0.0f, 0.0f);
    glRotatef(yRot, 0.0f, 1.0f, 0.0f);

    //Draw joints to backbuffer
    glBegin(GL_POINTS);
    for(i=0; i<model->num_bones; i++)
    {
        glColor3ub(i+1, 0, 0);
        glVertex3fv(&model->bone[i].mva[12]);
    }
    glEnd();

    glPopMatrix();
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DITHER);
    glPointSize(ps);

    glGetIntegerv(GL_VIEWPORT, vp);

    glReadPixels(x,vp[3]-y,1,1,
                 GL_RGB,GL_UNSIGNED_BYTE,(void *)pix);

    sel_bone=pix[0];
    if(sel_bone)
        retrieve_angles();
}

/*
 * handle_mouse_button
 */

void handle_mouse_button( int button, int state, int x, int y )
{
    if (state)
        return;
    switch (button)
    {
    case 1:
        if(selection)
            processSelection(x,y);
        else
        {
            old_x = x;
            old_y = y;
        }
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
//	  if(sel_bone-=(sel_bone?1:-model->num_bones))
//	    retrieve_angles();
        break;
    case 5: /* rotate wheel downward */
//	  if(sel_bone++<model->num_bones)
//	    retrieve_angles();
//	  else
//	    sel_bone = 0;
        break;
    }
}

/*
 * handle_mouse_motion
 */

void handle_mouse_motion(int button, int x, int y )
{
    switch (button)
    {
    case 1:
        if(!selection)
        {
            if(sel_bone)
            {
                yaw   -= (y-old_y) * mouse_scale_a;
                pitch += (x-old_x) * mouse_scale_a;
            }
            else
            {
                eCam += (y-old_y) * mouse_scale_t;
                aCam += (x-old_x) * mouse_scale_t;
            }
            old_y = y;
            old_x = x;
        }
        break;
    case 2:
        if(sel_bone)
        {
            roll  -= (y-old_y) * mouse_scale_a;
            pitch += (x-old_x) * mouse_scale_a;
        }
        old_y = y;
        old_x = x;
        break;
    case 3:
        xRot -=(y-old_y) * mouse_scale_r;
        yRot -=(x-old_x) * mouse_scale_r;
        old_y = y;
        old_x = x;
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


    GLenum err;

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

void event_loop(Display* display, Window window,
                Atom wmDeleteMessage,
                pym_physics_thread_context_t *phyCxt,
                GLuint *m_vaoID )
{
    int exertExternalForce = 0;
    while (1)
    {
        while (XPending(display) > 0)
        {
            XEvent event;
            int code;
            XNextEvent( display, &event );

            if (event.type == ClientMessage &&
                    event.xclient.data.l[0] == wmDeleteMessage)
            {
                printf("Shutting down now!!!\n");
                return;
            }

            switch (event.type)
            {
            case Expose:
                break;
            case ConfigureNotify:
                change_size(event.xconfigure.width, event.xconfigure.height);
                break;
            case KeyPress:
            {
                code = XLookupKeysym( &event.xkey, 0);

                if ((code == XK_Escape)||(code == XK_q))
                    return;
                if(code == XK_a)
                    exertExternalForce = 1;
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
                    if(model->num_poses>1)
                    {
                        animation=!animation;
                        pose = 1;
                        old_pose = 0;
                        t1 = 0.0f;
                    }
                if(code == XK_p)
                {

                    //Dump quaternions to STDOUT and add them as a pose

                    GLuint i;

                    model->num_poses++;
                    model->pose=(POSE_PTR)realloc(model->pose,
                                                  sizeof(POSE)*model->num_poses);
                    model->pose[model->num_poses-1].Q=
                        (VECTOR4D_PTR)malloc(sizeof(VECTOR4D)*model->num_bones);
                    model->pose[model->num_poses-1].t=2.0f;

                    printf("Dumping quaternions\n");
                    for(i=0; i<model->num_bones; i++)
                    {
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
                if (code == XK_r)
                {
                    wireframe = !wireframe;
                    if (wireframe)
                        glPolygonMode(GL_FRONT, GL_LINE);
                    else
                        glPolygonMode(GL_FRONT, GL_FILL);
                }
                if (code == XK_Shift_L)
                {
                    selection=GL_TRUE;
                    XDefineCursor(display, window, picking);
                }
                if (code == XK_bracketleft)
                {
                    if( (model->pose[model->num_poses-1].t -= t_inc) < t_inc )
                        model->pose[model->num_poses-1].t=t_inc;
                }
                if (code == XK_bracketright)
                    model->pose[model->num_poses-1].t += t_inc;
                break;
            }
            case KeyRelease:
            {
                code = XLookupKeysym( &event.xkey, 0);

                if (code == XK_Shift_L)
                {
                    selection=GL_FALSE;
                    XUndefineCursor(display, window);
                }
                break;
            }
            case ButtonPress:
            {
                handle_mouse_button(event.xbutton.button,
                                    0,
                                    event.xbutton.x,
                                    event.xbutton.y);
                break;
            }
            case ButtonRelease:
            {
                handle_mouse_button(event.xbutton.button,
                                    1,
                                    event.xbutton.x,
                                    event.xbutton.y);
                break;
            }
            case MotionNotify:
            {
                if (event.xmotion.state & Button1Mask)
                {
                    handle_mouse_motion(1,
                                        event.xmotion.x,
                                        event.xmotion.y);
                    break;
                }
                if (event.xmotion.state & Button2Mask)
                {
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
        }

        render(phyCxt, m_vaoID);

        //simplerender(m_vaoID);

        glXSwapBuffers( display, window );
        if (exertExternalForce) {
            pthread_mutex_lock(&main_mutex); {
                phyCxt->trunkExternalForce[0] = 5000;
                phyCxt->trunkExternalForce[1] = 4000;

            } pthread_mutex_unlock(&main_mutex);
            exertExternalForce = 0;
        }
    }
}

int CreateGridPatternGroundTexture(void) {
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
    unsigned char *data = malloc( gndTexSize * gndTexSize * 3 );
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

void PrintCmdLineHelp(int argc, const char *const argv[]) {
    printf("  Usage:\n");
    printf("    %s config_file <simulation conf file> [Options]\n", strrchr(argv[0], '/') + 1);
    printf("\n");
    printf("  Options:\n");
    printf("\n");
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

void setdata(GLuint *m_vboID, GLuint *m_vaoID)
{
    // First simple object
    float vert[9]; // vertex array
    float col[9];  // color array

    vert[0] =-0.3; vert[1] = 0.5; vert[2] =-1.0;
    vert[3] =-0.8; vert[4] =-0.5; vert[5] =-1.0;
    vert[6] = 0.2; vert[7] =-0.5; vert[8]= -1.0;

    col[0] = 1.0; col[1] = 1.0; col[2] = 1.0;
    col[3] = 0.0; col[4] = 1.0; col[5] = 0.0;
    col[6] = 0.0; col[7] = 0.0; col[8] = 1.0;

    // Second simple object
    float vert2[9]; // vertex array

    vert2[0] =-0.2; vert2[1] = 0.5; vert2[2] =-1.0;
    vert2[3] = 0.3; vert2[4] =-0.5; vert2[5] =-1.0;
    vert2[6] = 0.8; vert2[7] = 0.5; vert2[8]= -1.0;

    // Third simple object
    GLfloat vert3[] = {  5.3f,  5.3f,  0.0f,
                      -5.3f,  5.3f,  0.0f,
                      -5.3f, -5.3f,  0.0f,
                       5.3f, -5.3f,  0.0f  };



    // Three VAOs allocation (Vertex Array Object)
    glGenVertexArrays(3, m_vaoID);

    // First VAO setup
    glBindVertexArray(m_vaoID[0]);

    // 2 VBOs (Vertex Buffer Object) for the first VAO
    glGenBuffers(2, m_vboID);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboID[0]);
    glBufferData(GL_ARRAY_BUFFER, 9*sizeof(GLfloat), vert, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboID[1]);
    glBufferData(GL_ARRAY_BUFFER, 9*sizeof(GLfloat), col, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // Second VAO setup
    glBindVertexArray(m_vaoID[1]);

    // 1 VBO for the second VAO
    glGenBuffers(1, &m_vboID[2]);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboID[2]);
    glBufferData(GL_ARRAY_BUFFER, 9*sizeof(GLfloat), vert2, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);


    // Third VAO setup
    glBindVertexArray(m_vaoID[2]);

    // 1 VBO for the third VAO
    GLuint vboxx;
    glGenBuffers(1, &vboxx);

    glBindBuffer(GL_ARRAY_BUFFER, vboxx);
    glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), vert3, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

int main(int argc, char *argv[])
{
    int ret=0;
    char*			display_name;
    Display*		display;
    Window			window;
    int			scrnum;
    Window			root;
    int			x, y;
    GLXContext		context;
    XVisualInfo*		vinfo;
    XSetWindowAttributes	winattrs;
    XFontStruct*		font;
    unsigned long		winmask;
    static int		attributes[] =
    {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        0
    };
    printf("Pymuscle realtime simulator      -- 2010 Geoyeob Kim\n");
    if (argc < 2)
    {
        PrintCmdLineHelp(argc, argv);
        return 1;
    }
    /* GLUT is used for using glutSolidCube() or glutWireCube()
     * routines only. */
    glutInit(&argc, argv);


    /* Initialize debug message flags (dmflags) */
    int dmflags[PDMTE_COUNT] = {0,};
    dmflags[PDMTE_FBYF_REF_TRAJ_DEVIATION_REPORT] = 1;
    dmflags[PDMTE_FBYF_ANCHORED_JOINT_DISLOCATION_REPORT] = 1;
    FILE *dmstreams[PDMTE_COUNT];
    int i;
    FILE *devnull = fopen("/dev/null", "w");
    FOR_0(i, PDMTE_COUNT) {
    	if (dmflags[i]) dmstreams[i] = stdout;
    	else dmstreams[i] = devnull;
    }
    /* Parse command line options */
    pym_cmdline_options_t cmdopt;
    ret = PymParseCmdlineOptions(&cmdopt, argc, argv);
    if (ret < 0) {
        printf("Failed.\n");
        return -2;
    }
    /* Construct pymCfg structure */
    pym_config_t pymCfg;
    ret = PymConstructConfig(cmdopt.simconf, &pymCfg, dmstreams[PDMTE_INIT_MF_FOR_EACH_RB]);
    if (ret < 0) {
        printf("Failed.\n");
        return -1;
    }
    PymConvertRotParamInPlace(&pymCfg, RP_EXP);

    int exportFps = 0;
    int nBlenderFrame = 0;
    int nBlenderBody = 0;
    int corresMapIndex[pymCfg.nBody];
    FOR_0(i, pymCfg.nBody)
        corresMapIndex[i] = -1;
    double *trajData = 0;
    if (cmdopt.trajconf) {
        char corresMap[MAX_CORRESMAP][2][128];
        int nCorresMap = 0;
    	int parseRet = PymParseTrajectoryFile(
    			corresMap,
    			&nCorresMap,
    			&trajData,
    			&nBlenderBody,
    			&nBlenderFrame,
    			&exportFps,
    			cmdopt.trajconf,
    			cmdopt.trajdata);
        assert(parseRet == 0);
    	assert(nCorresMap > 0);
    	/* The simulation time step defined in simconf and
    	 * the frame time (reciprocal of FPS) in trajdata
    	 * should have the same value. If mismatch happens
    	 * we ignore simulation time step in simconf.
    	 */
    	if (fabs(1.0/exportFps - pymCfg.h) > 1e-6) {
    		printf("Warning - simulation time step defined in simconf and\n");
    		printf("          trajectory data do not match.\n");
    		printf("            simconf  : %3d FPS (%lf sec per frame)\n", (int)ceil(1.0/pymCfg.h), pymCfg.h);
    		printf("            trajconf : %3d FPS (%lf sec per frame)\n", exportFps, 1.0/exportFps);
    		printf("          simconf's value will be ignored.\n");
    		pymCfg.h = 1.0/exportFps;
    	}

        PymCorresMapIndexFromCorresMap(corresMapIndex,
                                       nCorresMap,
                                       corresMap,
                                       nBlenderBody,
                                       &pymCfg,
                                       dmstreams);
        char fnJaCfg[128] = {0};
        PymInferJointAnchorConfFileName(fnJaCfg, cmdopt.trajconf);
        pymCfg.na = PymParseJointAnchorFile(pymCfg.pymJa, sizeof(pymCfg.pymJa)/sizeof(pym_joint_anchor_t), fnJaCfg);
        assert(pymCfg.na >= 0);
        printf("Info - # of joint anchors parsed = %d\n", pymCfg.na);

        /*
         * We need at least three frames of trajectory data
         * to follow one or more reference trajectory frames since
         * the first (frame 0) is used as previous step and
         * the second (frame 1) is used as current step and
         * the third (frame 2) is used as the reference trajectory for next step
         *
         * We need (nBlenderFrame-2) simulation iteration to complete
         * following the trajectory entirely. So the following assertion helds:
         */
        assert(nBlenderFrame >= 3);
        PymSetInitialStateUsingTrajectory(&pymCfg, nBlenderBody, corresMapIndex, trajData);
        PymInitJointAnchors(&pymCfg, dmstreams);
        PymConstructAnchoredJointList(&pymCfg);
    } else {
        PymSetPymCfgChiRefToCurrentState(&pymCfg);
    }

    if (cmdopt.frame >= 0)
        pymCfg.nSimFrame = cmdopt.frame;
    else if (cmdopt.trajconf)
        pymCfg.nSimFrame = nBlenderFrame - 2;
    else
        pymCfg.nSimFrame = 100;

    FILE *outputFile = fopen(cmdopt.output, "w");
    if (!outputFile) {
        printf("Error: Opening the output file %s failed.\n", cmdopt.output);
        return -3;
    }
    fprintf(outputFile, "%d %d\n", pymCfg.nSimFrame, pymCfg.nBody);

    /* Let's start the simulation happily :) */
    printf("Starting the tracking simulation...\n");

    /******************************/
    /******************************/

    /* Open the connection to the X server */
    display_name = getenv("DISPLAY");
    display = XOpenDisplay(display_name);

    if (!display)
    {
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

    if (!vinfo)
    {
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

    winmask = CWBitGravity | CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    window = XCreateWindow( display, root,
                            x, y, width, height, 0, vinfo->depth, InputOutput,
                            vinfo->visual, winmask, &winattrs );
    if (!window)
    {
        printf("Error: couldn't create window\n");
        return 1;
    }

    const char *fullAppName = "Pymuscle Realtime Simulator";
    const char *taskbarAppName = "Pymuscle";
    XChangeProperty(display, window, XA_WM_NAME, XA_STRING, 8, 0,
                    fullAppName, strlen(fullAppName));
    XChangeProperty(display, window, XA_WM_ICON_NAME, XA_STRING, 8, 0,
                    taskbarAppName, strlen(taskbarAppName));

    Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wmDeleteMessage, 1);

    /* Create the picking cursor */
    picking = XCreateFontCursor(display, 34);

    /* Create the OpenGL context */
    context = glXCreateContext(display, vinfo, 0, True);

    if (!context)
    {
        printf("Error: glXCreateContext failed\n");
        return 1;
    }

    XFree(vinfo);

    XMapWindow( display, window );
    glXMakeCurrent( display, window, context );

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        printf("Error: glewInit() failed\n");
        return 1;
    }
assert(glXChooseFBConfig);

  static int visual_attribs[] =
    {
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
      if ( !fbc )
      {
        printf( "Failed to retrieve a framebuffer config\n" );
        exit(1);
      }
      printf( "Found %d matching FB configs.\n", fbcount );

      // Pick the FB config/visual with the most samples per pixel
      printf( "Getting XVisualInfos\n" );
      int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;


      for ( i = 0; i < fbcount; i++ )
      {
        XVisualInfo *vi = glXGetVisualFromFBConfig( display, fbc[i] );
        if ( vi )
        {
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

      GLXFBConfig bestFbc = fbc[ best_fbc ];

      // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
      XFree( fbc );


    int attribs[] =
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 2,
        GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        0
    };
    GLXContext ctx = 0;

//    if(glxewIsSupported("GLX_ARB_create_context"))
//    {
//        printf("GOOD\n");
//
//        ctx = glXCreateContextAttribsARB(display, bestFbc, 0, True, attribs);
//
//        //glXMakeCurrent(display, window, 0);
//        glXDestroyContext(display, context);
//        glXMakeCurrent(display, window, ctx);
//    }
//    else
//    {   //It's not possible to make a GL 3.x context. Use the old style context (GL 2.1 and before)
//        //m_hrc = tempContext;
//
//        printf("BAD\n");
//        return -2;
//    }
//    // Sync to ensure any errors generated are processed.
//    XSync( display, False );
//    if ( ctx )
//      printf( "Created OpenGL 3.2 context!\n" );


    /* Setup fonts */
    font = XLoadQueryFont(display,
                          "-*-courier-bold-r-normal--14-*-*-*-*-*-*-*");

    fps_font = glGenLists(96);

    glXUseXFont(font->fid, ' ', 96, fps_font);

    gndTex = CreateGridPatternGroundTexture();

    GLuint m_vaoID[3];      // two vertex array objects, one for each drawn object
    GLuint m_vboID[3];      // three VBOs

    setdata(m_vboID, m_vaoID);
    //preparedata();
	generateShadowFBO();
	loadShadowShader();


    /* Initialize the physics thread context */
    pym_physics_thread_context_t phyCxt = {
        .pymCfg = &pymCfg,
        .nBlenderBody = nBlenderBody,
        .cmdopt = &cmdopt,
        .corresMapIndex = corresMapIndex,
        .outputFile = outputFile,
        .dmstreams = dmstreams,
        .totalPureOptTime = 0,
        .trajData = trajData,
        .stop = 0,
        .trunkExternalForce = {0,}
    };
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
    pthread_create(&thPhysics, &attr, PhysicsThreadMain, (void *)&phyCxt);
    //pthread_create(&threads[1], &attr, inc_count, (void *)t2);
    //pthread_create(&threads[2], &attr, inc_count, (void *)t3);

    glClearColor(0.1, 0.1, 0.2, 0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_TEXTURE_2D);
    //glCullFace(GL_BACK);
    glEnable(GL_NORMALIZE);
    glEnable(GL_ALPHA_TEST);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

    const float cutoff = 180;
    glLightfv(GL_LIGHT0, GL_SPOT_CUTOFF, &cutoff);
    const float noAmbient[] = {0.5f, 0.5f, 0.5f, 1.0f};
    const float whiteDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, noAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteDiffuse);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, 1.0, 0.1, 100.0);

    /*************
     * Event loop
     *************/
    event_loop( display, window, wmDeleteMessage, &phyCxt, m_vaoID );

    pthread_mutex_lock(&main_mutex);
    phyCxt.stop = 1;
    pthread_mutex_unlock(&main_mutex);

    /* Wait for the physics thread to complete */
    puts("Wait for the physics thread to complete...\n");
    pthread_join(thPhysics, NULL);

    /* TODO: Force-canceling physics thread.
     *       Is there way to exit gracefully? */
    pthread_cancel(thPhysics);

    /* Print average fps */
    printf("\nAverage FPS: %.1f\n", fps_mean);

    fclose(outputFile);
    printf("Output written to %s\n", cmdopt.output);

    PymDestoryConfig(&pymCfg);

    free(trajData);

    if (cmdopt.freeTrajStrings) {
        free(cmdopt.trajconf);
        free(cmdopt.trajdata);
    }
    if (cmdopt.freeOutputStrings) {
        free(cmdopt.output);
    }

    printf("Accumulated pure MOSEK optimizer time : %lf s\n", phyCxt.totalPureOptTime);


//    /* Setup Rendering Context */
//    if (setup(argv[1])) {
//       printf("Setup failed, exiting ...\n");
//       ret=1;
//    }
//    else {
//       /* Event loop */
//      event_loop( display, window );
//      /* Print average fps */
//      printf("\nAverage FPS: %.1f\n", fps_mean);
//    }

    /* Clean up */
    if(model)
        deleteModel(model);
    if(texture_id)
        glDeleteTextures(1, &texture_id);

    glDeleteTextures(1, &gndTex);

    XFreeFont(display, font);
    //glXDestroyContext( display, context );
    glXDestroyContext( display, ctx );
    XDestroyWindow( display, window );
    XCloseDisplay( display );


    /* pthread Clean up and exit */
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&count_mutex);
    pthread_mutex_destroy(&main_mutex);
    pthread_cond_destroy(&count_threshold_cv);
    pthread_cond_destroy(&physics_thread_finished);
    //pthread_exit(ret);
    return ret;
}
