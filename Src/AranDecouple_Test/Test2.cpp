#include "Test2.h"

struct HitRecord
{
	GLuint numNames;	// Number of names in the name stack for this hit record
	GLuint minDepth;	// Minimum depth value of primitives (range 0 to 2^32-1)
	GLuint maxDepth;	// Maximum depth value of primitives (range 0 to 2^32-1)
	GLuint contents;	// Name stack contents
};

enum MessageHandleResult
{
	MHR_DO_NOTHING,
	MHR_EXIT_APP,
	MHR_NEXT_SCENE,
	MHR_RELOAD_SCENE
};

static void
SelectGraphicObject( const float mousePx, const float mousePy, ArnSceneGraph* sceneGraph, ArnViewportData* avd, ArnCamera* cam )
{
	if (!sceneGraph)
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
	glLoadIdentity();

	/* restrict the draw to an area around the cursor */
	const GLdouble pickingAroundFactor = 1.0;
	gluPickMatrix(mousePx, mousePy, pickingAroundFactor, pickingAroundFactor, view);

	/* your original projection matrix */
	ArnMatrix projMat;
	ArnGetProjectionMatrix(&projMat, avd, cam, true);
	glMultTransposeMatrixf(reinterpret_cast<const GLfloat*>(projMat.m));

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
		assert(node);
		printf("[Object 0x%p ID %d : %s]\n", static_cast<void*>(node), node->getObjectId(), node->getName());

		ArnMesh* mesh = dynamic_cast<ArnMesh*>(node);
		if (mesh)
		{
			ArnVec3 dim;
			mesh->getBoundingBoxDimension(&dim, true);
			printf("Mesh Dimension: "); dim.printFormatString();
		}
	}
}

static MessageHandleResult
HandleEvent(SDL_Event* event, ArnSceneGraph* curSceneGraph, ArnViewportData* avd)
{
	MessageHandleResult done = MHR_DO_NOTHING;
	ArnSkeleton* skel = 0;
	ArnCamera* activeCam = 0;
	if (curSceneGraph)
	{
		activeCam = reinterpret_cast<ArnCamera*>(curSceneGraph->findFirstNodeOfType(NDT_RT_CAMERA));
		skel = reinterpret_cast<ArnSkeleton*>(curSceneGraph->findFirstNodeOfType(NDT_RT_SKELETON));
	}
	switch( event->type ) {
		case SDL_MOUSEBUTTONUP:
			{
				if (event->button.button == SDL_BUTTON_LEFT)
				{
					SelectGraphicObject(float(event->motion.x), float(avd->Height - event->motion.y), curSceneGraph, avd, activeCam); // Y-coord flipped.

					if (curSceneGraph)
					{
						ArnMatrix modelview, projection;
						glGetFloatv(GL_MODELVIEW_MATRIX, reinterpret_cast<GLfloat*>(modelview.m));
						modelview = modelview.transpose();
						glGetFloatv(GL_PROJECTION_MATRIX, reinterpret_cast<GLfloat*>(projection.m));
						projection = projection.transpose();
						ArnVec3 origin, direction;
						ArnMakePickRay(&origin, &direction, float(event->motion.x), float(avd->Height - event->motion.y), &modelview, &projection, avd);
						ArnMesh* mesh = reinterpret_cast<ArnMesh*>(curSceneGraph->findFirstNodeOfType(NDT_RT_MESH));
						if (mesh)
						{
							bool bHit = false;
							unsigned int faceIdx = 0;
							ArnIntersectGl(mesh, &origin, &direction, &bHit, &faceIdx, 0, 0, 0, 0, 0);
							if (bHit)
								printf("Hit on Face %u of mesh %s\n", faceIdx, mesh->getName());
						}
					}
				}
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
				done = MHR_EXIT_APP;
			}
			else if (event->key.keysym.sym == SDLK_1)
			{
				if (skel && skel->getAnimCtrl() && skel->getAnimCtrl()->getTrackCount() > 0)
				{
					skel->getAnimCtrl()->SetTrackAnimationSet(0, 0);
					skel->getAnimCtrl()->SetTrackPosition(0, skel->getAnimCtrl()->GetTime());
					ARNTRACK_DESC desc;
					skel->getAnimCtrl()->GetTrackDesc(0, &desc);
					skel->getAnimCtrl()->SetTrackEnable(0, desc.Enable ? false : true);
					skel->getAnimCtrl()->SetTrackWeight(0, 1);
				}
				
			}
			else if (event->key.keysym.sym == SDLK_2)
			{
				if (skel && skel->getAnimCtrl() && skel->getAnimCtrl()->getTrackCount() > 1)
				{
					skel->getAnimCtrl()->SetTrackAnimationSet(1, 1);
					skel->getAnimCtrl()->SetTrackPosition(1, skel->getAnimCtrl()->GetTime());
					ARNTRACK_DESC desc;
					skel->getAnimCtrl()->GetTrackDesc(1, &desc);
					skel->getAnimCtrl()->SetTrackEnable(1, desc.Enable ? false : true);
					skel->getAnimCtrl()->SetTrackWeight(1, 1);
				}
			}
			else if (event->key.keysym.sym == SDLK_3)
			{
				if (skel && skel->getAnimCtrl() && skel->getAnimCtrl()->getTrackCount() > 2)
				{
					skel->getAnimCtrl()->SetTrackAnimationSet(2, 2);
					skel->getAnimCtrl()->SetTrackPosition(2, skel->getAnimCtrl()->GetTime());
					ARNTRACK_DESC desc;
					skel->getAnimCtrl()->GetTrackDesc(2, &desc);
					skel->getAnimCtrl()->SetTrackEnable(2, desc.Enable ? false : true);
					skel->getAnimCtrl()->SetTrackWeight(2, 1);
				}
			}
			else if (event->key.keysym.sym == SDLK_n)
			{
				done = MHR_NEXT_SCENE;
			}
			else if (event->key.keysym.sym == SDLK_r)
			{
				done = MHR_RELOAD_SCENE;
			}
			printf("key '%s' pressed\n",
				SDL_GetKeyName(event->key.keysym.sym));
			break;
		case SDL_QUIT:
			done = MHR_EXIT_APP;
			break;
	}
	return done;
}

