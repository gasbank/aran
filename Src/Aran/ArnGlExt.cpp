#include "AranPCH.h"

#ifdef WIN32
PFNGLMULTTRANSPOSEMATRIXFPROC			glMultTransposeMatrixf;

PFNGLGENBUFFERSARBPROC					glGenBuffersARB;
PFNGLBINDBUFFERARBPROC					glBindBufferARB;
PFNGLBUFFERDATAARBPROC					glBufferDataARB;
PFNGLBUFFERSUBDATAARBPROC				glBufferSubDataARB;
PFNGLDELETEBUFFERSARBPROC				glDeleteBuffersARB;
PFNGLGETBUFFERPARAMETERIVARBPROC		glGetBufferParameterivARB;
PFNGLMAPBUFFERARBPROC					glMapBufferARB;
PFNGLUNMAPBUFFERARBPROC					glUnmapBufferARB;

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


ARAN_API void ArnInitGlExtFunctions()
{
	glMultTransposeMatrixf		= (PFNGLMULTTRANSPOSEMATRIXFPROC)wglGetProcAddress("glMultTransposeMatrixf");

	glGenBuffersARB				= (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
	glBindBufferARB				= (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
	glBufferDataARB				= (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
	glBufferSubDataARB			= (PFNGLBUFFERSUBDATAARBPROC)wglGetProcAddress("glBufferSubDataARB");
	glDeleteBuffersARB			= (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");
	glGetBufferParameterivARB	= (PFNGLGETBUFFERPARAMETERIVARBPROC)wglGetProcAddress("glGetBufferParameterivARB");
	glMapBufferARB				= (PFNGLMAPBUFFERARBPROC)wglGetProcAddress("glMapBufferARB");
	glUnmapBufferARB			= (PFNGLUNMAPBUFFERARBPROC)wglGetProcAddress("glUnmapBufferARB");

	glActiveTextureARB			= (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
	glClientActiveTextureARB	= (PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB");
	glMultiTexCoord1dARB		= (PFNGLMULTITEXCOORD1DARBPROC)wglGetProcAddress("glMultiTexCoord1dARB");
	glMultiTexCoord1dvARB		= (PFNGLMULTITEXCOORD1DVARBPROC)wglGetProcAddress("glMultiTexCoord1dvARB");
	glMultiTexCoord1fARB		= (PFNGLMULTITEXCOORD1FARBPROC)wglGetProcAddress("glMultiTexCoord1fARB");
	glMultiTexCoord1fvARB		= (PFNGLMULTITEXCOORD1FVARBPROC)wglGetProcAddress("glMultiTexCoord1fvARB");
	glMultiTexCoord1iARB		= (PFNGLMULTITEXCOORD1IARBPROC)wglGetProcAddress("glMultiTexCoord1iARB");
	glMultiTexCoord1ivARB		= (PFNGLMULTITEXCOORD1IVARBPROC)wglGetProcAddress("glMultiTexCoord1ivARB");
	glMultiTexCoord1sARB		= (PFNGLMULTITEXCOORD1SARBPROC)wglGetProcAddress("glMultiTexCoord1sARB");
	glMultiTexCoord1svARB		= (PFNGLMULTITEXCOORD1SVARBPROC)wglGetProcAddress("glMultiTexCoord1svARB");
	glMultiTexCoord2dARB		= (PFNGLMULTITEXCOORD2DARBPROC)wglGetProcAddress("glMultiTexCoord2dARB");
	glMultiTexCoord2dvARB		= (PFNGLMULTITEXCOORD2DVARBPROC)wglGetProcAddress("glMultiTexCoord2dvARB");
	glMultiTexCoord2fARB		= (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
	glMultiTexCoord2fvARB		= (PFNGLMULTITEXCOORD2FVARBPROC)wglGetProcAddress("glMultiTexCoord2fvARB");
	glMultiTexCoord2iARB		= (PFNGLMULTITEXCOORD2IARBPROC)wglGetProcAddress("glMultiTexCoord2iARB");
	glMultiTexCoord2ivARB		= (PFNGLMULTITEXCOORD2IVARBPROC)wglGetProcAddress("glMultiTexCoord2ivARB");
	glMultiTexCoord2sARB		= (PFNGLMULTITEXCOORD2SARBPROC)wglGetProcAddress("glMultiTexCoord2sARB");
	glMultiTexCoord2svARB		= (PFNGLMULTITEXCOORD2SVARBPROC)wglGetProcAddress("glMultiTexCoord2svARB");
	glMultiTexCoord3dARB		= (PFNGLMULTITEXCOORD3DARBPROC)wglGetProcAddress("glMultiTexCoord3dARB");
	glMultiTexCoord3dvARB		= (PFNGLMULTITEXCOORD3DVARBPROC)wglGetProcAddress("glMultiTexCoord3dvARB");
	glMultiTexCoord3fARB		= (PFNGLMULTITEXCOORD3FARBPROC)wglGetProcAddress("glMultiTexCoord3fARB");
	glMultiTexCoord3fvARB		= (PFNGLMULTITEXCOORD3FVARBPROC)wglGetProcAddress("glMultiTexCoord3fvARB");
	glMultiTexCoord3iARB		= (PFNGLMULTITEXCOORD3IARBPROC)wglGetProcAddress("glMultiTexCoord3iARB");
	glMultiTexCoord3ivARB		= (PFNGLMULTITEXCOORD3IVARBPROC)wglGetProcAddress("glMultiTexCoord3ivARB");
	glMultiTexCoord3sARB		= (PFNGLMULTITEXCOORD3SARBPROC)wglGetProcAddress("glMultiTexCoord3sARB");
	glMultiTexCoord3svARB		= (PFNGLMULTITEXCOORD3SVARBPROC)wglGetProcAddress("glMultiTexCoord3svARB");
	glMultiTexCoord4dARB		= (PFNGLMULTITEXCOORD4DARBPROC)wglGetProcAddress("glMultiTexCoord4dARB");
	glMultiTexCoord4dvARB		= (PFNGLMULTITEXCOORD4DVARBPROC)wglGetProcAddress("glMultiTexCoord4dvARB");
	glMultiTexCoord4fARB		= (PFNGLMULTITEXCOORD4FARBPROC)wglGetProcAddress("glMultiTexCoord4fARB");
	glMultiTexCoord4fvARB		= (PFNGLMULTITEXCOORD4FVARBPROC)wglGetProcAddress("glMultiTexCoord4fvARB");
	glMultiTexCoord4iARB		= (PFNGLMULTITEXCOORD4IARBPROC)wglGetProcAddress("glMultiTexCoord4iARB");
	glMultiTexCoord4ivARB		= (PFNGLMULTITEXCOORD4IVARBPROC)wglGetProcAddress("glMultiTexCoord4ivARB");
	glMultiTexCoord4sARB		= (PFNGLMULTITEXCOORD4SARBPROC)wglGetProcAddress("glMultiTexCoord4sARB");
	glMultiTexCoord4svARB		= (PFNGLMULTITEXCOORD4SVARBPROC)wglGetProcAddress("glMultiTexCoord4svARB");
}

#endif

#ifndef WIN32
ARAN_API void ArnInitGlExtFunctions()
{
	// Do nothing on linux
}
#endif
