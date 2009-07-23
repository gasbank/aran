#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

#ifdef WIN32
	#include <SDL.h>
	#include <SDL_opengl.h>
#else
	#define GL_GLEXT_PROTOTYPES
	#include <SDL/SDL.h>
	#include <SDL/SDL_opengl.h>
#endif

#include "ft2build.h"
#include FT_FREETYPE_H

#ifdef WIN32
#include <d3dx9.h>
#endif
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_USE

#include "cml/cml.h"

#include "IL/il.h"

#include "Macros.h"
#include "Structs.h"
#include "Singleton.h"
#include "ArnXmlLoader.h"
#include "ArnXmlString.h"
#include "ArnSceneGraph.h"
#include "ArnCamera.h"
#include "VideoManGl.h"
#include "ArnSkeleton.h"
#include "ArnAnimationController.h"
#include "ArnMesh.h"

#ifdef WIN32
PFNGLGENBUFFERSARBPROC					glGenBuffersARB = 0;                     // VBO Name Generation Procedure
PFNGLBINDBUFFERARBPROC					glBindBufferARB = 0;                     // VBO Bind Procedure
PFNGLBUFFERDATAARBPROC					glBufferDataARB = 0;                     // VBO Data Loading Procedure
PFNGLBUFFERSUBDATAARBPROC				glBufferSubDataARB = 0;               // VBO Sub Data Loading Procedure
PFNGLDELETEBUFFERSARBPROC				glDeleteBuffersARB = 0;               // VBO Deletion Procedure
PFNGLGETBUFFERPARAMETERIVARBPROC		glGetBufferParameterivARB = 0; // return various parameters of VBO
PFNGLMAPBUFFERARBPROC					glMapBufferARB = 0;                       // map VBO procedure
PFNGLUNMAPBUFFERARBPROC					glUnmapBufferARB = 0;                   // unmap VBO procedure

PFNGLACTIVETEXTUREARBPROC				glActiveTextureARB;
PFNGLCLIENTACTIVETEXTUREARBPROC			glClientActiveTextureARB;
PFNGLMULTITEXCOORD1DARBPROC				glMultiTexCoord1dARB;
PFNGLMULTITEXCOORD1DVARBPROC			glMultiTexCoord1dvARB;
PFNGLMULTITEXCOORD1FARBPROC				glMultiTexCoord1fARB;
PFNGLMULTITEXCOORD1FVARBPROC			glMultiTexCoord1fvARB;
PFNGLMULTITEXCOORD1IARBPROC				glMultiTexCoord1iARB;
PFNGLMULTITEXCOORD1IVARBPROC			glMultiTexCoord1ivARB;
PFNGLMULTITEXCOORD1SARBPROC				glMultiTexCoord1sARB;
PFNGLMULTITEXCOORD1SVARBPROC			glMultiTexCoord1svARB;
PFNGLMULTITEXCOORD2DARBPROC				glMultiTexCoord2dARB;
PFNGLMULTITEXCOORD2DVARBPROC			glMultiTexCoord2dvARB;
PFNGLMULTITEXCOORD2FARBPROC				glMultiTexCoord2fARB;
PFNGLMULTITEXCOORD2FVARBPROC			glMultiTexCoord2fvARB;
PFNGLMULTITEXCOORD2IARBPROC				glMultiTexCoord2iARB;
PFNGLMULTITEXCOORD2IVARBPROC			glMultiTexCoord2ivARB;
PFNGLMULTITEXCOORD2SARBPROC				glMultiTexCoord2sARB;
PFNGLMULTITEXCOORD2SVARBPROC			glMultiTexCoord2svARB;
PFNGLMULTITEXCOORD3DARBPROC				glMultiTexCoord3dARB;
PFNGLMULTITEXCOORD3DVARBPROC			glMultiTexCoord3dvARB;
PFNGLMULTITEXCOORD3FARBPROC				glMultiTexCoord3fARB;
PFNGLMULTITEXCOORD3FVARBPROC			glMultiTexCoord3fvARB;
PFNGLMULTITEXCOORD3IARBPROC				glMultiTexCoord3iARB;
PFNGLMULTITEXCOORD3IVARBPROC			glMultiTexCoord3ivARB;
PFNGLMULTITEXCOORD3SARBPROC				glMultiTexCoord3sARB;
PFNGLMULTITEXCOORD3SVARBPROC			glMultiTexCoord3svARB;
PFNGLMULTITEXCOORD4DARBPROC				glMultiTexCoord4dARB;
PFNGLMULTITEXCOORD4DVARBPROC			glMultiTexCoord4dvARB;
PFNGLMULTITEXCOORD4FARBPROC				glMultiTexCoord4fARB;
PFNGLMULTITEXCOORD4FVARBPROC			glMultiTexCoord4fvARB;
PFNGLMULTITEXCOORD4IARBPROC				glMultiTexCoord4iARB;
PFNGLMULTITEXCOORD4IVARBPROC			glMultiTexCoord4ivARB;
PFNGLMULTITEXCOORD4SARBPROC				glMultiTexCoord4sARB;
PFNGLMULTITEXCOORD4SVARBPROC			glMultiTexCoord4svARB;
#endif

