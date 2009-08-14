#include "IkTest.h"

static float gs_linVelX = 0;
static float gs_linVelZ = 0;
static float gs_torque = 0;
static float gs_torqueAnkle = 0;

static void
SelectGraphicObject( const float mousePx, const float mousePy, ArnSceneGraphPtr sceneGraph, std::vector<ArnIkSolver*>& ikSolvers, const ArnViewportData* avd, ArnCamera* cam )
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
	ArnSceneGraphRenderGl(sceneGraph.get());
	foreach (ArnIkSolver* ikSolver, ikSolvers)
	{
		TreeDraw(*ikSolver->getTree());
	}

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
			const ArnNode* node = sceneGraph->getConstNodeById(buff[h].contents);
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
			
			foreach (ArnIkSolver* ikSolver, ikSolvers)
			{
				NodePtr node = ikSolver->getNodeByObjectId(buff[h].contents);
				if (node)
				{
					printf("[Object 0x%p ID %d %s]\n",
						node.get(), node->getObjectId(), node->getName());
					ikSolver->reconfigureRoot(node);
				}
			}
		}
	}
}

static MessageHandleResult
HandleEvent(SDL_Event* event, ArnSceneGraphPtr curSceneGraph, std::vector<ArnIkSolver*>& ikSolvers, const ArnViewportData* avd, SimWorldPtr swPtr)
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
		case SDL_JOYAXISMOTION:
		{
			//std::cout << "Type " << (int)event->jaxis.type << " / Which " << (int)event->jaxis.which << " axis " << (int)event->jaxis.axis << " / value " << (int)event->jaxis.value << std::endl;
			if ((int)event->jaxis.axis == 0)
			{
				if (abs(event->jaxis.value) > 9000)
				{
					gs_linVelX = float(event->jaxis.value / 16000.0);
				}
				else
				{
					gs_linVelX = 0;
				}
			}
			if ((int)event->jaxis.axis == 1)
			{
				if (abs(event->jaxis.value) > 9000)
				{
					gs_linVelZ = float(-event->jaxis.value / 16000.0);

				}
				else
				{
					gs_linVelZ = 0;
				}
			}
			GeneralBodyPtr gbPtr = swPtr->getBodyByNameFromSet("EndEffector");
			if (gbPtr)
			{
				gbPtr->setLinearVel(gs_linVelX, 0, gs_linVelZ);
			}
			/////////////////////////////////////////////////////////////////////////
			if ((int)event->jaxis.axis == 2)
			{
				if (abs(event->jaxis.value) > 9000)
				{
					gs_torque = event->jaxis.value / 10.0f;
				}
				else
				{
					gs_torque = 0;
				}
			}

			if ((int)event->jaxis.axis == 5)
			{
				if (abs(event->jaxis.value) > 10)
				{
					gs_torqueAnkle = event->jaxis.value / 500.0f;
				}
				else
				{
					gs_torqueAnkle = 0;
				}
			}
		}
		break;
		case SDL_JOYBALLMOTION:
		{
			std::cout << "SDL_JOYBALLMOTION" << std::endl;
		}
		break;
		case SDL_JOYHATMOTION:
		{
			std::cout << "SDL_JOYHATMOTION" << std::endl;
		}
		break;
        case SDL_JOYBUTTONDOWN:
        {
        	std::cout << "SDL_JOYBUTTONDOWN" << std::endl;
        }
        break;
        case SDL_JOYBUTTONUP:
        {
        	std::cout << "SDL_JOYBUTTONUP" << std::endl;
        	std::cout << "gs_torque =  " << gs_torque << std::endl;
        	std::cout << "gs_linVelX = " << gs_linVelX << std::endl;
        	std::cout << "gs_linVelZ = " << gs_linVelZ << std::endl;
        }
        break;
		case SDL_MOUSEBUTTONUP:
			{
				if (event->button.button == SDL_BUTTON_LEFT)
				{
					SelectGraphicObject(float(event->motion.x), float(avd->Height - event->motion.y),
						curSceneGraph, ikSolvers, avd, activeCam); // Y-coord flipped.

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
		case SDL_KEYDOWN:
			if ( event->key.keysym.sym == SDLK_ESCAPE )
			{
				done = MHR_EXIT_APP;
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

static ArnSceneGraphPtr
ConfigureTestScene(const char* sceneFileName, const ArnViewportData* avd)
{
	ArnSceneGraphPtr ret(ArnSceneGraph::createFrom(sceneFileName));
	if (!ret)
	{
		fprintf(stderr, " *** Scene graph file %s is not loaded correctly.\n", sceneFileName);
		fprintf(stderr, "     Check your input XML scene file.\n");
		return ArnSceneGraphPtr();
	}
	ret->interconnect(ret.get());
	std::cout << "   Scene file " << sceneFileName << " loaded successfully." << std::endl;
	return ret;
}

static ArnSceneGraphPtr
ConfigureNextTestSceneWithRetry(int& curSceneIndex, int nextSceneIndex, const std::vector<std::string>& sceneList,  const ArnViewportData& avd)
{
	assert(nextSceneIndex < (int)sceneList.size());
	curSceneIndex = nextSceneIndex;
	unsigned int retryCount = 0;
	ArnSceneGraphPtr ret;
	while (!ret)
	{
		ret = ConfigureTestScene(sceneList[curSceneIndex].c_str(), &avd);
		if (ret)
		{
			return ret;
		}
		else
		{
			curSceneIndex = (curSceneIndex + 1) % sceneList.size();
			++retryCount;
		}
		if (retryCount >= sceneList.size())
		{
			std::cerr << " *** All provided scene files have errors." << std::endl;
			return ArnSceneGraphPtr();
		}
	}
	return ret;
}

static ArnSceneGraphPtr
ReloadCurrentScene(int& curSceneIndex, const std::vector<std::string>& sceneList,  const ArnViewportData& avd)
{
	return ConfigureNextTestSceneWithRetry(curSceneIndex, curSceneIndex, sceneList, avd);
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
GetActiveCamAndLight(ArnCamera*& activeCam, ArnLight*& activeLight, ArnSceneGraph* sg)
{
	activeCam = reinterpret_cast<ArnCamera*>(sg->findFirstNodeOfType(NDT_RT_CAMERA));
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
	if (!sceneListStream.is_open())
	{
		fprintf(stderr, " *** SceneList.txt file corrupted or not available.\n");
		Cleanup();
		return -12;
	}
	int sceneCount = 0;
	while (std::getline(sceneListStream, sceneFile))
	{
		sceneList.push_back(sceneFile);
		++sceneCount;
	}
	return sceneCount;
}

int
DoMain()
{
	int							curSceneIndex		= -1;
	std::vector<std::string>	sceneList;
	ArnSceneGraphPtr			curSgPtr;
	std::vector<Node*>			node;

	//ConfigureJacobianTree(tree, jacob, node);

	ArnInitializeXmlParser();
	ArnInitializeImageLibrary();
	std::cout << " INFO  Raw pointer    size = " << sizeof(ArnSceneGraph*) << std::endl;
	std::cout << " INFO  Shared pointer size = " << sizeof(ArnSceneGraphPtr) << std::endl;

	if (LoadSceneList(sceneList) < 0)
	{
		std::cerr << " *** Aborting..." << std::endl;
		Cleanup();
		return -113;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////

	const int					windowWidth			= 800;
	const int					windowHeight		= 600;
	const int					bpp					= 32;
	const int					depthSize			= 24;
	bool						bFullScreen			= false;
	bool						bNoFrame			= false;
	ArnViewportData				avd;
	SimWorldPtr					swPtr;

	avd.X		= 0;
	avd.Y		= 0;
	avd.Width	= windowWidth;
	avd.Height	= windowHeight;
	avd.MinZ	= 0;
	avd.MaxZ	= 1.0f;

	// Load first scene file into memory.
	assert(sceneList.size() > 0);
	assert(curSceneIndex == -1);
	curSgPtr = ConfigureNextTestSceneWithRetry(curSceneIndex, 0, sceneList, avd);
	if (!curSgPtr)
	{
		std::cerr << " *** Aborting..." << std::endl;
		Cleanup();
		return -11;
	}
	assert(curSgPtr);
	ArnCamera* activeCam = 0;
	ArnLight* activeLight = 0;
	std::vector<ArnIkSolver*> ikSolvers;

	// Initialize renderer-independent data in scene graph objects.
	swPtr.reset(SimWorld::createFrom(curSgPtr.get()));
	GetActiveCamAndLight(activeCam, activeLight, curSgPtr.get());
	foreach (ArnIkSolver* ikSolver, ikSolvers) { delete ikSolver; }
	ikSolvers.clear();
	ArnCreateArnIkSolversOnSceneGraph(ikSolvers, curSgPtr);

	// SDL Window init start
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		Cleanup();
		return -304;
	}
	SDL_JoystickOpen(0);

	/* Set the flags we want to use for setting the video mode */
	int video_flags = SDL_OPENGL;
	if (bFullScreen)
		video_flags |= SDL_FULLSCREEN;
	if (bNoFrame)
		video_flags |= SDL_NOFRAME;

	/* Initialize the display */
	int rgb_size[3] = { 8, 8, 8 };
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rgb_size[0] );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, rgb_size[1] );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, rgb_size[2] );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, depthSize );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
	// Swap Control On --> Refresh rate not exceeds Vsync(60 Hz mostly)
	SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 1 );

	if ( SDL_SetVideoMode( windowWidth, windowHeight, bpp, video_flags ) == NULL ) {
		fprintf(stderr, "Couldn't set GL mode: %s\n", SDL_GetError());
		Cleanup();
		SDL_Quit();
		return -2039;
	}
	// OpenGL context available from this line.

	printf("Screen BPP: %d\n", SDL_GetVideoSurface()->format->BitsPerPixel);
	printf("\n");
	printf( "Vendor     : %s\n", glGetString( GL_VENDOR ) );
	printf( "Renderer   : %s\n", glGetString( GL_RENDERER ) );
	printf( "Version    : %s\n", glGetString( GL_VERSION ) );
	//printf( "Extensions : %s\n", glGetString( GL_EXTENSIONS ) );
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

	if (ArnInitGlExtFunctions() < 0)
	{
		std::cerr << " *** OpenGL extensions needed to run this program are not available." << std::endl;
		std::cerr << "     Check whether you are in the remote control display or have a legacy graphics adapter." << std::endl;
		std::cerr << "     Aborting..." << std::endl;
		Cleanup();
		SDL_Quit();
		return -102;
	}
	ArnInitializePhysics();
	ArnInitializeGl();

	// Initialize renderer-dependent data in scene graph objects.
	ArnInitializeRenderableObjectsGl(curSgPtr.get());
	ArnConfigureViewportProjectionMatrixGl(&avd, activeCam); // Projection matrix is not changed during runtime for now.
	ArnConfigureViewMatrixGl(activeCam);

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

	unsigned int frames = 0;
	unsigned int start_time = SDL_GetTicks();

	/* Loop until done. */
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

		// Update phase

		// Physics simulation frequency (Hz)
		// higher --> accurate, stable, slow
		// lower  --> errors, unstable, fast
		static const unsigned int simFreq = 200;
		// Maximum simulation step iteration count for clamping
		// to keep app from advocating all resources to step further.
		static const unsigned int simMaxIteration = 500;

		unsigned int simLoop = unsigned int(frameDurationMs / 1000.0 * simFreq);
		if (simLoop > simMaxIteration)
			simLoop = simMaxIteration; 
		for (unsigned int step = 0; step < simLoop; ++step)
		{
			swPtr->updateFrame(1.0 / simFreq);
		}
		if (curSgPtr)
			curSgPtr->update(SDL_GetTicks() / 1000.0, frameDurationMs / 1000.0f);
		
		// Rendering phase
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

		foreach (ArnIkSolver* ikSolver, ikSolvers)
		{
			glPushMatrix();
			TreeDraw(*ikSolver->getTree());
			glPopMatrix();
		}

		glPushMatrix();
		{
			if (curSgPtr)
			{
				ArnSceneGraphRenderGl(curSgPtr.get());
			}
		}
		glPopMatrix();

		SDL_GL_SwapBuffers();

		/* Check for error conditions. */
		gl_error = glGetError( );
		if( gl_error != GL_NO_ERROR )
		{
			fprintf( stderr, "ARAN: OpenGL error: %d\n", gl_error );
		}
		sdl_error = SDL_GetError( );
		if( sdl_error[0] != '\0' )
		{
			fprintf(stderr, "ARAN: SDL error '%s'\n", sdl_error);
			SDL_ClearError();
		}

		while( SDL_PollEvent( &event ) )
		{
			done = HandleEvent(&event, curSgPtr, ikSolvers, &avd, swPtr);
			
			int reconfigScene = false;
			if (done == MHR_NEXT_SCENE)
			{
				int nextSceneIndex = (curSceneIndex + 1) % sceneList.size();
				curSgPtr = ConfigureNextTestSceneWithRetry(curSceneIndex, nextSceneIndex, sceneList, avd);
				if (!curSgPtr)
				{
					std::cerr << " *** Aborting..." << std::endl;
					done = MHR_EXIT_APP;
				}
				else
				{
					reconfigScene = true;
				}
			}
			else if (done == MHR_RELOAD_SCENE)
			{
				curSgPtr = ReloadCurrentScene(curSceneIndex, sceneList, avd);
				if (!curSgPtr)
				{
					std::cerr << " *** Aborting..." << std::endl;
					done = MHR_EXIT_APP;
				}
				else
				{
					reconfigScene = true;
				}
			}

			if (reconfigScene)
			{
				// Initialize renderer-independent data in scene graph objects.
				swPtr.reset(SimWorld::createFrom(curSgPtr.get()));
				GetActiveCamAndLight(activeCam, activeLight, curSgPtr.get());
				foreach (ArnIkSolver* ikSolver, ikSolvers) { delete ikSolver; }
				ikSolvers.clear();
				ArnCreateArnIkSolversOnSceneGraph(ikSolvers, curSgPtr);

				// Initialize renderer-dependent data in scene graph objects.
				ArnInitializeRenderableObjectsGl(curSgPtr.get());
				ArnConfigureViewportProjectionMatrixGl(&avd, activeCam); // Projection matrix is not changed during runtime for now.
				ArnConfigureViewMatrixGl(activeCam);
			}
		}
		++frames;
		frameEndMs = SDL_GetTicks();
	}

	/* Print out the frames per second */
	unsigned int this_time = SDL_GetTicks();
	if ( this_time != start_time )
	{
		printf("%2.2f FPS\n", ((float)frames/(this_time-start_time))*1000.0);
	}
	Cleanup();
	SDL_Quit();

	foreach (ArnIkSolver* ikSolver, ikSolvers)
	{
		delete ikSolver;
	}
	ikSolvers.clear();
	return 0;
}

int main(int argc, char *argv[])
{
	int retCode = 0;
	retCode = DoMain();

#ifdef ARNOBJECT_MEMORY_LEAK_CHECK
	// Simple check for the memory leak of ArnObjects.
	std::cout << "ArnObject ctor count: " << ArnObject::getCtorCount() << std::endl;
	std::cout << "ArnObject dtor count: " << ArnObject::getDtorCount() << std::endl;
	ArnObject::printInstances();
#endif // #ifdef ARNOBJECT_MEMORY_LEAK_CHECK
	return retCode;
}
