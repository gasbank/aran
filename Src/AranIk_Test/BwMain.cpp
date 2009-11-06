#include "BwPch.h"
#include "BwMain.h"
#include "BwTopWindow.h"
#include "BwOpenGlWindow.h"
#include "BwAppContext.h"
#include "BwWin32Timer.h"
#include "BwDrawingOptionsWindow.h"

#if !HAVE_GL
#error OpenGL in FLTK not enabled.
#endif

static inline double
FootHeight(double t, double stepLength, double maxStepHeight)
{
	if (t <= 0 || t >= stepLength)
		return 0;
	else
		return -4.0*maxStepHeight*t*(t-stepLength)/(stepLength*stepLength);
}


/*
static MessageHandleResult
HandleEvent(SDL_Event* event, AppContext& ac)
{
	static std::pair<int, int> mousePosition;
	MessageHandleResult done = MHR_DO_NOTHING;
	ArnSkeleton* skel = 0;
	if (ac.sgPtr)
	{
		skel = reinterpret_cast<ArnSkeleton*>(ac.sgPtr->findFirstNodeOfType(NDT_RT_SKELETON));
	}
	switch( event->type )
	{
	case SDL_VIDEORESIZE:
		{
			ac.windowWidth = event->resize.w;
			ac.windowHeight = event->resize.h;
			ac.avd.Width	= ac.windowWidth;
			ac.avd.Height	= ac.windowHeight;
			glViewport(0, 0, ac.windowWidth, ac.windowHeight);
			printf("Resized window size is %d x %d.\n", ac.windowWidth, ac.windowHeight);
			break;
		}
	case SDL_JOYAXISMOTION:
		{
			//std::cout << "Type " << (int)event->jaxis.type << " / Which " << (int)event->jaxis.which << " axis " << (int)event->jaxis.axis << " / value " << (int)event->jaxis.value << std::endl;
			if ((int)event->jaxis.axis == 0)
			{
				if (abs(event->jaxis.value) > 9000)
				{
					ac.linVelX = float(event->jaxis.value / 16000.0);
				}
				else
				{
					ac.linVelX = 0;
				}
			}
			if ((int)event->jaxis.axis == 1)
			{
				if (abs(event->jaxis.value) > 9000)
				{
					ac.linVelZ = float(-event->jaxis.value / 16000.0);

				}
				else
				{
					ac.linVelZ = 0;
				}
			}
			GeneralBodyPtr gbPtr = ac.swPtr->getBodyByNameFromSet("EndEffector");
			if (gbPtr)
			{
				gbPtr->setLinearVel(ac.linVelX, 0, ac.linVelZ);
			}
			/////////////////////////////////////////////////////////////////////////
			if ((int)event->jaxis.axis == 2)
			{
				if (abs(event->jaxis.value) > 9000)
				{
					ac.torque = event->jaxis.value / 10.0f;
				}
				else
				{
					ac.torque = 0;
				}
			}

			if ((int)event->jaxis.axis == 5)
			{
				if (abs(event->jaxis.value) > 10)
				{
					ac.torqueAnkle = event->jaxis.value / 500.0f;
				}
				else
				{
					ac.torqueAnkle = 0;
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
			std::cout << "gs_torque =  " << ac.torque << std::endl;
			std::cout << "gs_linVelX = " << ac.linVelX << std::endl;
			std::cout << "gs_linVelZ = " << ac.linVelZ << std::endl;
		}
		break;
	case SDL_MOUSEBUTTONUP:
		{
			if (event->button.button == SDL_BUTTON_LEFT)
			{
				SelectGraphicObject(
					ac,
					float(event->motion.x),
					float(ac.avd.Height - event->motion.y) // Note that Y-coord flipped.
					);

				if (ac.sgPtr)
				{
					ArnMatrix modelview, projection;
					glGetFloatv(GL_MODELVIEW_MATRIX, reinterpret_cast<GLfloat*>(modelview.m));
					modelview = modelview.transpose();
					glGetFloatv(GL_PROJECTION_MATRIX, reinterpret_cast<GLfloat*>(projection.m));
					projection = projection.transpose();
					ArnVec3 origin, direction;
					ArnMakePickRay(&origin, &direction, float(event->motion.x), float(ac.avd.Height - event->motion.y), &modelview, &projection, &ac.avd);
					ArnMesh* mesh = reinterpret_cast<ArnMesh*>(ac.sgPtr->findFirstNodeOfType(NDT_RT_MESH));
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
			else if (event->button.button == SDL_BUTTON_WHEELUP)
			{
				if (ac.viewMode == VM_TOP || ac.viewMode == VM_LEFT || ac.viewMode == VM_FRONT)
				{
					if (ac.orthoViewDistance > 1)
						--ac.orthoViewDistance;
				}
			}
			else if (event->button.button == SDL_BUTTON_WHEELDOWN)
			{
				if (ac.viewMode == VM_TOP || ac.viewMode == VM_LEFT || ac.viewMode == VM_FRONT)
				{
					++ac.orthoViewDistance;
				}
			}
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		{
			break;
		}
	case SDL_MOUSEMOTION:
		{
			mousePosition.first = event->motion.x;
			mousePosition.second = event->motion.y;

			printf("%d %d\n", event->motion.x, event->motion.y);

			if (ac.bPanningButtonDown)
			{
				int dx = (int)event->motion.x - ac.panningStartPoint.first;
				int dy = (int)event->motion.y - ac.panningStartPoint.second;
				const float aspectRatio = (float)ac.windowWidth / ac.windowHeight;
				ac.dPanningCenter.first = -(2.0f * ac.orthoViewDistance * aspectRatio / ac.windowWidth) * dx;
				ac.dPanningCenter.second = -(-2.0f * ac.orthoViewDistance / ac.windowHeight) * dy;

				//printf("%f   %f\n", ac.dPanningCenter.first, ac.dPanningCenter.second);
			}
			break;
		} 
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
		else if (event->key.keysym.sym == SDLK_KP7)
		{
			ac.viewMode = VM_TOP;
			printf("  View mode set to top.\n");
		}
		else if (event->key.keysym.sym == SDLK_KP1)
		{
			ac.viewMode = VM_LEFT;
			printf("  View mode set to left.\n");
		}
		else if (event->key.keysym.sym == SDLK_KP3)
		{
			ac.viewMode = VM_FRONT;
			printf("  View mode set to front.\n");
		}
		else if (event->key.keysym.sym == SDLK_KP4)
		{
			ac.viewMode = VM_CAMERA;
			printf("  View mode set to camera.\n");
		}
		else if (event->key.keysym.sym == SDLK_h)
		{
			ac.bRenderHud = !ac.bRenderHud;
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
		else if (event->key.keysym.sym == SDLK_SPACE)
		{	
			ac.bPanningButtonDown = true;
			ac.panningStartPoint = mousePosition;

			
			const float aspectRatio = (float)ac.windowWidth / ac.windowHeight;
			float worldX = (2.0f * ac.orthoViewDistance * aspectRatio / ac.windowWidth) * mousePosition.first + (ac.panningCenter.first - ac.orthoViewDistance * aspectRatio);
			float worldY = (-2.0f * ac.orthoViewDistance / ac.windowHeight) * mousePosition.second + (ac.panningCenter.second + ac.orthoViewDistance);
			printf("Panning start point is (%d, %d) or (%.3f, %.3f)\n", mousePosition.first, mousePosition.second, worldX, worldY);
			
		}
		//printf("key '%s' pressed\n", SDL_GetKeyName(event->key.keysym.sym));
		break;
	case SDL_KEYUP:
		ac.bHoldingKeys[event->key.keysym.sym] = false;

		if (event->key.keysym.sym == SDLK_c)
		{
			ac.bNextCamera = true;
		}
		else if (event->key.keysym.sym == SDLK_SPACE)
		{
			ac.bPanningButtonDown = false;

			ac.panningCenter.first += ac.dPanningCenter.first;
			ac.panningCenter.second += ac.dPanningCenter.second;

			ac.dPanningCenter.first = 0;
			ac.dPanningCenter.second = 0;
		}
		break;
	case SDL_QUIT:
		done = MHR_EXIT_APP;
		break;
	}
	return done;
}
*/

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

