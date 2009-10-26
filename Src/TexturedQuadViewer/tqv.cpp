#include "tqvPCH.h"
#include "tqv.h"

bool g_bDrawWireframe = true;

static MessageHandleResult HandleEvent(SDL_Event* event, AppContext& ac)
{
	MessageHandleResult done = MHR_DO_NOTHING;
	switch( event->type )
	{
	case SDL_VIDEORESIZE:
		break;
	case SDL_KEYDOWN:
		ac.bHoldingKeys[event->key.keysym.sym] = true;

		if ( event->key.keysym.sym == SDLK_ESCAPE )
		{
			done = MHR_EXIT_APP;
		}
		else if (event->key.keysym.sym == SDLK_n)
		{
			done = MHR_NEXT_SCENE;
		}
		else if (event->key.keysym.sym == SDLK_b)
		{
			done = MHR_PREV_SCENE;
		}
		else if (event->key.keysym.sym == SDLK_r)
		{
			done = MHR_RELOAD_SCENE;
		}
		else if (event->key.keysym.sym == SDLK_f)
		{
			g_bDrawWireframe = !g_bDrawWireframe;
		}
		break;
	case SDL_KEYUP:
		ac.bHoldingKeys[event->key.keysym.sym] = false;
		break;
	case SDL_QUIT:
		done = MHR_EXIT_APP;
		break;
	}
	return done;
}

static void RenderScene(const AppContext& ac)
{
	glViewport(0, 0, ac.windowWidth, ac.windowHeight);
	glClearColor( 1.0, 1.0, 1.0, 1.0 );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho(-1, 1, -1, 1, -1, 1);
	gluOrtho2D(0, ac.windowWidth, 0, ac.windowHeight);
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable( GL_TEXTURE_2D );

	glColor3f(1, 1, 1);
	typedef std::pair<double, double> TqvVertPos;
	double newWidth = ac.quadVertices.rbegin()->first;
	double newHeight = ac.quadVertices.rbegin()->second;
	for (int k = 0; k < (g_bDrawWireframe ? 2 : 1); ++k)
	{
		if (k == 0)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			ac.texture->getRenderableObject()->render(true);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		
		glBegin(GL_QUADS);
		for (int j = 0; j < ac.gridCountY-1; ++j)
		{
			for (int i = 0; i < ac.gridCountX-1; ++i)
			{
				const int v = j*ac.gridCountX + i;
				glTexCoord2d((double)i / (ac.gridCountX-1), (double)j / (ac.gridCountY-1));
				glVertex3d  (ac.quadVertices[v                ].first         , ac.quadVertices[v                ].second, 0);

				glTexCoord2d((double)(i+1) / (ac.gridCountX-1), (double)j / (ac.gridCountY-1));
				glVertex3d  (ac.quadVertices[v+1              ].first         , ac.quadVertices[v+1              ].second, 0);

				glTexCoord2d((double)(i+1) / (ac.gridCountX-1), (double)(j+1) / (ac.gridCountY-1));
				glVertex3d  (ac.quadVertices[v+1+ac.gridCountX].first         , ac.quadVertices[v+1+ac.gridCountX].second, 0);

				glTexCoord2d((double)i / (ac.gridCountX-1), (double)(j+1) / (ac.gridCountY-1));
				glVertex3d  (ac.quadVertices[v  +ac.gridCountX].first         , ac.quadVertices[v  +ac.gridCountX].second, 0);
			}
		}
		glEnd();
	}

	
}

static void Cleanup()
{
	ArnCleanupXmlParser();
	ArnCleanupImageLibrary();
	ArnCleanupPhysics();
	ArnCleanupGl();
}

static void ParseInputFile(AppContext& ac)
{
	std::ifstream fin(ac.inputFileName.c_str());
	assert(fin.is_open());
	std::string imgFileName;
	fin >> imgFileName;
	fin >> ac.gridCountX >> ac.gridCountY;
	ac.quadVertices.resize(ac.gridCountX * ac.gridCountY);
	int i = 0;
	while (!fin.eof())
	{
		double x, y;
		fin >> x >> y;
		if (!fin.eof())
		{
			ac.quadVertices[i] = std::make_pair(x, y);
			++i;
		}
	}
	assert(i == ac.gridCountX * ac.gridCountY);
	delete ac.texture;
	ac.texture = ArnTexture::createFrom(imgFileName.c_str());
	ac.texture->init();
	ac.texture->setName(ac.texture->getFileName());
	
	assert(ac.texture);
	
	fin.close();
}

struct BitmapStruct
{
	int width, height;
	bool rgbOrBgr;
	unsigned char* data;
};