struct HitRecord
{
	GLuint numNames;	// Number of names in the name stack for this hit record
	GLuint minDepth;	// Minimum depth value of primitives (range 0 to 2^32-1)
	GLuint maxDepth;	// Maximum depth value of primitives (range 0 to 2^32-1)
	GLuint contents;	// Name stack contents
};


void SelectGraphicObject( const float mousePx, const float mousePy, ArnSceneGraph* sceneGraph, ArnViewportData* avd, ArnCamera* cam )
{
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
	glLoadIdentity();

	/* restrict the draw to an area around the cursor */
	const GLdouble pickingAroundFactor = 1.0;
	gluPickMatrix(mousePx, mousePy, pickingAroundFactor, pickingAroundFactor, view);

	/* your original projection matrix */
	ArnMatrix projMat;
	ArnGetProjectionMatrix(&projMat, avd, cam, cml::right_handed);
	glMultMatrixf(reinterpret_cast<const GLfloat*>(projMat.m));

	/* Draw the objects onto the screen */
	glMatrixMode(GL_MODELVIEW);

	/* draw only the names in the stack, and fill the array */
	glFlush();
	SDL_GL_SwapBuffers();
	sceneGraph->render();

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
		ArnNode* node = sceneGraph->getNodeById(buff[h].contents);
		printf("[Object %p ID %d : %s]\n", static_cast<void*>(node), node->getObjectId(), node->getName());
	}
}

