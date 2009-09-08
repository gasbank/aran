#include "AranGlPCH.h"
#include "ArnSkinningShaderGl.h"
#include "ArnSkeleton.h"
#include "ArnBone.h"
#include "ArnMesh.h"
#include <sys/stat.h>

IMPLEMENT_SINGLETON(ArnSkinningShaderGl);

ArnSkinningShaderGl::ArnSkinningShaderGl()
{
	//ctor
}

ArnSkinningShaderGl::~ArnSkinningShaderGl()
{
	//dtor
	glDeleteObjectARB( m_vertexShader );
    glDeleteObjectARB( m_programObj );
}

unsigned char *readShaderFile( const char *fileName )
{
    FILE *file = fopen( fileName, "r" );

    if ( file == NULL )
    {
        assert(!"Cannot open shader file!");
        return 0;
    }

    struct stat fileStats;

    if ( stat( fileName, &fileStats ) != 0 )
    {
        assert(!"Cannot get file stats for shader file!");
        return 0;
    }

    unsigned char *buffer = new unsigned char[fileStats.st_size];

    int bytes = fread( buffer, 1, fileStats.st_size, file );

    buffer[bytes] = 0;

    fclose( file );

    return buffer;
}

void
ArnSkinningShaderGl::initShader()
{
	char *ext = (char*)glGetString( GL_EXTENSIONS );

	if ( strstr( ext, "GL_ARB_shading_language_100" ) == NULL )
	{
		//This extension string indicates that the OpenGL Shading Language,
		// version 1.00, is supported.
		assert(!"GL_ARB_shading_language_100 extension was not found");
		return;
	}

	//
	// GL_ARB_shader_objects
	//

	if ( strstr( ext, "GL_ARB_shader_objects" ) == NULL )
	{
		assert(!"GL_ARB_shader_objects extension was not found");
		return;
	}

	//
	// GL_ARB_vertex_shader
	//

	if ( strstr( ext, "GL_ARB_vertex_shader" ) == NULL )
	{
		assert(!"GL_ARB_vertex_shader extension was not found");
		return;
	}

	const char *vertexShaderStrings[1];
	GLint bVertCompiled;
	GLint bLinked;
	char str[4096];

	//
	// Create the vertex shader...
	//

	m_vertexShader = glCreateShaderObjectARB( GL_VERTEX_SHADER_ARB );

	unsigned char *vertexShaderAssembly = readShaderFile( "shaders/ogl_glslang_skinning_nvidia.vert" );
	//unsigned char *vertexShaderAssembly = readShaderFile( "shaders/ogl_glslang_skinning_ati.vert" );
	vertexShaderStrings[0] = (char*)vertexShaderAssembly;
	glShaderSourceARB( m_vertexShader, 1, vertexShaderStrings, NULL );
	glCompileShaderARB( m_vertexShader);
	delete vertexShaderAssembly;

	glGetObjectParameterivARB( m_vertexShader, GL_OBJECT_COMPILE_STATUS_ARB,
							   &bVertCompiled );
	if ( bVertCompiled  == false )
	{
		glGetInfoLogARB(m_vertexShader, sizeof(str), NULL, str);
		assert(!"Vertex Shader Compile Error");
	}

	//
	// Create a program object and attach our anisotropic shader to it...
	//

	m_programObj = glCreateProgramObjectARB();
	glAttachObjectARB( m_programObj, m_vertexShader );

	//
	// Link the program object and print out the info log...
	//

	glLinkProgramARB( m_programObj );
	glGetObjectParameterivARB( m_programObj, GL_OBJECT_LINK_STATUS_ARB, &bLinked );

	if ( bLinked == false )
	{
		glGetInfoLogARB( m_programObj, sizeof(str), NULL, str );
		assert(!"Linking Error");
	}

	//
	// Locate some parameters by name so we can set them later...
	//

	m_location_inverseModelView = glGetUniformLocation( m_programObj, "inverseModelView" );
	m_location_eyePosition      = glGetUniformLocation( m_programObj, "eyePosition" );
	m_location_lightVector      = glGetUniformLocation( m_programObj, "lightVector" );
	for (int i = 0; i < 32; ++i)
	{
		char varName[128];
		sprintf(varName, "boneMatrices[%d]", i);
		m_location_boneMatrices[i] = glGetUniformLocation( m_programObj, varName );
	}

	m_location_weights        = glGetAttribLocationARB( m_programObj, "weights" );
	m_location_matrixIndices  = glGetAttribLocationARB( m_programObj, "matrixIndices" );
	m_location_numBones       = glGetAttribLocationARB( m_programObj, "numBones" );

	printf(" - Skinning shader for OpenGL configured successfully.\n");
}

