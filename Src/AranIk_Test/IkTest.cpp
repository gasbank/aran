/*!
 * @file IkTest.cpp
 * @author Geoyeob Kim
 * @date 2009
 */
#include "IkTest.h"

class AppContext : private Uncopyable
{
public:
											AppContext();
											~AppContext();
	static const int						windowWidth			= 800;
	static const int						windowHeight		= 600;
	ArnViewportData							avd;
	std::vector<std::string>				sceneList;
	int										curSceneIndex;
	ArnSceneGraphPtr						sgPtr;
	SimWorldPtr								swPtr;
	ArnCamera*								activeCam;
	ArnLight*								activeLight;
	std::vector<ArnIkSolver*>				ikSolvers;
	GeneralBodyPtr							trunk;
	ArnPlane								contactCheckPlane;
	boost::circular_buffer<ArnVec3>			bipedComPos;			///< Store a trail of biped COM positions
	float									bipedMass;
	float									linVelX;
	float									linVelZ;
	float									torque;
	float									torqueAnkle;
	char									bHoldingKeys[SDLK_LAST];
	bool									bNextCamera;

	// Volatile: should be clear()-ed every frame.
	std::list<ArnVec3>						isects;					///< Foot-ground intersection points
	std::vector<ArnVec3>					supportPolygon;			///< Support polygon
};

AppContext::AppContext()
: bipedComPos(50)
{
}

AppContext::~AppContext()
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

static void
SelectGraphicObject(AppContext& ac, const float mousePx, const float mousePy)
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
	glLoadIdentity();

	/* restrict the draw to an area around the cursor */
	const GLdouble pickingAroundFactor = 1.0;
	gluPickMatrix(mousePx, mousePy, pickingAroundFactor, pickingAroundFactor, view);

	/* your original projection matrix */
	ArnMatrix projMat;
	ArnGetProjectionMatrix(&projMat, &ac.avd, ac.activeCam, true);
	glMultTransposeMatrixf(reinterpret_cast<const GLfloat*>(projMat.m));


	/* Draw the objects onto the screen */
	glMatrixMode(GL_MODELVIEW);
		/* draw only the names in the stack, and fill the array */
		glFlush();
		SDL_GL_SwapBuffers();
		ArnSceneGraphRenderGl(ac.sgPtr.get(), true);
		foreach (ArnIkSolver* ikSolver, ac.ikSolvers)
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
				NodePtr node = ikSolver->getNodeByObjectId(buff[h].contents);
				if (node)
				{
					printf("[Object 0x%p ID %d %s] Endeffector=%d\n",
						node.get(), node->getObjectId(), node->getName(), node->isEndeffector());
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
				}
			}
		}
	}
}