int HandleEvent(SDL_Event *event, ArnSceneGraph* sceneGraph, ArnViewportData* avd, ArnCamera* cam)
{
	int done;

	ArnSkeleton* skel = reinterpret_cast<ArnSkeleton*>(sceneGraph->findFirstNodeOfType(NDT_RT_SKELETON));


	done = 0;
	switch( event->type ) {
		case SDL_MOUSEBUTTONUP:
			{
				SelectGraphicObject(float(event->motion.x), float(avd->Height - event->motion.y), sceneGraph, avd, cam); // Y-coord flipped.

				float frustumPlanes[6][4];
				cml_mat44 cmlmv, cmlproj, cmlvp;
				cml_vec3 cmleye(cam->getCameraData().pos.x, cam->getCameraData().pos.y, cam->getCameraData().pos.z);
				cml_vec3 cmltarget(cam->getCameraData().targetPos.x, cam->getCameraData().targetPos.y, cam->getCameraData().targetPos.z);
				cml_vec3 cmlup(cam->getCameraData().upVector.x, cam->getCameraData().upVector.y, cam->getCameraData().upVector.z);
				cml_vec3 cmlcorners[8];
				cml::matrix_look_at_RH(cmlmv, cmleye, cmltarget, cmlup);
				cml::matrix_perspective_yfov_RH(cmlproj, cam->getFov(), (float)avd->Width / avd->Height, cam->getNearClip(), cam->getFarClip(), cml::z_clip_zero);
				cml::extract_frustum_planes(cmlmv, cmlproj, frustumPlanes, cml::z_clip_zero, false);
				cml::get_frustum_corners(frustumPlanes, cmlcorners);
				for (int i = 0; i < 8; ++i)
				{
					//printf("%.3f %.3f %.3f\n", cmlcorners[i][0], cmlcorners[i][1], cmlcorners[i][2]);
				}
				cml::matrix_viewport(cmlvp, 0.0f, 0.0f + avd->Width, 0.0f, 0.0f + avd->Height, cml::z_clip_zero);
				cml_vec3 cmlorigin, cmldir;
				cml::make_pick_ray(float(event->motion.x), float(avd->Height - event->motion.y), cmlmv, cmlproj, cmlvp, cmlorigin, cmldir);
				//printf("Ray origin   : %.3f %.3f %.3f\n", cmlorigin[0], cmlorigin[1], cmlorigin[2]);
				//printf("Ray direction: %.3f %.3f %.3f\n", cmldir[0], cmldir[1], cmldir[2]);

				ArnVec3 origin(cmlorigin[0], cmlorigin[1], cmlorigin[2]);
				ArnVec3 direction(cmldir[0], cmldir[1], cmldir[2]);
				ArnMesh* mesh = reinterpret_cast<ArnMesh*>(sceneGraph->findFirstNodeOfType(NDT_RT_MESH));
				bool bHit = false;
				unsigned int faceIdx = 0;
				ArnIntersectGl(mesh, &origin, &direction, &bHit, &faceIdx, 0, 0, 0, 0, 0);
				if (bHit)
					printf("Hit on Face %u\n", faceIdx);
			}
			break;

		/*
		case SDL_ACTIVEEVENT:

			printf( "app %s ", event->active.gain ? "gained" : "lost" );
			if ( event->active.state & SDL_APPACTIVE ) {
				printf( "active " );
			} else if ( event->active.state & SDL_APPMOUSEFOCUS ) {
				printf( "mouse " );
			} else if ( event->active.state & SDL_APPINPUTFOCUS ) {
				printf( "input " );
			}
			printf( "focus\n" );
			break;
		*/

		case SDL_KEYDOWN:
			if ( event->key.keysym.sym == SDLK_ESCAPE )
			{
				done = 1;
			}
			else if (event->key.keysym.sym == SDLK_1)
			{
				skel->getAnimCtrl()->SetTrackAnimationSet(0, 0);
				skel->getAnimCtrl()->SetTrackPosition(0, skel->getAnimCtrl()->GetTime());
				ARNTRACK_DESC desc;
				skel->getAnimCtrl()->GetTrackDesc(0, &desc);
				skel->getAnimCtrl()->SetTrackEnable(0, desc.Enable ? false : true);
				skel->getAnimCtrl()->SetTrackWeight(0, 1);
			}
			else if (event->key.keysym.sym == SDLK_2)
			{
				skel->getAnimCtrl()->SetTrackAnimationSet(1, 1);
				skel->getAnimCtrl()->SetTrackPosition(1, skel->getAnimCtrl()->GetTime());
				ARNTRACK_DESC desc;
				skel->getAnimCtrl()->GetTrackDesc(1, &desc);
				skel->getAnimCtrl()->SetTrackEnable(1, desc.Enable ? false : true);
				skel->getAnimCtrl()->SetTrackWeight(1, 1);
			}
			else if (event->key.keysym.sym == SDLK_3)
			{
				skel->getAnimCtrl()->SetTrackAnimationSet(2, 2);
				skel->getAnimCtrl()->SetTrackPosition(2, skel->getAnimCtrl()->GetTime());
				ARNTRACK_DESC desc;
				skel->getAnimCtrl()->GetTrackDesc(2, &desc);
				skel->getAnimCtrl()->SetTrackEnable(2, desc.Enable ? false : true);
				skel->getAnimCtrl()->SetTrackWeight(2, 1);
			}
			printf("key '%s' pressed\n",
				SDL_GetKeyName(event->key.keysym.sym));
			break;
		case SDL_QUIT:
			done = 1;
			break;
	}
	return(done);
}