void
ArnSkinningShaderGl::setShaderConstants(const ArnSkeleton* skel, const std::vector<VertexGroup>& vertGroup, const ArnMesh* skinnedMesh)
{
	// TODO: Unnecessarily duplicated computation...
	int boneMatIdx = 0;
	foreach (const VertexGroup& vg, vertGroup)
	{
		const ArnNode* boneNode = skel->getConstNodeByName(vg.name);
		if (boneNode && boneNode->getType() == NDT_RT_BONE)
		{
			const ArnBone* bone = reinterpret_cast<const ArnBone*>(boneNode);
			ArnMatrix m(ArnConsts::ARNMAT_IDENTITY);

			while (bone)
			{
				const ArnXformable* boneParent = dynamic_cast<const ArnXformable*>(bone->getParent());
				ArnMatrix boneWorldMat(ArnConsts::ARNMAT_IDENTITY);

				while (boneParent)
				{
					ArnMatrix boneMat;
					ArnQuat boneQ = boneParent->getLocalXform_Rot() * boneParent->getAnimLocalXform_Rot();
					ArnMatrixTransformation(&boneMat, 0, 0, &boneParent->getLocalXform_Scale(), 0, &boneQ, &boneParent->getLocalXform_Trans());
					boneWorldMat = boneMat * boneWorldMat;

					boneParent = dynamic_cast<const ArnXformable*>(boneParent->getParent());
				}

				const ArnVec3 parentBoneTailDiff = boneWorldMat.getColumnVec3(1) * static_cast<ArnBone*>(bone->getParent())->getBoneLength();
				boneWorldMat.m[0][3] += parentBoneTailDiff.x;
				boneWorldMat.m[1][3] += parentBoneTailDiff.y;
				boneWorldMat.m[2][3] += parentBoneTailDiff.z;

				ArnMatrix boneWorldMatInv;
				ArnMatrixInverse(&boneWorldMatInv, 0, &boneWorldMat);
				const ArnMatrix meshWorldMat = skinnedMesh->computeWorldXform();

				const ArnMatrix model2BoneMat = boneWorldMatInv * meshWorldMat;
				ArnMatrix bone2ModelMat;
				ArnMatrixInverse(&bone2ModelMat, 0, &model2BoneMat);

				ArnMatrix boneRotMat;
				bone->getAnimLocalXform_Rot().getRotationMatrix(&boneRotMat);

				m *= bone2ModelMat * boneRotMat * model2BoneMat;


				bone = dynamic_cast<const ArnBone*>(bone->getParent());
			}
			// Matrices should be transposed before applied in OpenGL.
			glUniformMatrix4fvARB( m_location_boneMatrices[boneMatIdx], 1, false, reinterpret_cast<const GLfloat*>(m.transpose().m) );
			//glUniformMatrix4fvARB( m_location_boneMatrices[boneMatIdx], 1, false, reinterpret_cast<const GLfloat*>(ArnConsts::ARNMAT_IDENTITY.m) );
		}
		++boneMatIdx;
	}

	//
	// Set up some lighting...
	//
	float fEyePosition[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float fLightVector[] = { 0.0f, 0.0f, -1.0f, 0.0f };

	// Normalize light vector
	float fLength = sqrtf( fLightVector[0]*fLightVector[0] +
						   fLightVector[1]*fLightVector[1] +
						   fLightVector[2]*fLightVector[2] );
	fLightVector[0] /= fLength;
	fLightVector[1] /= fLength;
	fLightVector[2] /= fLength;

	// Set the light's directional vector
	if ( m_location_lightVector != (unsigned int)(-1) )
		glUniform4fARB( m_location_lightVector, fLightVector[0], fLightVector[1], fLightVector[2], fLightVector[3] );

	// Set the viewer's eye position
	if ( m_location_eyePosition != (unsigned int)(-1) )
		glUniform4fARB( m_location_eyePosition, fEyePosition[0], fEyePosition[1], fEyePosition[2], fEyePosition[3] );

	// Set the inverse of the current model-view matrix
	ArnMatrix modelView;
	ArnMatrix inverseModelView;
	glGetFloatv(GL_MODELVIEW_MATRIX, reinterpret_cast<GLfloat*>(modelView.m));
	ArnMatrixInverse(&inverseModelView, 0, &modelView);
	if ( m_location_inverseModelView != (unsigned int)(-1) )
		glUniformMatrix4fvARB( m_location_inverseModelView, 1, false, reinterpret_cast<const GLfloat*>(inverseModelView.m) );
	}

