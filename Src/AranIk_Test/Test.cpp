#include "AranApi.h"
#include "ArnSceneGraph.h"
#include "ArnMesh.h"
#include "ArnCamera.h"
#include "ArnVertexBuffer.h"
#include "ArnIndexBuffer.h"
#include "ArnXmlString.h"
#include "ArnXmlLoader.h"
#include "ArnLight.h"
#include "VideoManGl.h"
#include "ArnSkeleton.h"
#include "ArnAnimationController.h"

#ifndef WIN32
#include "inotify-cxx.h"
#endif

VideoMan*					g_videoMan = 0;
ArnSceneGraph*				g_sg = 0;
GLuint						g_lists = 0;
void*						g_font = GLUT_BITMAP_8_BY_13;
int							g_newCount = 0;
int							g_deleteCount = 0;
ArnCamera*					g_mainCamera = 0;
int							g_mouseClickX = 0;
int							g_mouseClickY = 0;
ArnVec3						g_unprojected(0, 0, 0);
#ifndef WIN32
Inotify						g_ino;
#endif
double						g_lastTime = 0;
double						g_lastTimeDiff = 0;

#ifdef WIN32
const char*					g_testFile = "E:/Documents and Settings/Administrator/ActionChangeTest.xml";
#else
const char*					g_testFile = "/home/gbu/BipedTest.xml";
#endif
///////////////////////////////////////////////////////////////////////////////
// write 2d text using GLUT
// The projection matrix must be set to orthogonal before call this function.
///////////////////////////////////////////////////////////////////////////////
void DrawString(const char *str, int x, int y, float color[4], void *font)
{
	glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
	glDisable(GL_LIGHTING);     // need to disable lighting for proper text color

	glColor4fv(color);          // set text color
	glRasterPos2i(x, y);        // place text position

	// loop all characters in the string
	while(*str)
	{
		glutBitmapCharacter(font, *str);
		++str;
	}

	glEnable(GL_LIGHTING);
	glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////////
// draw a string in 3D space
///////////////////////////////////////////////////////////////////////////////
void DrawString3D(const char *str, float pos[3], float color[4], void *font)
{
	glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
	glDisable(GL_LIGHTING);     // need to disable lighting for proper text color

	glColor4fv(color);          // set text color
	glRasterPos3fv(pos);        // place text position

	// loop all characters in the string
	while(*str)
	{
		glutBitmapCharacter(font, *str);
		++str;
	}

	glEnable(GL_LIGHTING);
	glPopAttrib();
}

void showInfo()
{
	// backup current model-view matrix
	glPushMatrix();                     // save current modelview matrix
	glLoadIdentity();                   // reset modelview matrix

	// set to 2D orthogonal projection
	glMatrixMode(GL_PROJECTION);     // switch to projection matrix
	glPushMatrix();                  // save current projection matrix
	glLoadIdentity();                // reset projection matrix
	gluOrtho2D(0, g_videoMan->GetWindowSizeW(), 0, g_videoMan->GetWindowSizeH());  // set to orthogonal projection

	float color[4] = { 1, 1, 0, 1 };
	std::stringstream ss;
	ss.str("");
	ss << std::fixed << std::setprecision(1);
	ss << g_videoMan->getFPS() << " FPS" << std::ends; // update fps string
	ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
	DrawString(ss.str().c_str(), 1, 1, color, g_font);

	ss.str("");
	ss << std::fixed << std::setprecision(3);
	ss << "Running time: " << g_videoMan->getTime() << " s" << std::ends; // update fps string
	ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
	DrawString(ss.str().c_str(), 1, 14, color, g_font);

	ss.str("");
	ss << "Click: (" << g_mouseClickX << ", " << g_mouseClickY << ") / Unprojected: (" << g_unprojected.x << ", " << g_unprojected.y << ", " << g_unprojected.z << ")" << std::ends;
	DrawString(ss.str().c_str(), 1, 28, color, g_font);

	ss.str("");
	ss << std::fixed << std::setprecision(6);
	ss << "UpdateFrame interval: " << g_lastTimeDiff << " sec" << std::ends;
	ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
	DrawString(ss.str().c_str(), 1, 42, color, g_font);

	// restore projection matrix
	glPopMatrix();                   // restore to previous projection matrix

	// restore modelview matrix
	glMatrixMode(GL_MODELVIEW);      // switch to modelview matrix
	glPopMatrix();                   // restore to previous modelview matrix
}

void RenderFrameCb()
{
	g_videoMan->clearFrameBuffer();
	g_videoMan->setupProjectionMatrix();
	g_videoMan->setupViewMatrix();
	g_videoMan->setupLightsFromSceneGraph();

	//VisualizeAxes(5);
	g_videoMan->renderSceneGraph();

	showInfo();
	g_videoMan->swapFrameBuffer();
}

void ReloadTestFile()
{
	// Create and init the scene graph instance from XML file
	// and attach that one to the video manager.
	assert(!g_videoMan->isRendering());
	g_videoMan->detachSceneGraph();
	delete g_sg;
	g_sg = ArnSceneGraph::createFrom(g_testFile);
	assert(g_sg);
	g_sg->interconnect(g_sg);
	g_sg->initRendererObjects();
	g_videoMan->attachSceneGraph(g_sg);
	ArnCamera* cam = reinterpret_cast<ArnCamera*>(g_sg->findFirstNodeOfType(NDT_RT_CAMERA));
	g_videoMan->SetCamera(*cam);
	std::cout << "Scene graph file reloaded." << std::endl;
}

void UpdateFrameCb(double dTime, float fElapsedTime)
{
	g_lastTimeDiff = g_videoMan->getTime() - g_lastTime;
	//printf("UpdateFrame interval: %.6f\n", (float)g_lastTimeDiff);
	g_lastTime = g_videoMan->getTime();
	g_videoMan->updateSceneGraph(dTime, fElapsedTime);
	if (g_lastTimeDiff > 0.001)
		fprintf(stderr, "UpdateFrame too slow. ");

	//Sleep(5);
	//printf("UpdateFrame called.\n");
#ifndef WIN32
	g_ino.WaitForEvents();
	if (g_ino.GetEventCount())
	{
		InotifyEvent inoEvent;
		while (g_ino.GetEventCount())
			g_ino.GetEvent(&inoEvent);

		g_ino.FindWatch(g_testFile)->SetEnabled(false);

		ReloadTestFile();

		g_ino.FindWatch(g_testFile)->SetEnabled(true);
	}
#endif
}

void ReshapeCb(int w, int h)
{
	g_videoMan->SetWindowSize(w, h);
	g_videoMan->setupProjectionMatrix();
}

void KeyboardCb(unsigned char key, int x, int y)
{
	ArnSkeleton* skel = static_cast<ArnSkeleton*>(g_sg->getNodeByName("Armature.004"));
	switch(key)
	{
	case 27: // ESCAPE
		exit(0);
		break;
	case ' ':
		break;
	case 'r':
		ReloadTestFile();
		break;
	case 'd':
		break;
	case 'c':
		{
			char eyeStr[64], atStr[64], upStr[64];
			const ARN_CAMERA& c = g_videoMan->getCamera();
			c.eye.getFormatString(eyeStr, 64);
			c.at.getFormatString(atStr, 64);
			c.up.getFormatString(upStr, 64);
			printf("Camera Info:\n");
			printf("   Eye  = %s\n", eyeStr);
			printf("   At   = %s\n", atStr);
			printf("   Up   = %s\n", upStr);
			printf("   Near = %.3f\n", c.nearClip);
			printf("   Far  = %.3f\n", c.farClip);
			printf("   Fov  = %.3f (%.3f deg)\n", c.angle, ArnToDegree(c.angle));
		}
		break;
	case 'n':
		skel->setActionToNext();
		break;
	case '1':
		{
			skel->getAnimCtrl()->SetTrackAnimationSet(0, 0);
			skel->getAnimCtrl()->SetTrackPosition(0, skel->getAnimCtrl()->GetTime());
			ARNTRACK_DESC desc;
			skel->getAnimCtrl()->GetTrackDesc(0, &desc);
			skel->getAnimCtrl()->SetTrackEnable(0, desc.Enable ? false : true);
			skel->getAnimCtrl()->SetTrackWeight(0, 1);
			printf("1 is pressed.\n");
		}
		break;
	case '2':
		{
			skel->getAnimCtrl()->SetTrackAnimationSet(1, 1);
			skel->getAnimCtrl()->SetTrackPosition(1, skel->getAnimCtrl()->GetTime());
			ARNTRACK_DESC desc;
			skel->getAnimCtrl()->GetTrackDesc(1, &desc);
			skel->getAnimCtrl()->SetTrackEnable(1, desc.Enable ? false : true);
			skel->getAnimCtrl()->SetTrackWeight(1, 1);
		}
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void MouseCb(int button, int state, int x, int y)
{
	g_mouseClickX = x;
	g_mouseClickY = y;

	ArnVec3 point(x, y, 0);
	ArnMatrix modelview, projection;
	glGetFloatv(GL_MODELVIEW_MATRIX, reinterpret_cast<GLfloat*>(&modelview));
	glGetFloatv(GL_PROJECTION_MATRIX, reinterpret_cast<GLfloat*>(&projection));
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	ArnViewportData avd;
	avd.X		= viewport[0];
	avd.Y		= viewport[1];
	avd.Width	= viewport[2];
	avd.Height	= viewport[3];
	avd.MinZ	= 0;
	avd.MaxZ	= 1;
	
	ArnVec3Unproject(&g_unprojected, &point, &avd, &projection, &modelview);
}

void AppExitHandler()
{
	DeallocateXmlParser();
	glDeleteLists(g_lists, 1);
	delete g_mainCamera;
	delete g_sg;
	delete g_videoMan;
	printf("New Count    = %d\n", g_newCount);
	printf("Delete Count = %d\n", g_deleteCount);
	if (g_newCount != g_deleteCount)
	{
		fprintf(stderr, "WARN: It is highly likely that the app has memory leak on heap space.\n");
	}
}

void OutOfMem()
{
	std::cerr << "Unable to satisfy request for memory.\n";
	std::abort();
}

#ifdef WIN32
void* operator new(size_t size)
#else
void* operator new(size_t size) throw (std::bad_alloc)
#endif
{
	void* p = malloc(size);
	if (!p)
		throw std::bad_alloc();
	++g_newCount;
	return p;
}

void operator delete(void* p) throw ()
{
	free(p);
	if (p)
		++g_deleteCount;
}

void VbIbTest()
{
	ArnSceneGraph* sceneGraph = ArnSceneGraph::createFromEmptySceneGraph();
	float verts[] = {  0, 2, 0 ,   -2, -2, 0,   2, -2, 0 };
	short ind[] = { 0, 1, 2 };
	ArnVertexBuffer* vb = ArnVertexBuffer::createFromArray(3, 3*sizeof(float), verts);
	ArnIndexBuffer* ib = ArnIndexBuffer::createFromArray(3, sizeof(short), ind);
	ArnMesh* triangle = ArnMesh::createFromVbIb(vb, ib);
	triangle->initRendererObject();
	triangle->setLocalXform_Trans(ArnVec3(3, 0, 0));
	triangle->setName("SimpleTriangle");
	sceneGraph->attachChild(triangle);
	g_videoMan->attachSceneGraph(sceneGraph);
}

int mainxx(int argc, char** argv)
{
	atexit(AppExitHandler); // main stack variables are not deallocated.
	std::set_new_handler(OutOfMem);
	//unsigned long a = 1000000000L;
	//int* p = new int[a];

	g_videoMan = VideoMan::create(RENDERER_GL, 640, 480, argc, argv);
	if (!g_videoMan)
	{
		fprintf(stderr, "Video manager init failed. Abort execution...\n");
		abort();
	}

	g_lists = glGenLists(1);
	glNewList(g_lists, GL_COMPILE);
	glBegin(GL_TRIANGLES);
	glColor3f(1, 0, 0);
	glVertex3f( 0.0f, 1.0f, 0.0f);					// Top
	glVertex3f(-1.0f,-1.0f, 0.0f);					// Bottom Left
	glVertex3f( 1.0f,-1.0f, 0.0f);					// Bottom Right
	glEnd();
	glEndList();

	InitializeXmlParser();
	ArnXmlString axs;


	// Create and init the scene graph instance from XML file
	// and attach that one to the video manager.

	g_sg = ArnSceneGraph::createFrom(g_testFile);
	assert(g_sg);
	g_sg->interconnect(g_sg);
	g_sg->initRendererObjects();
	g_videoMan->attachSceneGraph(g_sg);
	ArnCamera* cam = reinterpret_cast<ArnCamera*>(g_sg->findFirstNodeOfType(NDT_RT_CAMERA));
	if (cam)
	{
		g_videoMan->SetCamera(*cam);
	}
	else
	{
		g_mainCamera = ArnCamera::createFrom("Auto-generated Camera", ArnQuat::createFromEuler(0, 0, 0), ArnVec3(0, 0, 15), (float)(M_PI / 4));
		g_videoMan->SetCamera(*g_mainCamera);
	}

	g_videoMan->setUpdateFrameCallback(UpdateFrameCb);
	g_videoMan->setRenderFrameCallback(RenderFrameCb);
	g_videoMan->setReshapeCallback(ReshapeCb);
	g_videoMan->setKeyboardCallback(KeyboardCb);
	g_videoMan->setMouseCallback(MouseCb);
	g_videoMan->setClearColor(ArnColorValue4f(0.25f, 0.25f, 0.25f, 1.0f));

#ifndef WIN32
	InotifyWatch watch(g_testFile, IN_CLOSE);
	g_ino.Add(watch);
	g_ino.SetNonBlock(true);
	std::cout << "Inotify Enabled Count: " << g_ino.GetEnabledCount() << std::endl;
#endif


	float a = 1;
	// Sign (1-bit)  |        Mantissa (23-bit)         | Exponent (8-bit)
	//      0             000 0000 0000 0000 0000 0000        0000 0000
	//*(int*)&a = 0x00000100;
	printf("a = %.3f", a);

	//
	// Starting main loop!!!!!!!!!!
	//
	g_videoMan->StartMainLoop();
	//
	// Codes below will not executed!
	// Place memory deallocation and others at appExitHandler()
	//

	return 0;
}