void DrawString(const char *str, int x, int y, float color[4], void *font)
{
	glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
	glDisable(GL_LIGHTING);     // need to disable lighting for proper text color

	glColor4fv(color);          // set text color
	glRasterPos2i(x, y);        // place text position

	// loop all characters in the string
	while(*str)
	{
		//glutBitmapCharacter(font, *str);
		++str;
	}

	glEnable(GL_LIGHTING);
	glPopAttrib();
}

void RenderInfo(const ArnViewportData* viewport, unsigned int timeMs, unsigned int durationMs, GLuint fontTextureId)
{
	// backup current model-view matrix
	glPushMatrix();                     // save current modelview matrix
	glLoadIdentity();                   // reset modelview matrix

	// set to 2D orthogonal projection
	glMatrixMode(GL_PROJECTION);     // switch to projection matrix
	glPushMatrix();                  // save current projection matrix
	glLoadIdentity();                // reset projection matrix
	gluOrtho2D(viewport->X, viewport->Width, viewport->Y, viewport->Height);  // set to orthogonal projection

	std::stringstream ss;
	ss.str("");
	ss << std::fixed << std::setprecision(1);
	ss << 1.0f / ((float)durationMs / 1000) << " FPS" << std::ends; // update fps string
	ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
	//DrawString(ss.str().c_str(), 1, 1, color, GLUT_BITMAP_8_BY_13);

	ss.str("");
	ss << std::fixed << std::setprecision(3);
	ss << "Running time: " << ((float)durationMs / 1000) << " s" << std::ends; // update fps string
	ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
	//DrawString(ss.str().c_str(), 1, 14, color, GLUT_BITMAP_8_BY_13);

	glDisable(GL_LIGHTING);
	glBindTexture(GL_TEXTURE_2D, fontTextureId);
	glBlendFunc(GL_ONE, GL_ONE);
	glColor4d(1, 1, 1, 1);
	glScaled(256, 256, 1);
	glBegin(GL_QUADS);
	glTexCoord2d(1, 1); glVertex3d(1, 1, 0);
	glTexCoord2d(0, 1); glVertex3d(0, 1, 0);
	glTexCoord2d(0, 0); glVertex3d(0, 0, 0);
	glTexCoord2d(1, 0); glVertex3d(1, 0, 0);
	glEnd();
	glEnable(GL_LIGHTING);


	// restore projection matrix
	glPopMatrix();                   // restore to previous projection matrix

	// restore modelview matrix
	glMatrixMode(GL_MODELVIEW);      // switch to modelview matrix
	glPopMatrix();                   // restore to previous modelview matrix
}

GLuint ArnCreateTextureFromArrayGl( const unsigned char* data, int width, int height, bool wrap )
{
	GLuint texture;
	// allocate a texture name
	glGenTextures( 1, &texture );

	// select our current texture
	glBindTexture( GL_TEXTURE_2D, texture );

	// select modulate to mix texture with color for shading
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	// when texture area is small, bilinear filter the closest MIP map
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
	// when texture area is large, bilinear filter the first MIP map
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// if wrap is true, the texture wraps over at the edges (repeat)
	//       ... false, the texture ends at the edges (clamp)

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLfloat>(wrap ? GL_REPEAT : GL_CLAMP) );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLfloat>(wrap ? GL_REPEAT : GL_CLAMP) );

	// build our texture MIP maps
	gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data );
	return texture;
}

