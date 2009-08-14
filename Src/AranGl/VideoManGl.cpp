#include "AranGlPCH.h"
#include "VideoManGl.h"
#include "ArnSceneGraph.h"
#include "ArnLight.h"
#include "ArnViewportData.h"
#include "ArnMesh.h"
#include "ArnMath.h"
#include "AranGl.h"

VideoManGl::VideoManGl()
: m_glInfoValid(false)
, m_vboSupported(false)
{
}

void
VideoManGl::setWorldViewProjection( const ArnMatrix& matWorld, const ArnMatrix& matView, const ArnMatrix& matProj )
{

}

void
VideoManGl::setReshapeCallback( void reshape(int,int) )
{
	//glutReshapeFunc(reshape);
}

void
VideoManGl::setKeyboardCallback(void keyboardCB(unsigned char, int, int))
{
	//glutKeyboardFunc(keyboardCB);
}

void
VideoManGl::setMouseCallback( void mouseCB(int, int, int, int) )
{
	//glutMouseFunc(mouseCB);
}


HRESULT
VideoManGl::StartMainLoop()
{
	//glutMainLoop();
	return S_OK;
}

void
VideoManGl::clearFrameBuffer()
{
	// Should be start of rendering
	VideoMan::Draw();

	const ArnColorValue4f& cc = getClearColor();
	glClearColor(cc.r, cc.g, cc.b, cc.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();									// Reset The Current Modelview Matrix
}

void
VideoManGl::swapFrameBuffer()
{
	//glutSwapBuffers();
	// Should be end of rendering
}

void
VideoManGl::setupViewMatrix() const
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	const ARN_CAMERA* mc = getMainCamera();
	gluLookAt(
		mc->eye.x, mc->eye.y, mc->eye.z,
		mc->at.x, mc->at.y, mc->at.z,
		mc->up.x, mc->up.y, mc->up.z );
}

void
VideoManGl::setupGlInfo()
{
	m_glInfo.getInfo();
	m_glInfoValid = true;

	m_glInfo.printSelf();
}

bool
VideoManGl::setupVboSupport() const
{
	if (!m_glInfoValid)
	{
		std::cout << "This method should be called after setupGlInfo()" << std::endl;
		return false;
	}
	bool ret = false;

	if(m_glInfo.isExtensionSupported("GL_ARB_vertex_buffer_object"))
	{
		ret = true;
		std::cout << "Video card supports GL_ARB_vertex_buffer_object." << std::endl;
	}
	else
	{
		ret = false;
		std::cout << "Video card does NOT support GL_ARB_vertex_buffer_object." << std::endl;
	}

	if (!ret)
	{
		// Your graphic card should support VBO.
		// If you are in Win32, you can use D3D9 renderer instead of OpenGL.
		abort();
	}
	return ret;
}

void
VideoManGl::setLight(int lightId, const ArnLight* light)
{
	ArnConfigureLightGl(lightId, light);
}

void IdleCb()
{
	GetVideoManager().updateTime();
	double elapsedTimeBetweenFrames = GetVideoManager().getTime() - GetVideoManager().getLastRefreshedTime();
	GetVideoManager().updateFrame( GetVideoManager().getTime(), float(elapsedTimeBetweenFrames) );
	if (elapsedTimeBetweenFrames > 1.0 / 60)
	{
		//printf("Post redisplay called....");
		//glutPostRedisplay();
	}
}

void DisplayCb()
{
	VideoMan& vm = GetVideoManager();
	if (vm.setRenderStarted())
	{
		vm.renderFrame();
		vm.setRenderFinished();
	}
}

VideoManGl*
VideoManGl::create(int argc, char** argv, int width, int height)
{
	/*
	glutInit(&argc, argv);
	glutInitDisplayMode ( GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA );
	glutInitWindowSize  ( width, height );
	glutCreateWindow    ( "VideoMan" );
	glewInit();
	*/

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	/*
	glutIdleFunc(IdleCb);
	glutDisplayFunc(DisplayCb);
	*/
	glEnable(GL_LIGHTING);
	for (int lightId = 0; lightId < 8; ++lightId)
	{
		glDisable(GL_LIGHT0 + lightId);
	}
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//glFrontFace(GL_CCW);


	VideoManGl* vmgl = new VideoManGl();
	vmgl->setupGlInfo();
	vmgl->setupVboSupport();

	return vmgl;
}

void
VideoManGl::setupProjectionMatrix() const
{
	glViewport(0, 0, GetWindowSizeW(), GetWindowSizeH());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float fovdeg = ArnToDegree(getMainCamera()->angle);
	float aspect = 0;
	if (GetWindowSizeH() == 0)
		aspect = (float)GetWindowSizeW();
	else
		aspect = (float)GetWindowSizeW() / GetWindowSizeH();
	gluPerspective(fovdeg, aspect, getMainCamera()->nearClip, getMainCamera()->farClip);
	glMatrixMode(GL_MODELVIEW);
}
