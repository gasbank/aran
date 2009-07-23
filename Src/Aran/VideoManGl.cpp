#include "AranPCH.h"
#include "VideoManGl.h"
#include "ArnSceneGraph.h"
#include "ArnLight.h"

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

#ifdef _WIN32
	/*
	// check VBO is supported by your video card
	if(m_glInfo.isExtensionSupported("GL_ARB_vertex_buffer_object"))
	{
		// get pointers to GL functions
		glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
		glBindBufferARB = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
		glBufferDataARB = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
		glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)wglGetProcAddress("glBufferSubDataARB");
		glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");
		glGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC)wglGetProcAddress("glGetBufferParameterivARB");
		glMapBufferARB = (PFNGLMAPBUFFERARBPROC)wglGetProcAddress("glMapBufferARB");
		glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)wglGetProcAddress("glUnmapBufferARB");

		// check once again VBO extension
		if(glGenBuffersARB && glBindBufferARB && glBufferDataARB && glBufferSubDataARB &&
			glMapBufferARB && glUnmapBufferARB && glDeleteBuffersARB && glGetBufferParameterivARB)
		{
			ret = true;
			std::cout << "Video card supports GL_ARB_vertex_buffer_object." << std::endl;
		}
		else
		{
			ret = false;
			std::cout << "Video card does NOT support GL_ARB_vertex_buffer_object." << std::endl;
		}
	}
	else
	{
		ret = false;
		std::cout << "Video card does NOT support GL_ARB_vertex_buffer_object." << std::endl;
	}
	*/
#else // for linux, do not need to get function pointers, it is up-to-date
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
#endif

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


/////////////////////////////////////////////////////////////////////////////////////////

void ArnConfigureViewportProjectionMatrixGl(const ArnViewportData* viewportData, const ArnCamera* cam)
{
	glViewport(viewportData->X, viewportData->Y, viewportData->Width, viewportData->Height);
	ArnConfigureProjectionMatrixGl(viewportData, cam);
}

void ArnConfigureProjectionMatrixGl( const ArnViewportData* viewportData, const ArnCamera* cam )
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float fovdeg = ArnToDegree(cam->getFov()); // OpenGL uses degrees as angle unit rather than radians.
	float aspect = 0;
	if (viewportData->Height == 0)
		aspect = (float)viewportData->Width;
	else
		aspect = (float)viewportData->Width / viewportData->Height;
	gluPerspective(fovdeg, aspect, cam->getNearClip(), cam->getFarClip());
}


void ArnConfigureViewMatrixGl(ArnCamera* cam)
{
	ArnMatrix localTf = cam->getLocalXform();

	ARN_CAMERA mainCamera;
	mainCamera.eye.x = localTf.m[0][3];
	mainCamera.eye.y = localTf.m[1][3];
	mainCamera.eye.z = localTf.m[2][3];
	mainCamera.at.x = localTf.m[0][3] - localTf.m[0][2];
	mainCamera.at.y = localTf.m[1][3] - localTf.m[1][2];
	mainCamera.at.z = localTf.m[2][3] - localTf.m[2][2];
	mainCamera.up.x = localTf.m[0][1];
	mainCamera.up.y = localTf.m[1][1];
	mainCamera.up.z = localTf.m[2][1];
	mainCamera.nearClip = cam->getNearClip();
	mainCamera.farClip = cam->getFarClip();
	mainCamera.angle = cam->getFov(); // FOV in radian

	cam->getCameraData().pos = mainCamera.eye;
	cam->getCameraData().upVector = mainCamera.up;
	cam->getCameraData().targetPos = mainCamera.at;
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		mainCamera.eye.x, mainCamera.eye.y, mainCamera.eye.z,
		mainCamera.at.x, mainCamera.at.y, mainCamera.at.z,
		mainCamera.up.x, mainCamera.up.y, mainCamera.up.z );
}