static void
UpdateScene(BwAppContext& ac, unsigned int frameStartMs, unsigned int frameDurationMs)
{
	// Physics simulation frequency (Hz)
	// higher --> accurate, stable, slow
	// lower  --> errors, unstable, fast
	static const unsigned int simFreq = 1000;
	// Maximum simulation step iteration count for clamping
	// to keep app from advocating all resources to step further.
	static const unsigned int simMaxIteration = 100;

	unsigned int simLoop = (unsigned int)(frameDurationMs / 1000.0 * simFreq);
	if (simLoop > simMaxIteration)
		simLoop = simMaxIteration;
	for (unsigned int step = 0; step < simLoop; ++step)
	{
		//printf("frame duration: %d / simloop original: %d / current simstep %d\n", frameDurationMs, (unsigned int)(frameDurationMs / 1000.0 * simFreq), step);

		ac.swPtr->updateFrame(1.0 / simFreq);
	}
	if (ac.sgPtr)
	{
		ac.sgPtr->update(ac.timer.getTicks() / 1000.0, frameDurationMs / 1000.0f);
	}
	foreach (ArnIkSolver* ikSolver, ac.ikSolvers)
	{
		ikSolver->update();

		NodePtr selNode = ikSolver->getSelectedEndeffector();
		if (selNode)
		{
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

	if (ac.bNextCamera)
	{
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

	ac.activeCam->setLocalXform_Trans( ac.activeCam->getLocalXform_Trans() + cameraDiff );
	ac.activeCam->recalcLocalXform();

	ac.isects.clear();
	ac.verticalLineIsects.clear();

	
	if (ac.trunk)
	{
		// Update COM pos
		ArnVec3 bipedComPos;
		ac.trunk->calculateLumpedComAndMass(&bipedComPos, &ac.bipedMass);
		ac.bipedComPos.push_back(bipedComPos); // Save the trail of COM for visualization
		//ac.trunk->calculateLumpedGroundIntersection(ac.isects);
		
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

	//unsigned int contactCount = ac.swPtr->getContactCount();
	ac.supportPolygon.clear();

	/*
	unsigned int isectsCount = ac.isects.size();
	if (isectsCount)
	{
	std::sort(ac.isects.begin(), ac.isects.end(),
	ret<bool>( (&_1->*&ArnVec3::x) < (&_2->*&ArnVec3::x)) );
	}
	int a = 10;
	*/

	//unsigned int isectsCount = ac.isects.size();
	//if (isectsCount)
	//{
	//	std::vector<Point_2> isectsCgal;
	//	/*
	//	for (unsigned int i = 0; i < isectsCount; ++i)
	//	{
	//		ArnVec3 contactPos;
	//		ac.swPtr->getContactPosition(i, &contactPos);
	//		isectsCgal.push_back(Point_2(contactPos.x, contactPos.y));
	//	}
	//	*/
	//	foreach (const ArnVec3& contactPos, ac.isects)
	//	{
	//		isectsCgal.push_back(Point_2(contactPos.x, contactPos.y));
	//	}

	//	std::vector<Point_2> out;
	//	out.resize(isectsCount);
	//	std::vector<Point_2>::iterator outEnd;

	//	outEnd = convex_hull_2(isectsCgal.begin(), isectsCgal.end(), out.begin(), CGAL::Cartesian<double>());

	//	for (std::vector<Point_2>::const_iterator it = out.begin(); it != outEnd; ++it)
	//	{
	//		const Point_2& p = *it;
	//		ac.supportPolygon.push_back(ArnVec3(p[0], p[1], 0));
	//	}
	//}
}

static void
AddToSceneGraphList( const ArnNode* node, Fl_Select_Browser* list, int depth ) 
{
	std::string itemName("@s@f");
	for (int i = 0; i < depth; ++i)
		itemName += "  ";
	if (strlen(node->getName()))
		itemName += node->getName();
	else
		itemName += "<unnamed>";
	list->add(itemName.c_str());

	foreach (const ArnNode* child, node->getChildren())
	{
		AddToSceneGraphList(child, list, depth+1);
	}
}

static void
UpdateSceneGraphList( BwAppContext& ac ) 
{
	if (ac.sceneGraphList)
	{
		ac.sceneGraphList->clear();
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
	foreach (ArnIkSolver* ikSolver, ac.ikSolvers)
	{
		delete ikSolver;
	}
	ac.ikSolvers.clear();
	ac.bipedComPos.clear();
	if (ac.swPtr)
	{
		ac.trunk = ac.swPtr->getBodyByNameFromSet("Trunk");

		// Create ArnSkeleton from rigid body links!
		if (ac.trunk)
		{
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
	ArnCreateArnIkSolversOnSceneGraph(ac.ikSolvers, ac.sgPtr);
	UpdateSceneGraphList(ac);
	return 0;
}

/*!
* @brief Scene graph가 새로 로드되었을 때 수행되는 초기화 (렌더러 종속)
*/
static int
InitializeRendererDependentsFromSg(BwAppContext& ac)
{
	ArnInitializeRenderableObjectsGl(ac.sgPtr.get());
	return 0;
}

static int
InitializeRendererDependentOnce(BwAppContext& ac)
{
	// OpenGL context available from this line.
	if (!glGetString( GL_VENDOR ) || !glGetString( GL_RENDERER ) || !glGetString( GL_VERSION ))
	{
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
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	//glShadeModel(GL_FLAT);							// Enable Smooth Shading
	glClearDepth(1.0f);									// Depth Buffer Setup
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
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

	
	/// \c SceneList.txt 를 파싱합니다.
	if (LoadSceneList(ac.sceneList) < 0)
	{
		std::cerr << " *** Init failed..." << std::endl;
		return -10;
	}

	memset(ac.bHoldingKeys, 0, sizeof(ac.bHoldingKeys));

	/// Viewport를 초기화합니다.
	ac.avd.X		= 0;
	ac.avd.Y		= 0;
	ac.avd.MinZ		= 0;
	ac.avd.MaxZ		= 1.0f;

	ac.viewMode = VM_UNKNOWN;
	ac.orthoViewDistance = 5;
	// Panning by dragging
	ac.bPanningButtonDown = false;
	ac.panningCenter[0] = 0;
	ac.panningCenter[1] = 0;
	ac.panningCenter[2] = 0;
	ac.dPanningCenter[0] = 0;
	ac.dPanningCenter[1] = 0;
	ac.dPanningCenter[2] = 0;
	// Drawing options
	ac.bRenderGrid					= true;
	ac.bRenderHud					= false;
	ac.bRenderJointIndicator		= false;
	ac.bRenderEndeffectorIndicator	= false;
	ac.bJointAxisIndicator			= false;
	ac.bContactIndicator			= false;
	ac.bContactForaceIndicator		= false;
	// Scene graph UI
	ac.sceneGraphList = 0;
	// Timer init
	ac.timer.start();
	
	ac.bSimulate = false;

	ac.contactCheckPlane.setV0(ArnVec3(0, 0, 0));
	ac.contactCheckPlane.setNormal(ArnVec3(0, 0, 1));

	/// 다음 카메라로 변경 플래그 초기화
	ac.bNextCamera	= false;

	/// 첫 장면 파일을 메모리에 로드합니다.
	assert(ac.sceneList.size() > 0);
	ac.curSceneIndex = -1;
	ac.sgPtr = ConfigureNextTestSceneWithRetry(ac.curSceneIndex, 0, ac.sceneList, ac.avd);
	if (!ac.sgPtr)
	{
		std::cerr << " *** Scene graph loading failed..." << std::endl;
		Cleanup();
		return -20;
	}
	assert(ac.sgPtr);


	/// 처음으로 로드한 모델 파일에 종속적인 데이터를 초기화합니다.
	if (InitializeRendererIndependentsFromSg(ac) < 0)
	{
		Cleanup();
		return -1;
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

void idle_cb(void* ac)
{
	static unsigned int start_time			= 0;
	static unsigned int frameStartMs		= 0;
	static unsigned int frameDurationMs		= 0;
	static unsigned int frameEndMs			= 0;
	static char frameStr[32];

	BwAppContext& appContext = *(BwAppContext*)ac;
	if (appContext.bSimulate)
	{
		start_time = (unsigned int)appContext.timer.getTicks();
		frameDurationMs = frameEndMs - frameStartMs;
		frameStartMs = (unsigned int)appContext.timer.getTicks();
		UpdateScene(appContext, frameStartMs, frameDurationMs);
		++appContext.frames;
		frameEndMs = (unsigned int)appContext.timer.getTicks();
		appContext.glWindow->redraw();
		sprintf(frameStr, "%d", appContext.frames);
		appContext.frameLabel->label(frameStr);
	}
}

void simulate_button_cb(Fl_Widget *o, void *p)
{
	Fl_Light_Button* widget = (Fl_Light_Button*)o;
	BwAppContext& appContext = *(BwAppContext*)p;
	appContext.bSimulate = widget->value() ? true : false;

	if (appContext.bSimulate)
	{
		if (!Fl::has_idle(idle_cb, &appContext))
			Fl::add_idle(idle_cb, &appContext);
	}
	else
	{
		if (Fl::has_idle(idle_cb, &appContext))
			Fl::remove_idle(idle_cb, &appContext);
	}
}

struct SceneButtonsHolder
{
	BwAppContext* ac;
	BwOpenGlWindow* openGlWindow;
	MessageHandleResult mhr;
};

void scene_buttons_cb(Fl_Widget* o, void* p)
{
	SceneButtonsHolder* sbh = (SceneButtonsHolder*)p;
	BwAppContext& ac = *sbh->ac;
	BwOpenGlWindow& openGlWindow = *sbh->openGlWindow;
	MessageHandleResult done = sbh->mhr;
	
	int reconfigScene = false;
	if (done == MHR_NEXT_SCENE || done == MHR_PREV_SCENE)
	{
		int nextSceneIndex;
		if (done == MHR_NEXT_SCENE)
			nextSceneIndex = (ac.curSceneIndex + 1) % ac.sceneList.size();
		else
		{
			if (ac.curSceneIndex == 0)
				nextSceneIndex = ac.sceneList.size() - 1;
			else
				nextSceneIndex = ac.curSceneIndex - 1;
		}

		ac.sgPtr = ConfigureNextTestSceneWithRetry(ac.curSceneIndex, nextSceneIndex, ac.sceneList, ac.avd);
		if (!ac.sgPtr)
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
		ac.sgPtr = ReloadCurrentScene(ac.curSceneIndex, ac.sceneList, ac.avd);
		if (!ac.sgPtr)
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
		InitializeRendererIndependentsFromSg(ac);
		InitializeRendererDependentsFromSg(ac);

		openGlWindow.redraw();
	}
}

int main(int argc, char **argv)
{
	int ret = 0;
	{
		BwAppContext appContext;
		InitializeRendererIndependentOnce(appContext);
		BwTopWindow topWindow(800, 600, "aran", appContext);

		std::ifstream windowPosAndSizeInput("BwWindow.txt");
		if (windowPosAndSizeInput.is_open())
		{
			int x, y, w, h;
			windowPosAndSizeInput >> x >> y >> w >> h;
			topWindow.resize(x, y, w, h);
		}

		BwOpenGlWindow openGlWindow(10, 75, topWindow.w()-20-200, topWindow.h()-90, 0, appContext);
		//openGlWindow.mode(FL_RGB | FL_DOUBLE | FL_DEPTH);
		topWindow.setShapeWindow(&openGlWindow);
		//sw.mode(FL_RGB);
		topWindow.resizable(&openGlWindow);

		SceneButtonsHolder sbh[3];
		sbh[0].ac = &appContext;
		sbh[0].openGlWindow = &openGlWindow;
		sbh[0].mhr = MHR_RELOAD_SCENE;
		
		sbh[1].ac = &appContext;
		sbh[1].openGlWindow = &openGlWindow;
		sbh[1].mhr = MHR_PREV_SCENE;

		sbh[2].ac = &appContext;
		sbh[2].openGlWindow = &openGlWindow;
		sbh[2].mhr = MHR_NEXT_SCENE;
		

		Fl_Button reloadSceneButton(10, 5, 70, 30, "Reload");
		reloadSceneButton.callback(scene_buttons_cb, &sbh[0]);
		Fl_Button nextSceneButton(10+75, 5, 50, 30, "Prev");
		nextSceneButton.callback(scene_buttons_cb, &sbh[1]);
		Fl_Button prevSceneButton(10+75+55, 5, 50, 30, "Next");
		prevSceneButton.callback(scene_buttons_cb, &sbh[2]);
		Fl_Light_Button simulateButton(10, 40, 100, 30, "@> Simulate");
		simulateButton.callback(simulate_button_cb, &appContext);
		Fl_Button frameLabel(10+100+5, 40, 50, 30, "Frame");
		frameLabel.box(FL_NO_BOX);
		appContext.frameLabel = &frameLabel;

		Fl_Hor_Slider slider(260, 5, topWindow.w()-270, 30, "Sides:");
		slider.align(FL_ALIGN_LEFT);
		slider.callback(sides_cb,&openGlWindow);
		//slider.value(sw.sides);
		slider.step(1);
		slider.bounds(3,40);

		Fl_Hor_Slider oslider(260, 40, topWindow.w()-270, 30, "Overlay:");
		oslider.align(FL_ALIGN_LEFT);
		oslider.callback(overlay_sides_cb,&openGlWindow);
		//oslider.value(sw.overlay_sides);
		oslider.step(1);
		oslider.bounds(3,40);

		BwDrawingOptionsWindow drawingOptions(topWindow.w()-200, 75, 190, 100, 0, appContext, openGlWindow);

		Fl_Select_Browser sceneList(topWindow.w()-200, 75+110, 190, 100);

		Fl_Select_Browser sceneGraphList(topWindow.w()-200, 75+110+110, 190, topWindow.h()-90-110-110);
		appContext.sceneGraphList = &sceneGraphList;
		appContext.glWindow = &openGlWindow;

		topWindow.setSceneList(&sceneList);
		topWindow.setDrawingOptionsWindow(&drawingOptions);
		topWindow.end();
		topWindow.show(argc,argv);
		//printf("Can do overlay = %d\n", sw.can_do_overlay());

		openGlWindow.show();
		openGlWindow.make_current();
		openGlWindow.redraw_overlay();

		InitializeRendererDependentOnce(appContext);
		UpdateSceneGraphList(appContext);

		// scene list available now
		foreach (const std::string& scene, appContext.sceneList)
		{
			sceneList.add(scene.c_str());
		}

		//Fl::add_idle(idle_cb, &appContext);
		
		ret = Fl::run();

		std::ofstream windowPosAndSize("BwWindow.txt");
		windowPosAndSize << topWindow.x() << " " << topWindow.y() << " " << topWindow.w() << " " << topWindow.h() << std::endl;
		windowPosAndSize.close();
	}
	
#ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
	// Simple check for the memory leak of ArnObjects.
	std::cout << "ArnObject ctor count: " << ArnObject::getCtorCount() << std::endl;
	std::cout << "ArnObject dtor count: " << ArnObject::getDtorCount() << std::endl;
	ArnObject::printInstances();
#endif // #ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
	
	return ret;
}