static MessageHandleResult
HandleEvent(SDL_Event* event, AppContext& ac)
{
	MessageHandleResult done = MHR_DO_NOTHING;
	ArnSkeleton* skel = 0;
	if (ac.sgPtr)
	{
		skel = reinterpret_cast<ArnSkeleton*>(ac.sgPtr->findFirstNodeOfType(NDT_RT_SKELETON));
	}
	switch( event->type ) {
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
			}
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
			else if (event->key.keysym.sym == SDLK_r)
			{
				done = MHR_RELOAD_SCENE;
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

			printf("key '%s' pressed\n", SDL_GetKeyName(event->key.keysym.sym));
			break;
		case SDL_KEYUP:
			ac.bHoldingKeys[event->key.keysym.sym] = false;

			if (event->key.keysym.sym == SDLK_c)
			{
				ac.bNextCamera = true;
			}
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
Walk(ArnIkSolver* ikSolver)
{

}

static void
UpdateScene(AppContext& ac, unsigned int frameStartMs, unsigned int frameDurationMs)
{
	// Physics simulation frequency (Hz)
	// higher --> accurate, stable, slow
	// lower  --> errors, unstable, fast
	static const unsigned int simFreq = 150;
	// Maximum simulation step iteration count for clamping
	// to keep app from advocating all resources to step further.
	static const unsigned int simMaxIteration = 100;

	unsigned int simLoop = (unsigned int)(frameDurationMs / 1000.0 * simFreq);
	if (simLoop > simMaxIteration)
		simLoop = simMaxIteration;
	for (unsigned int step = 0; step < simLoop; ++step)
	{
		ac.swPtr->updateFrame(1.0 / simFreq);
	}
	if (ac.sgPtr)
	{
		ac.sgPtr->update(SDL_GetTicks() / 1000.0, frameDurationMs / 1000.0f);
	}
	foreach (ArnIkSolver* ikSolver, ac.ikSolvers)
	{
		ikSolver->update();

		Walk(ikSolver);

		NodePtr selNode = ikSolver->getSelectedEndeffector();
		if (selNode)
		{
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
		}
	}

	if (ac.bNextCamera)
	{
		ac.activeCam = ac.sgPtr->getNextCamera(ac.activeCam);
		ac.bNextCamera = false;
	}

	ArnVec3 cameraDiff(0, 0, 0);
	static const float cameraDiffAmount = 0.1f;
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

	ac.activeCam->setLocalXform_Trans( ac.activeCam->getLocalXform_Trans() + cameraDiff );
	ac.activeCam->recalcLocalXform();

	ac.isects.clear();
	if (ac.trunk)
	{
		ArnVec3 bipedComPos;
		ac.trunk->calculateLumpedComAndMass(&bipedComPos, &ac.bipedMass);
		ac.bipedComPos.push_back(bipedComPos);
		//ac.trunk->calculateLumpedGroundIntersection(ac.isects);
		ac.trunk->calculateLumpedIntersection(ac.isects, ac.contactCheckPlane);
	}

	//unsigned int contactCount = ac.swPtr->getContactCount();
	ac.supportPolygon.clear();

	unsigned int isectsCount = ac.isects.size();
	if (isectsCount)
	{
		std::vector<Point_2> isectsCgal;
		/*
		for (unsigned int i = 0; i < isectsCount; ++i)
		{
			ArnVec3 contactPos;
			ac.swPtr->getContactPosition(i, &contactPos);
			isectsCgal.push_back(Point_2(contactPos.x, contactPos.y));
		}
		*/
		foreach (const ArnVec3& contactPos, ac.isects)
		{
			isectsCgal.push_back(Point_2(contactPos.x, contactPos.y));
		}

		std::vector<Point_2> out;
		out.resize(isectsCount);
		std::vector<Point_2>::iterator outEnd;

		outEnd = convex_hull_2(isectsCgal.begin(), isectsCgal.end(), out.begin(), CGAL::Cartesian<double>());

		for (std::vector<Point_2>::const_iterator it = out.begin(); it != outEnd; ++it)
		{
			const Point_2& p = *it;
			ac.supportPolygon.push_back(ArnVec3(p[0], p[1], 0));
		}
	}
}

static void
RenderScene(const AppContext& ac)
{
	glClearColor( 0.5, 0.5, 0.5, 1.0 );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (ac.activeCam)
	{
		ArnConfigureViewportProjectionMatrixGl(&ac.avd, ac.activeCam);
		ArnConfigureViewMatrixGl(ac.activeCam);
	}
	if (ac.activeLight)
	{
		ArnConfigureLightGl(0, ac.activeLight);
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, 0);

	foreach (ArnIkSolver* ikSolver, ac.ikSolvers)
	{
		glPushMatrix();
		TreeDraw(*ikSolver->getTree());
		glPopMatrix();
	}

	glPushMatrix();
	{
		if (ac.sgPtr)
		{
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
	glPushAttrib(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	{
		if (ac.trunk)
		{
			glPushMatrix();
			const ArnVec3& bipedComPos = *ac.bipedComPos.rbegin();
			glTranslatef(bipedComPos.x, bipedComPos.y, bipedComPos.z);
			ArnSetupBasicMaterialGl(&ArnConsts::ARNCOLOR_GREEN);
			ArnRenderSphereGl(0.025, 16, 16);

			ArnVec3 netContactForce;
			const unsigned int contactCount = ac.swPtr->getContactCount();
			for (unsigned int i = 0; i < contactCount; ++i)
			{
				ArnVec3 contactForce;
				ac.swPtr->getContactForce1(i, &contactForce);
				netContactForce += contactForce;
			}
			glEnable(GL_COLOR_MATERIAL);
			glBegin(GL_LINES);
			glColor3f(0, 0, 1); glVertex3f(0, 0, 0);
			glColor3f(0, 0, 1); glVertex3f(netContactForce.x, netContactForce.y, netContactForce.z);
			glEnd();
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

		const unsigned int contactCount = ac.swPtr->getContactCount();
		for (unsigned int i = 0; i < contactCount; ++i)
		{
			ArnVec3 contactPos, contactForce;
			ac.swPtr->getContactPosition(i, &contactPos);
			glPushMatrix();
			{
				glTranslatef(contactPos.x, contactPos.y, contactPos.z);
				ArnSetupBasicMaterialGl(&ArnConsts::ARNCOLOR_YELLOW);
				ArnRenderSphereGl(0.025, 16, 16);

				/*

				ac.swPtr->getContactForce1(i, &contactForce);
				//contactForce /= 100;
				glEnable(GL_COLOR_MATERIAL);
				glBegin(GL_LINES);
				glColor3f(1, 0, 0); glVertex3f(0, 0, 0);
				glColor3f(1, 0, 0); glVertex3f(contactForce.x, contactForce.y, contactForce.z);
				glEnd();
				glDisable(GL_COLOR_MATERIAL);

				ac.swPtr->getContactForce2(i, &contactForce);
				glEnable(GL_COLOR_MATERIAL);
				glBegin(GL_LINES);
				glColor3f(0, 0, 1); glVertex3f(0, 0, 0);
				glColor3f(0, 0, 1); glVertex3f(contactForce.x, contactForce.y, contactForce.z);
				glEnd();
				glDisable(GL_COLOR_MATERIAL);
				*/
			}
			glPopMatrix();
		}
	}
	glPopAttrib();
}

static void
RenderHud(const AppContext& ac)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	const double aspect = (double)AppContext::windowWidth / AppContext::windowHeight;
	glOrtho(-0.5 * aspect, 0.5 * aspect, -0.5, 0.5, 0, 1);
	glMatrixMode(GL_MODELVIEW);

	glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT | GL_LINE_BIT | GL_POINT_BIT);
	glLineWidth(1);
	glPointSize(4);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glPushMatrix();
		glLoadIdentity();
		glTranslatef(0.5, 0, 0);
		glScalef(0.25, 0.25, 1);

		ArnDrawAxesGl(0.5f);
		if (ac.supportPolygon.size())
		{
			glBegin(GL_LINE_LOOP);
			foreach (const ArnVec3& v, ac.supportPolygon)
			{
				glColor3f(0, 0, 0); glVertex2f(v.x, v.y);
			}
			glEnd();
		}
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

/*!
 * @brief Scene graph가 새로 로드되었을 때 수행되는 초기화 (렌더러와 무관)
 */
static int
InitializeRendererIndependentsFromSg(AppContext& ac)
{
	assert(ac.sgPtr);
	ac.swPtr.reset(SimWorld::createFrom(ac.sgPtr.get()));
	GetActiveCamAndLight(ac.activeCam, ac.activeLight, ac.sgPtr.get());
	foreach (ArnIkSolver* ikSolver, ac.ikSolvers)
	{
		delete ikSolver;
	}
	ac.ikSolvers.clear();
	//ArnCreateArnIkSolversOnSceneGraph(ac.ikSolvers, ac.sgPtr);
	ac.bipedComPos.clear();
	if (ac.swPtr)
	{
		ac.trunk = ac.swPtr->getBodyByNameFromSet("Trunk");

		if (ac.trunk)
		{
			ac.bipedComPos.clear();
			ArnVec3 comPos;
			ac.trunk->calculateLumpedComAndMass(&comPos, &ac.bipedMass);
			ac.bipedComPos.push_back(comPos);
			std::cout << " - Biped total mass: " << ac.bipedMass << std::endl;

			ArnSkeleton* trunkSkel = ac.trunk->createLumpedArnSkeleton();
			ac.sgPtr->attachChildToFront(trunkSkel);
		}
	}
	return 0;
}

/*!
 * @brief Scene graph가 새로 로드되었을 때 수행되는 초기화 (렌더러 종속)
 */
static int
InitializeRendererDependentsFromSg(AppContext& ac)
{
	ArnInitializeRenderableObjectsGl(ac.sgPtr.get());
	return 0;
}

/*!
 * @brief 프로그램 실행 시 한 번만 수행되는 초기화 루틴
 */
static int
InitializeAppContextOnce(AppContext& ac)
{
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
	ac.avd.Width	= AppContext::windowWidth;
	ac.avd.Height	= AppContext::windowHeight;
	ac.avd.MinZ		= 0;
	ac.avd.MaxZ		= 1.0f;

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
		return -20;
	}
	assert(ac.sgPtr);

	/*!
	 * SDL 라이브러리를 초기화합니다.
	 * 이후에 예기치않은 오류가 발생했을 경우에는 SDL_Quit() 함수를 호출한 후 반환해야 합니다.
	 */
	const int		bpp					= 32;
	const int		depthSize			= 24;
	bool			bFullScreen			= false;
	bool			bNoFrame			= false;
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) < 0 )
	{
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return -30;
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

	if ( SDL_SetVideoMode( AppContext::windowWidth, AppContext::windowHeight, bpp, video_flags ) == NULL ) {
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

	/// 처음으로 로드한 모델 파일에 종속적인 데이터를 초기화합니다.
	if (InitializeRendererIndependentsFromSg(ac) < 0)
	{
		SDL_Quit();
		return -1;
	}
	if (InitializeRendererDependentsFromSg(ac) < 0)
	{
		SDL_Quit();
		return -2;
	}
	return 0;
}

/*!
 * @brief 주요 루틴 시작 함수
 */
int
DoMain()
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
	if (InitializeAppContextOnce(ac) < 0)
	{
		Cleanup();
		return -4;
	}

	/// 프로그램 메인 루프를 시작합니다.
	static unsigned int frames = 0;
	static unsigned int start_time;
	static unsigned int frameStartMs = 0;
	static unsigned int frameDurationMs = 0;
	static unsigned int frameEndMs = 0;
	static bool bExitLoop = false;
	start_time = SDL_GetTicks();
	while( !bExitLoop )
	{
		frameDurationMs = frameEndMs - frameStartMs;
		frameStartMs = SDL_GetTicks();
		GLenum gl_error;
		SDL_Event event;
		char* sdl_error;

		// 1. Update phase
		UpdateScene(ac, frameStartMs, frameDurationMs);

		// 2. Rendering phase
		RenderScene(ac);
		RenderHud(ac);

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
			static MessageHandleResult done;
			done = HandleEvent(&event, ac);

			int reconfigScene = false;
			if (done == MHR_NEXT_SCENE)
			{
				int nextSceneIndex = (ac.curSceneIndex + 1) % ac.sceneList.size();
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
			}

			if (done == MHR_EXIT_APP)
				bExitLoop = true;
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

	foreach (ArnIkSolver* ikSolver, ac.ikSolvers)
	{
		delete ikSolver;
	}
	ac.ikSolvers.clear();

	Cleanup();
	SDL_Quit();
	return 0;
}

int main(int argc, char *argv[])
{
	int retCode = 0;
	retCode = DoMain();

#ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
	// Simple check for the memory leak of ArnObjects.
	std::cout << "ArnObject ctor count: " << ArnObject::getCtorCount() << std::endl;
	std::cout << "ArnObject dtor count: " << ArnObject::getDtorCount() << std::endl;
	ArnObject::printInstances();
#endif // #ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
	return retCode;
}