static int InitializeAppContextOnce(AppContext& ac, int argc, char** argv)
{
	assert(argc == 2);
	ac.inputFileName = argv[1];
	ac.texture = 0;
	ParseInputFile(ac);

	ac.windowWidth	= (int)ac.quadVertices.rbegin()->first;
	ac.windowHeight = (int)ac.quadVertices.rbegin()->second;

	memset(ac.bHoldingKeys, 0, sizeof(ac.bHoldingKeys));
	

	/*!
	* SDL 라이브러리를 초기화합니다.
	* 이후에 예기치않은 오류가 발생했을 경우에는 SDL_Quit() 함수를 호출한 후 반환해야 합니다.
	*/
	const int		bpp					= 32;
	const int		depthSize			= 24;
	bool			bFullScreen			= false;
	bool			bNoFrame			= false;
	if( SDL_Init( SDL_INIT_VIDEO /*| SDL_INIT_JOYSTICK*/ ) < 0 )
	{
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return -30;
	}

	/* Set the flags we want to use for setting the video mode */
	int video_flags = SDL_OPENGL | SDL_RESIZABLE;
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

	if ( SDL_SetVideoMode( ac.windowWidth, ac.windowHeight, bpp, video_flags ) == NULL ) {
		fprintf(stderr, "Couldn't set GL mode: %s\n", SDL_GetError());
		SDL_Quit();
		return -40;
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

	/* Set the window manager title bar */
	SDL_WM_SetCaption( "aran", "aran" );

	/// OpenGL 플래그를 설정합니다.
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LINE_SMOOTH);
	//glEnable(GL_LIGHTING);
	glDisable(GL_LIGHTING);
	//glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);
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
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
		SDL_Quit();
		return -50;
	}

	/// ARAN OpenGL 패키지를 초기화합니다.
	if (ArnInitializeGl() < 0)
	{
		SDL_Quit();
		return -3;
	}

	ac.texture->attachChild( ArnTextureGl::createFrom(ac.texture) );
	return 0;
}

static int DoMain(int argc, char** argv)
{
	/*!
	* 렌더러 독립적 ARAN 패키지인 ARAN Core, ARAN Physics를 초기화합니다.
	* 초기화가 성공한 이후 프로그램의 치명적인 오류로 인해 실행이 중단될 경우
	* 반드시 Cleanup() 을 호출해야 합니다.
	* 본 초기화가 실패할 경우에는 프로그램이 종료됩니다.
	*/
	if (ArnInitializeXmlParser() < 0)
	{
		Cleanup();
		return -1;
	}
	if (ArnInitializeImageLibrary() < 0)
	{
		Cleanup();
		return -2;
	}
	if (ArnInitializePhysics() < 0)
	{
		Cleanup();
		return -3;
	}
	std::cout << " INFO  Raw pointer    size = " << sizeof(ArnSceneGraph*) << std::endl;
	std::cout << " INFO  Shared pointer size = " << sizeof(ArnSceneGraphPtr) << std::endl;

	/*!
	* Application-wide context를 초기화합니다.
	* 이 초기화는 프로그램 구동시 단 한번 시행됩니다.
	*/
	AppContext ac;
	if (InitializeAppContextOnce(ac, argc, argv) < 0)
	{
		Cleanup();
		return -4;
	}

	/// 프로그램 메인 루프를 시작합니다.
	unsigned int frames = 0;
	unsigned int start_time;
	unsigned int frameStartMs = 0;
	unsigned int frameDurationMs = 0;
	unsigned int frameEndMs = 0;
	bool bExitLoop = false;
	start_time = SDL_GetTicks();
	MessageHandleResult done;
	while( !bExitLoop )
	{
		frameDurationMs = frameEndMs - frameStartMs;
		frameStartMs = SDL_GetTicks();
		GLenum gl_error;
		SDL_Event event;
		char* sdl_error;

		RenderScene(ac);

		SDL_GL_SwapBuffers();

		/* Check for error conditions. */
		gl_error = glGetError();
		if( gl_error != GL_NO_ERROR )
		{
			fprintf( stderr, "ARAN: OpenGL error: %s\n", gluErrorString(gl_error) );
		}
		sdl_error = SDL_GetError();
		if( sdl_error[0] != '\0' )
		{
			fprintf(stderr, "ARAN: SDL error '%s'\n", sdl_error);
			SDL_ClearError();
		}

		while( SDL_PollEvent( &event ) )
		{
			done = HandleEvent(&event, ac);

			if (done == MHR_EXIT_APP)
				bExitLoop = true;
			else if (done == MHR_RELOAD_SCENE)
			{
				SDL_SysWMinfo info;
				SDL_VERSION(&info.version); // this is important!
				SDL_GetWMInfo(&info);

				RECT winRect, clientRect;
				GetWindowRect(info.window, &winRect);
				GetClientRect(info.window, &clientRect);
				assert(clientRect.left == 0);
				assert(clientRect.top == 0);
				int dx = (winRect.right - winRect.left) - clientRect.right;
				int dy = (winRect.bottom - winRect.top) - clientRect.bottom;

				ParseInputFile(ac);
				ac.texture->attachChild( ArnTextureGl::createFrom(ac.texture) );

				ac.windowWidth	= (int)ac.quadVertices.rbegin()->first;
				ac.windowHeight = (int)ac.quadVertices.rbegin()->second;
				
				SetWindowPos(info.window, 0, 0, 0, ac.windowWidth + dx, ac.windowHeight + dy, SWP_NOMOVE | SWP_NOZORDER);
				
			}
		}
		++frames;
		frameEndMs = SDL_GetTicks();
	}

	/* Print out the frames per second */
	unsigned int this_time = SDL_GetTicks();
	if ( this_time != start_time )
	{
		printf("%.2f FPS\n", ((float)frames/(this_time-start_time))*1000.0);
	}

	delete ac.texture;

	Cleanup();
	SDL_Quit();
	return 0;
}

int main(int argc, char** argv)
{
	//scm_boot_guile(argc, argv, guile_inner_main, 0);
	int retCode = 0;
	retCode = DoMain(argc, argv);
#ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
	// Simple check for the memory leak of ArnObjects.
	std::cout << "ArnObject ctor count: " << ArnObject::getCtorCount() << std::endl;
	std::cout << "ArnObject dtor count: " << ArnObject::getDtorCount() << std::endl;
	ArnObject::printInstances();
#endif // #ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
	return retCode;
}