void ArnConfigureLightGl(GLuint lightId, const ArnLight* light)
{
	assert(lightId < 8);
	if (light)
	{
		DWORD lightType = light->getD3DLightData().Type;
		if (lightType == 1)
		{
			// Point light (e.g. bulb)
			ArnVec4 pos(light->getD3DLightData().Position, 1);
			float a0 = 0.001f;
			float a1 = 0.1f;
			float a2 = 0.00001f;
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			{
				//glLoadIdentity();
				glLightfv(GL_LIGHT0 + lightId, GL_POSITION, pos.getRawData());
				glLightfv(GL_LIGHT0 + lightId, GL_SPOT_DIRECTION, (const GLfloat*)&POINT4FLOAT::ZERO);
			}
			glPopMatrix();
			glLightfv(GL_LIGHT0 + lightId, GL_CONSTANT_ATTENUATION, &a0);
			glLightfv(GL_LIGHT0 + lightId, GL_LINEAR_ATTENUATION, &a1);
			glLightfv(GL_LIGHT0 + lightId, GL_QUADRATIC_ATTENUATION, &a2);

			//pos.printFormatString();
		}
		else if (lightType == 2)
		{
			// Spot light
			ArnVec4 pos(light->getD3DLightData().Position, 1);
			ArnVec4 dir(light->getD3DLightData().Direction, 1);
			float cutoff = 90;
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			{
				//glLoadIdentity();
				glLightfv(GL_LIGHT0 + lightId, GL_POSITION, pos.getRawData());
				glLightfv(GL_LIGHT0 + lightId, GL_SPOT_DIRECTION, dir.getRawData());
			}
			glPopMatrix();
			glLightfv(GL_LIGHT0 + lightId, GL_SPOT_CUTOFF, &cutoff);
		}
		else if (lightType == 3)
		{
			// Directional light (e.g. sunlight)
			ArnVec4 dir(light->getD3DLightData().Direction, 1);
			float cutoff = 180;
			glPushMatrix();
			{
				//glLoadIdentity();
				glLightfv(GL_LIGHT0 + lightId, GL_SPOT_DIRECTION, dir.getRawData());
			}
			glPopMatrix();
			glLightfv(GL_LIGHT0 + lightId, GL_SPOT_CUTOFF, &cutoff);
		}
		else
		{
			ARN_THROW_UNEXPECTED_CASE_ERROR
		}

		glLightfv(GL_LIGHT0 + lightId, GL_AMBIENT, (const GLfloat*)&light->getD3DLightData().Ambient);
		glLightfv(GL_LIGHT0 + lightId, GL_DIFFUSE, (const GLfloat*)&light->getD3DLightData().Diffuse);
		glLightfv(GL_LIGHT0 + lightId, GL_SPECULAR, (const GLfloat*)&light->getD3DLightData().Specular);
		//float exponent = 64;
		//glLightfv(GL_LIGHT0 + lightId, GL_SPOT_EXPONENT, (const GLfloat*)&exponent);

		glEnable(GL_LIGHT0 + lightId);
		//light->getD3DLightData().Ambient.printFormatString();
	}
	else
	{
		glDisable(GL_LIGHT0 + lightId);
	}
}

void ArnDrawAxesGl(float size)
{
	glPushMatrix();
	glPushAttrib(GL_LINE_BIT);
	{
		glLineWidth(2);
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glColor3f(1,0,0);		glVertex3f(0,0,0);		glVertex3f(size,0,0);
		glColor3f(0,1,0);		glVertex3f(0,0,0);		glVertex3f(0,size,0);
		glColor3f(0,0,1);		glVertex3f(0,0,0);		glVertex3f(0,0,size);
		glEnd();
		glEnable(GL_LIGHTING);
	}
	glPopAttrib();
	glPopMatrix();
}