int main(int argc, char *argv[])
{
	const int windowWidth = 640;
	const int windowHeight = 480;
	const int bpp = 32;
	const int depthSize = 24;
	bool bFullScreen = false;
	bool bNoFrame = false;

	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		fprintf(stderr,"Couldn't initialize SDL: %s\n",SDL_GetError());
		exit( 1 );
	}

	/* Set the flags we want to use for setting the video mode */
	int video_flags = SDL_OPENGL;
	if (bFullScreen)
		video_flags |= SDL_FULLSCREEN;
	if (bNoFrame)
		video_flags |= SDL_NOFRAME;


	/* Initialize the display */
	int rgb_size[3];
	rgb_size[0] = 8;
	rgb_size[1] = 8;
	rgb_size[2] = 8;
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rgb_size[0] );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, rgb_size[1] );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, rgb_size[2] );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, depthSize );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
	SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 1 ); // Swap Control On --> Refresh rate to 60 Hz

	if ( SDL_SetVideoMode( windowWidth, windowHeight, bpp, video_flags ) == NULL ) {
		fprintf(stderr, "Couldn't set GL mode: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}
	printf("Screen BPP: %d\n", SDL_GetVideoSurface()->format->BitsPerPixel);
	printf("\n");
	printf( "Vendor     : %s\n", glGetString( GL_VENDOR ) );
	printf( "Renderer   : %s\n", glGetString( GL_RENDERER ) );
	printf( "Version    : %s\n", glGetString( GL_VERSION ) );
	printf( "Extensions : %s\n", glGetString( GL_EXTENSIONS ) );
	printf("\n");

	int value;
	SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &value );
	printf( "SDL_GL_RED_SIZE: requested %d, got %d\n", rgb_size[0],value);

	SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &value );
	printf( "SDL_GL_GREEN_SIZE: requested %d, got %d\n", rgb_size[1],value);

	SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &value );
	printf( "SDL_GL_BLUE_SIZE: requested %d, got %d\n", rgb_size[2],value);

	SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &value );
	printf( "SDL_GL_DEPTH_SIZE: requested %d, got %d\n", depthSize, value );

	SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &value );
	printf( "SDL_GL_DOUBLEBUFFER: requested 1, got %d\n", value );

	SDL_GL_GetAttribute( SDL_GL_ACCELERATED_VISUAL, &value );
	printf( "SDL_GL_ACCELERATED_VISUAL: requested 1, got %d\n", value );

	//SDL_GL_GetAttribute( SDL_GL_SWAP_CONTROL, &value );
	//printf( "SDL_GL_SWAP_CONTROL: requested 1, got %d\n", value );