static void
DrawString(const char *str, int x, int y, float color[4], void *font)
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

static void
RenderInfo(const ArnViewportData* viewport, unsigned int timeMs, unsigned int durationMs, GLuint fontTextureId)
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

static GLuint
ArnCreateTextureFromArrayGl( const unsigned char* data, int width, int height, bool wrap )
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

static void
PrintMeshVertexList(const ArnMesh* mesh)
{
	if (!mesh)
		return;
	unsigned int vertCount = mesh->getTotalVertCount();
	printf("====== First Mesh Vertex List =======\n");
	for (unsigned int v = 0; v < vertCount; ++v)
	{
		ArnVec3 pos;
		mesh->getVert(&pos, 0, 0, v, false);
		printf("[%d] ", v);
		pos.printFormatString();
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
	printf("====== First Mesh Vertex List End =======\n");
}

static void
PrintFrustumCornersBasedOnGlMatrixStack(const ArnViewportData* avd)
{
	float frustumPlanes[6][4];
	ArnVec3 frustumCorners[8];
	ArnMatrix modelview, projection;
	glGetFloatv(GL_MODELVIEW_MATRIX, reinterpret_cast<GLfloat*>(modelview.m));
	modelview = modelview.transpose();
	glGetFloatv(GL_PROJECTION_MATRIX, reinterpret_cast<GLfloat*>(projection.m));
	projection = projection.transpose();
	ArnExtractFrustumPlanes(frustumPlanes, &modelview, &projection);
	ArnGetFrustumCorners(frustumCorners, frustumPlanes);
	printf("=== Eight Frustum Corder points (calculated from OpenGL matrix stacks) ===\n");
	for (int i = 0; i < 8; ++i)
	{
		frustumCorners[i].printFormatString();
	}
	printf("=== Eight Frustum Corder Points End ===\n");

	printf("     === Test Ray on center screen coordinates ===\n");
	ArnVec3 origin, dir;
	ArnMakePickRay(&origin, &dir, avd->Width / 2.0f, avd->Height / 2.0f, &modelview, &projection, avd);
	printf("Ray origin   : "); origin.printFormatString();
	printf("Ray direction: "); dir.printFormatString();
	printf("     === Test Ray End\n");
}

static void
PrintFrustumCornersBasedOnActiveCamera(const ArnCamera* activeCam, const ArnViewportData* avd)
{
	float frustumPlanes[6][4];
	ArnVec3 frustumCorners[8];
	ArnMatrix camModelview, camProjection;
	ArnVec3 eye(activeCam->getLocalXform().m[0][3], activeCam->getLocalXform().m[1][3], activeCam->getLocalXform().m[2][3]);
	ArnVec3 at(activeCam->getLocalXform().m[0][3]-activeCam->getLocalXform().m[0][2], activeCam->getLocalXform().m[1][3]-activeCam->getLocalXform().m[1][2], activeCam->getLocalXform().m[2][3]-activeCam->getLocalXform().m[2][2]);
	ArnVec3 up(activeCam->getLocalXform().m[0][1], activeCam->getLocalXform().m[1][1], activeCam->getLocalXform().m[2][1]);
	ArnMatrixLookAtRH(&camModelview, &eye, &at, &up);
	ArnMatrixPerspectiveYFov(&camProjection, activeCam->getFov(), (float)avd->Width / avd->Height, activeCam->getNearClip(), activeCam->getFarClip(), true);
	ArnExtractFrustumPlanes(frustumPlanes, &camModelview, &camProjection);
	ArnGetFrustumCorners(frustumCorners, frustumPlanes);
	printf("=== Eight Frustum Corder points (calculated from CML routines) ===\n");
	for (int i = 0; i < 8; ++i)
	{
		frustumCorners[i].printFormatString();
	}
	printf("=== Eight Frustum Corder Points End ===\n");

	printf("     === Test Ray on center screen coordinates ===\n");
	ArnVec3 origin, dir;
	ArnMakePickRay(&origin, &dir, avd->Width / 2.0f, avd->Height / 2.0f, &camModelview, &camProjection, avd);
	printf("Ray origin   : "); origin.printFormatString();
	printf("Ray direction: "); dir.printFormatString();
	printf("     === Test Ray End\n");
}

static void
PrintRayCastingResultUsingGlu(const ArnViewportData* avd)
{
	GLdouble glmv[16], glproj[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, glmv);
	glGetDoublev(GL_PROJECTION_MATRIX, glproj);
	GLdouble objx, objy, objz;
	GLint viewport[4] = { avd->X, avd->Y, avd->Width, avd->Height };
	gluUnProject(avd->Width / 2.0, avd->Height / 2.0, 0, glmv, glproj, viewport, &objx, &objy, &objz);
	printf("gluUnproject of center screen point result: %.3f, %.3f, %.3f\n", objx, objy, objz);
}

static int
ConfigureTestScene(ArnSceneGraph*& curSceneGraph, ArnCamera*& activeCam, ArnLight*& activeLight, const char* sceneFileName, const ArnViewportData* avd)
{
	assert(curSceneGraph || activeCam || activeLight == 0);

	curSceneGraph = ArnSceneGraph::createFrom(sceneFileName);
	if (!curSceneGraph)
	{
		fprintf(stderr, " *** Scene graph file %s is not loaded correctly.\n", sceneFileName);
		fprintf(stderr, "     Check your input XML scene file.\n");
		return -5;
	}
	curSceneGraph->interconnect(curSceneGraph);
	curSceneGraph->initRendererObjects();

	// Camera setting
	activeCam = reinterpret_cast<ArnCamera*>(curSceneGraph->findFirstNodeOfType(NDT_RT_CAMERA));
	assert(activeCam);
	activeCam->recalcLocalXform();
	activeCam->recalcAnimLocalXform();
	activeCam->printCameraOrientation();
	ArnConfigureViewportProjectionMatrixGl(avd, activeCam); // Projection matrix is not changed during runtime for now.
	ArnConfigureViewMatrixGl(activeCam);

	activeLight = reinterpret_cast<ArnLight*>(curSceneGraph->findFirstNodeOfType(NDT_RT_LIGHT));
	assert(activeLight);
	std::cout << "   Scene file " << sceneFileName << " loaded successfully." << std::endl;
	return 0;
}

static int
ConfigureNextTestSceneWithRetry(ArnSceneGraph*& curSceneGraph, ArnCamera*& activeCam, ArnLight*& activeLight, int& curSceneIndex, int nextSceneIndex, const std::vector<std::string>& sceneList,  const ArnViewportData& avd)
{
	assert(nextSceneIndex < (int)sceneList.size());
	delete curSceneGraph;
	curSceneIndex = nextSceneIndex;
	curSceneGraph = 0;
	activeCam = 0;
	activeLight = 0;
	unsigned int retryCount = 0;
	while (ConfigureTestScene(curSceneGraph, activeCam, activeLight, sceneList[curSceneIndex].c_str(), &avd) < 0)
	{
		curSceneIndex = (curSceneIndex + 1) % sceneList.size();
		++retryCount;
		if (retryCount >= sceneList.size())
		{
			std::cerr << " *** All provided scene files have errors." << std::endl;
			return -12;
		}
	}

	// Note: Scene loaded successfully.

	//
	// DEBUG PURPOSE
	// A little test on modelview, projection matrix
	// by checking frustum corner points and ray casting.
	//

	//PrintMeshVertexList(reinterpret_cast<ArnMesh*>(curSceneGraph->findFirstNodeOfType(NDT_RT_MESH)));
	PrintFrustumCornersBasedOnGlMatrixStack(&avd);
	PrintFrustumCornersBasedOnActiveCamera(activeCam, &avd);
	PrintRayCastingResultUsingGlu(&avd);
	return 0;
}

static int
ReloadCurrentScene(ArnSceneGraph*& curSceneGraph, ArnCamera*& activeCam, ArnLight*& activeLight, int& curSceneIndex, const std::vector<std::string>& sceneList,  const ArnViewportData& avd)
{
	return ConfigureNextTestSceneWithRetry(curSceneGraph, activeCam, activeLight, curSceneIndex, curSceneIndex, sceneList, avd);
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
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
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

	if (ArnInitGlExtFunctions() < 0)
	{
		std::cerr << "OpenGL extensions needed to run this program are not available." << std::endl;
	}

	/* Set the window manager title bar */
	SDL_WM_SetCaption( "aran", "aran" );

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
	wchar_t testString[128];
	swprintf(testString, 128, L"build %ld", ArnGetBuildCount());
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

	

	// Create and init the scene graph instance from XML file
	// and attach that one to the video manager.
	ArnInitializeXmlParser();
	ArnInitializeImageLibrary();

	ArnViewportData avd;
	avd.X = 0;
	avd.Y = 0;
	avd.Width = windowWidth;
	avd.Height = windowHeight;
	avd.MinZ = 0;
	avd.MaxZ = 1.0f;

	std::vector<std::string> sceneList;
	std::ifstream sceneListStream("SceneList.txt");
	std::string sceneFile;
	if (!sceneListStream.is_open())
	{
		fprintf(stderr, " *** SceneList.txt file corrupted or not available. Aborting...\n");
		return -12;
	}
	while (std::getline(sceneListStream, sceneFile))
	{
		sceneList.push_back(sceneFile);
	}
	
	ArnSceneGraph* curSceneGraph = 0;
	ArnCamera* activeCam = 0;
	ArnLight* activeLight = 0;
	int curSceneIndex = -1;
	if (sceneList.size() > 0)
	{
		if (ConfigureNextTestSceneWithRetry(curSceneGraph, activeCam, activeLight, curSceneIndex, 0, sceneList, avd) < 0)
		{
			std::cerr << " *** Aborting..." << std::endl;
			return -11;
		}
	}
	else
	{
		std::cerr << " *** No scene file available on scene list file. Aborting..." << std::endl;
		return -19;
	}
	
	// TODO: Normalized cube map for normal mapping
	//GLuint norCubeMap = ArnCreateNormalizationCubeMapGl();

	/* Loop until done. */
	unsigned int start_time = SDL_GetTicks();
	unsigned int frames = 0;
	MessageHandleResult done = MHR_DO_NOTHING;
	unsigned int frameStartMs = 0;
	unsigned int frameDurationMs = 0;
	unsigned int frameEndMs = 0;
	while( done != MHR_EXIT_APP )
	{
		frameDurationMs = frameEndMs - frameStartMs;
		frameStartMs = SDL_GetTicks();
		GLenum gl_error;
		SDL_Event event;
		char* sdl_error;

		/* Do our drawing, too. */
		glClearColor( 0.5, 0.5, 0.5, 1.0 );
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		if (activeCam)
			ArnConfigureViewMatrixGl(activeCam);
		if (activeLight)
			ArnConfigureLightGl(0, activeLight);
		
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, 0);

		glPushMatrix();
		{
			if (curSceneGraph)
			{
				curSceneGraph->render();
				curSceneGraph->update((double)SDL_GetTicks() / 1000, (float)frameDurationMs / 1000);
			}			
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
			done = HandleEvent(&event, curSceneGraph, &avd);

			if (done == MHR_NEXT_SCENE)
			{
				int nextSceneIndex = (curSceneIndex + 1) % sceneList.size();
				if (ConfigureNextTestSceneWithRetry(curSceneGraph, activeCam, activeLight, curSceneIndex, nextSceneIndex, sceneList, avd) < 0)
				{
					std::cerr << " *** Aborting..." << std::endl;
					done = MHR_EXIT_APP;
				}
			}
			else if (done == MHR_RELOAD_SCENE)
			{
				if (ReloadCurrentScene(curSceneGraph, activeCam, activeLight, curSceneIndex, sceneList, avd) < 0)
				{
					std::cerr << " *** Aborting..." << std::endl;
					done = MHR_EXIT_APP;
				}
			}
		}
		++frames;
		frameEndMs = SDL_GetTicks();
	}

	/* Print out the frames per second */
	unsigned int this_time = SDL_GetTicks();
	if ( this_time != start_time ) {
		printf("%2.2f FPS\n", ((float)frames/(this_time-start_time))*1000.0);
	}

	delete curSceneGraph;
	curSceneGraph = 0;

	glDeleteTextures(1, &fontTextureId);
	ArnCleanupXmlParser();
	ArnCleanupImageLibrary();

	/* Destroy our GL context, etc. */
	SDL_Quit();
	return 0;
}