GLuint ArnCreateNormalizationCubeMapGl()
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, tex);
	
	unsigned char * data = new unsigned char[32*32*3];
	if(!data)
	{
		printf("Unable to allocate memory for texture data for cube map\n");
		return 0;
	}

	//some useful variables
	int size=32;
	float offset=0.5f;
	float halfSize=16.0f;
	ArnVec3 tempVector;
	unsigned char * bytePtr;

	//positive x
	bytePtr=data;
	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tempVector.x = halfSize;
			tempVector.y = -(j+offset-halfSize);
			tempVector.z = -(i+offset-halfSize);
			ArnVec3Normalize(&tempVector, &tempVector);
			tempVector.x = (tempVector.x + 1) / 2;
			tempVector.y = (tempVector.y + 1) / 2;
			tempVector.z = (tempVector.z + 1) / 2;
			bytePtr[0]=(unsigned char)(tempVector.x * 255);
			bytePtr[1]=(unsigned char)(tempVector.y * 255);
			bytePtr[2]=(unsigned char)(tempVector.z * 255);
			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	//negative x
	bytePtr=data;
	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tempVector.x = -halfSize;
			tempVector.y = -(j+offset-halfSize);
			tempVector.z = i+offset-halfSize;
			ArnVec3Normalize(&tempVector, &tempVector);
			tempVector.x = (tempVector.x + 1) / 2;
			tempVector.y = (tempVector.y + 1) / 2;
			tempVector.z = (tempVector.z + 1) / 2;
			bytePtr[0]=(unsigned char)(tempVector.x * 255);
			bytePtr[1]=(unsigned char)(tempVector.y * 255);
			bytePtr[2]=(unsigned char)(tempVector.z * 255);
			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	//positive y
	bytePtr=data;
	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tempVector.x = i+offset-halfSize;
			tempVector.y = halfSize;
			tempVector.z = j+offset-halfSize;
			ArnVec3Normalize(&tempVector, &tempVector);
			tempVector.x = (tempVector.x + 1) / 2;
			tempVector.y = (tempVector.y + 1) / 2;
			tempVector.z = (tempVector.z + 1) / 2;
			bytePtr[0]=(unsigned char)(tempVector.x * 255);
			bytePtr[1]=(unsigned char)(tempVector.y * 255);
			bytePtr[2]=(unsigned char)(tempVector.z * 255);
			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	//negative y
	bytePtr=data;
	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tempVector.x = i+offset-halfSize;
			tempVector.y = -halfSize;
			tempVector.z = -(j+offset-halfSize);
			ArnVec3Normalize(&tempVector, &tempVector);
			tempVector.x = (tempVector.x + 1) / 2;
			tempVector.y = (tempVector.y + 1) / 2;
			tempVector.z = (tempVector.z + 1) / 2;
			bytePtr[0]=(unsigned char)(tempVector.x * 255);
			bytePtr[1]=(unsigned char)(tempVector.y * 255);
			bytePtr[2]=(unsigned char)(tempVector.z * 255);
			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	//positive z
	bytePtr=data;
	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tempVector.x = i+offset-halfSize;
			tempVector.y = -(j+offset-halfSize);
			tempVector.z = halfSize;
			ArnVec3Normalize(&tempVector, &tempVector);
			tempVector.x = (tempVector.x + 1) / 2;
			tempVector.y = (tempVector.y + 1) / 2;
			tempVector.z = (tempVector.z + 1) / 2;
			bytePtr[0]=(unsigned char)(tempVector.x * 255);
			bytePtr[1]=(unsigned char)(tempVector.y * 255);
			bytePtr[2]=(unsigned char)(tempVector.z * 255);
			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	//negative z
	bytePtr=data;
	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tempVector.x = -(i+offset-halfSize);
			tempVector.y = -(j+offset-halfSize);
			tempVector.z = -halfSize;
			ArnVec3Normalize(&tempVector, &tempVector);
			tempVector.x = (tempVector.x + 1) / 2;
			tempVector.y = (tempVector.y + 1) / 2;
			tempVector.z = (tempVector.z + 1) / 2;
			bytePtr[0]=(unsigned char)(tempVector.x * 255);
			bytePtr[1]=(unsigned char)(tempVector.y * 255);
			bytePtr[2]=(unsigned char)(tempVector.z * 255);
			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	delete [] data;

	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, 0);
	return tex;
}