#ifdef WIN32
	// get pointers to GL functions
	glGenBuffersARB				= (PFNGLGENBUFFERSARBPROC)SDL_GL_GetProcAddress("glGenBuffersARB");
	glBindBufferARB				= (PFNGLBINDBUFFERARBPROC)SDL_GL_GetProcAddress("glBindBufferARB");
	glBufferDataARB				= (PFNGLBUFFERDATAARBPROC)SDL_GL_GetProcAddress("glBufferDataARB");
	glBufferSubDataARB			= (PFNGLBUFFERSUBDATAARBPROC)SDL_GL_GetProcAddress("glBufferSubDataARB");
	glDeleteBuffersARB			= (PFNGLDELETEBUFFERSARBPROC)SDL_GL_GetProcAddress("glDeleteBuffersARB");
	glGetBufferParameterivARB	= (PFNGLGETBUFFERPARAMETERIVARBPROC)SDL_GL_GetProcAddress("glGetBufferParameterivARB");
	glMapBufferARB				= (PFNGLMAPBUFFERARBPROC)SDL_GL_GetProcAddress("glMapBufferARB");
	glUnmapBufferARB			= (PFNGLUNMAPBUFFERARBPROC)SDL_GL_GetProcAddress("glUnmapBufferARB");

	glActiveTextureARB			= (PFNGLACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glActiveTextureARB");
	glClientActiveTextureARB	= (PFNGLCLIENTACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glClientActiveTextureARB");
	glMultiTexCoord1dARB		= (PFNGLMULTITEXCOORD1DARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1dARB");
	glMultiTexCoord1dvARB		= (PFNGLMULTITEXCOORD1DVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1dvARB");
	glMultiTexCoord1fARB		= (PFNGLMULTITEXCOORD1FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1fARB");
	glMultiTexCoord1fvARB		= (PFNGLMULTITEXCOORD1FVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1fvARB");
	glMultiTexCoord1iARB		= (PFNGLMULTITEXCOORD1IARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1iARB");
	glMultiTexCoord1ivARB		= (PFNGLMULTITEXCOORD1IVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1ivARB");
	glMultiTexCoord1sARB		= (PFNGLMULTITEXCOORD1SARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1sARB");
	glMultiTexCoord1svARB		= (PFNGLMULTITEXCOORD1SVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1svARB");
	glMultiTexCoord2dARB		= (PFNGLMULTITEXCOORD2DARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2dARB");
	glMultiTexCoord2dvARB		= (PFNGLMULTITEXCOORD2DVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2dvARB");
	glMultiTexCoord2fARB		= (PFNGLMULTITEXCOORD2FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
	glMultiTexCoord2fvARB		= (PFNGLMULTITEXCOORD2FVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2fvARB");
	glMultiTexCoord2iARB		= (PFNGLMULTITEXCOORD2IARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2iARB");
	glMultiTexCoord2ivARB		= (PFNGLMULTITEXCOORD2IVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2ivARB");
	glMultiTexCoord2sARB		= (PFNGLMULTITEXCOORD2SARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2sARB");
	glMultiTexCoord2svARB		= (PFNGLMULTITEXCOORD2SVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2svARB");
	glMultiTexCoord3dARB		= (PFNGLMULTITEXCOORD3DARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3dARB");
	glMultiTexCoord3dvARB		= (PFNGLMULTITEXCOORD3DVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3dvARB");
	glMultiTexCoord3fARB		= (PFNGLMULTITEXCOORD3FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3fARB");
	glMultiTexCoord3fvARB		= (PFNGLMULTITEXCOORD3FVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3fvARB");
	glMultiTexCoord3iARB		= (PFNGLMULTITEXCOORD3IARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3iARB");
	glMultiTexCoord3ivARB		= (PFNGLMULTITEXCOORD3IVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3ivARB");
	glMultiTexCoord3sARB		= (PFNGLMULTITEXCOORD3SARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3sARB");
	glMultiTexCoord3svARB		= (PFNGLMULTITEXCOORD3SVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3svARB");
	glMultiTexCoord4dARB		= (PFNGLMULTITEXCOORD4DARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4dARB");
	glMultiTexCoord4dvARB		= (PFNGLMULTITEXCOORD4DVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4dvARB");
	glMultiTexCoord4fARB		= (PFNGLMULTITEXCOORD4FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4fARB");
	glMultiTexCoord4fvARB		= (PFNGLMULTITEXCOORD4FVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4fvARB");
	glMultiTexCoord4iARB		= (PFNGLMULTITEXCOORD4IARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4iARB");
	glMultiTexCoord4ivARB		= (PFNGLMULTITEXCOORD4IVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4ivARB");
	glMultiTexCoord4sARB		= (PFNGLMULTITEXCOORD4SARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4sARB");
	glMultiTexCoord4svARB		= (PFNGLMULTITEXCOORD4SVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4svARB");
#endif

	/* Set the window manager title bar */
	SDL_WM_SetCaption( "Realtime User Control Biped", "RUCB" );

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	//glShadeModel(GL_FLAT);							// Enable Smooth Shading
	glClearDepth(1.0f);									// Depth Buffer Setup
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	for (int lightId = 0; lightId < 8; ++lightId)
	{
		glDisable(GL_LIGHT0 + lightId);
	}


	FT_Library library;
	int ftError = FT_Init_FreeType(&library);
	if (ftError)
		fprintf(stderr, "FreeType init failed.\n");
	FT_Face face;
	ftError = FT_New_Face(library, "tahoma.ttf", 0, &face);
	if (ftError)
		fprintf(stderr, "FreeType new face creation failed.\n");
	ftError = FT_Set_Char_Size(face, 0, 16*64, 300, 300);
	if (ftError)
		fprintf(stderr, "FreeType face char size  failed.\n");
	ftError = FT_Set_Pixel_Sizes(face, 0, 16);
	if (ftError)
		fprintf(stderr, "FreeType face pixel size  failed.\n");

	FT_GlyphSlot slot = face->glyph; /* a small shortcut */
	FT_UInt glyph_index;
	int pen_x, pen_y;
	pen_x = 0;
	pen_y = 0;
	const wchar_t* testString = L"Realtime User Control Biped";
	size_t testStringLen = wcslen(testString);
	int textTextureSize = 256;
	unsigned char* fontTexture = (unsigned char*)malloc( textTextureSize * textTextureSize * 4 ); // RGBA texture
	memset(fontTexture, 0, textTextureSize * textTextureSize* 4);
	for ( size_t n = 0; n < testStringLen; n++ )
	{
		glyph_index = FT_Get_Char_Index( face, testString[n] ); /* load glyph image into the slot (erase previous one) */
		ftError = FT_Load_Char(face, testString[n], FT_LOAD_RENDER);
		if ( ftError )
			continue; /* ignore errors */

		for (int row = 1; row <= slot->bitmap.rows; ++row)
		{
			for (int w = 0; w < slot->bitmap.width; ++w)
			{
				const size_t fontTexOffset = 4 * (w + (row)*textTextureSize + pen_x + slot->bitmap_left);
				char slotValue = slot->bitmap.buffer[w + (slot->bitmap.rows - row) * slot->bitmap.width];
				fontTexture[fontTexOffset + 0] = slotValue; // RED color
				fontTexture[fontTexOffset + 1] = 0; // GREEN color
				fontTexture[fontTexOffset + 2] = 0; // BLUE color
				fontTexture[fontTexOffset + 3] = 0xff; // ALPHA
			}
		}
		pen_x += slot->advance.x >> 6;
	}
	GLuint fontTextureId = ArnCreateTextureFromArrayGl(fontTexture, textTextureSize, textTextureSize, false);

	ilInit();

	// Create and init the scene graph instance from XML file
	// and attach that one to the video manager.
	InitializeXmlParser();
	ArnXmlString axs;
	if (argc != 2)
	{
		fprintf(stderr, " *** Provide XML scene file path as the first argument.\n");
		return -9;
	}
	ArnSceneGraph* sceneGraph = ArnSceneGraph::createFrom(argv[1]);
	if (!sceneGraph)
	{
		fprintf(stderr, " *** Scene graph is not loaded correctly. Check your input XML scene file.\n");
		return -5;
	}
	sceneGraph->interconnect(sceneGraph);
	sceneGraph->initRendererObjects();
	ArnCamera* cam = reinterpret_cast<ArnCamera*>(sceneGraph->findFirstNodeOfType(NDT_RT_CAMERA));
	if (!cam)
		cam = ArnCamera::createFrom("Auto-generated Camera", ArnQuat::createFromEuler(0, 0, 0), ArnVec3(0, 0, 30), (float)(ARN_PI / 4));

	ArnViewportData avd;
	avd.X = 0;
	avd.Y = 0;
	avd.Width = windowWidth;
	avd.Height = windowHeight;
	avd.MinZ = 0;
	avd.MaxZ = 1.0f;
	cam->recalcLocalXform();
	ArnConfigureViewportProjectionMatrixGl(&avd, cam);
	ArnConfigureViewMatrixGl(cam);

	ArnLight* light = reinterpret_cast<ArnLight*>(sceneGraph->findFirstNodeOfType(NDT_RT_LIGHT));
	assert(light);
	ArnConfigureLightGl(0, light);

	ArnMesh* mesh = reinterpret_cast<ArnMesh*>(sceneGraph->findFirstNodeOfType(NDT_RT_MESH));
	assert(mesh);
	unsigned int vertCount = mesh->getVertCount(0);
	printf("====== First Mesh Vertices =======\n");
	for (unsigned int v = 0; v < vertCount; ++v)
	{
		ArnVec3 pos;
		mesh->getVert(&pos, 0, 0, 0, v);
		printf("[%d] ", v); pos.printFormatString();



	}
	const unsigned int faceGroupCount = mesh->getFaceGroupCount();
	for (unsigned int fg = 0; fg < faceGroupCount; ++fg)
	{
		unsigned int triCount, quadCount;
		mesh->getFaceCount(triCount, quadCount, fg);
		for (unsigned int tc = 0; tc < triCount; ++tc)
		{
			unsigned int totalIndex;
			unsigned int tinds[3];
			mesh->getTriFace(totalIndex, tinds, fg, tc);
			printf("Tri OrigInd/VertInds: %d / %d %d %d\n", totalIndex, tinds[0], tinds[1], tinds[2]);
		}
	}
	printf("====== First Mesh Vertices End =======\n");


	ArnMatrix modelview, projection;
	glGetFloatv(GL_MODELVIEW_MATRIX, reinterpret_cast<GLfloat*>(modelview.m));
	glGetFloatv(GL_PROJECTION_MATRIX, reinterpret_cast<GLfloat*>(projection.m));
	float frustumPlanes[6][4];
	ArnExtractFrustumPlanes(frustumPlanes, &modelview, &projection);
	ArnVec3 frustumCorners[8];
	ArnGetFrustumCorners(frustumCorners, frustumPlanes);

	for (int i = 0; i < 8; ++i)
	{
		frustumCorners[i].printFormatString();
	}

	cml_mat44 cmlmv, cmlproj, cmlvp;
	cml_vec3 cmleye(0, 0, 5), cmltarget(0, 0, 4), cmlup(0, 1, 0);
	cml_vec3 cmlcorners[8];
	cml::matrix_look_at_RH(cmlmv, cmleye, cmltarget, cmlup);
	cml::matrix_perspective_yfov_RH(cmlproj, ArnToRadian(45), (float)avd.Width / avd.Height, 3.5f, 10.0f, cml::z_clip_zero);
	cml::extract_frustum_planes(cmlmv, cmlproj, frustumPlanes, cml::z_clip_zero, false);
	cml::get_frustum_corners(frustumPlanes, cmlcorners);
	for (int i = 0; i < 8; ++i)
	{
		printf("%.3f %.3f %.3f\n", cmlcorners[i][0], cmlcorners[i][1], cmlcorners[i][2]);
	}
	cml::matrix_viewport(cmlvp, 0.0f, 0.0f + avd.Width, 0.0f, 0.0f + avd.Height, cml::z_clip_zero);
	cml_vec3 origin, dir;
	cml::make_pick_ray(100.0f, 100.0f, cmlmv, cmlproj, cmlvp, origin, dir);
	printf("Ray origin   : %.3f %.3f %.3f\n", origin[0], origin[1], origin[2]);
	printf("Ray direction: %.3f %.3f %.3f\n", dir[0], dir[1], dir[2]);

	GLuint norCubeMap = ArnCreateNormalizationCubeMapGl();

	/* Loop until done. */
	unsigned int start_time = SDL_GetTicks();
	unsigned int frames = 0;
	int done = 0;
	unsigned int frameStartMs = 0;
	unsigned int frameDurationMs = 0;
	unsigned int frameEndMs = 0;
	while( !done )
	{
		frameDurationMs = frameEndMs - frameStartMs;
		frameStartMs = SDL_GetTicks();
		GLenum gl_error;
		SDL_Event event;
		char* sdl_error;

		/* Do our drawing, too. */
		glClearColor( 0.5, 0.5, 0.5, 1.0 );
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, 0);
		glPushMatrix();
		{
			sceneGraph->render();
			sceneGraph->update((double)SDL_GetTicks() / 1000, (float)frameDurationMs / 1000);
			RenderInfo(&avd, SDL_GetTicks(), frameDurationMs, fontTextureId);
		}
		glPopMatrix();
		SDL_GL_SwapBuffers();

		/* Check for error conditions. */
		gl_error = glGetError( );

		if( gl_error != GL_NO_ERROR ) {
			fprintf( stderr, "testgl: OpenGL error: %d\n", gl_error );
		}

		sdl_error = SDL_GetError( );

		if( sdl_error[0] != '\0' ) {
			fprintf(stderr, "testgl: SDL error '%s'\n", sdl_error);
			SDL_ClearError();
		}

		/* Allow the user to see what's happening */

		/* Check if there's a pending event. */
		while( SDL_PollEvent( &event ) ) {
			done = HandleEvent(&event, sceneGraph, &avd, cam);
		}
		++frames;
		frameEndMs = SDL_GetTicks();
	}

	/* Print out the frames per second */
	unsigned int this_time = SDL_GetTicks();
	if ( this_time != start_time ) {
		printf("%2.2f FPS\n",
			((float)frames/(this_time-start_time))*1000.0);
	}

	delete cam;
	cam = 0;
	delete sceneGraph;
	sceneGraph = 0;

	glDeleteTextures(1, &fontTextureId);

	/* Destroy our GL context, etc. */
	SDL_Quit();
	return 0;
}
