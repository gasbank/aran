#include "AranGlPCH.h"
#include "BasicObjects.h"

static GLuint compileUnitBox()
{
	GLuint listId = glGenLists(1);

    glNewList(listId, GL_COMPILE);

    glBegin(GL_QUADS);

    glNormal3f(0, 0, 1); glVertex3f( -0.5f, -0.5f, 0.5f);					// Top
    glNormal3f(0, 0, 1); glVertex3f(  0.5f, -0.5f, 0.5f);					// Top
    glNormal3f(0, 0, 1); glVertex3f(  0.5f,  0.5f, 0.5f);					// Top
	glNormal3f(0, 0, 1); glVertex3f( -0.5f,  0.5f, 0.5f);					// Top

	glNormal3f(0, 0, -1); glVertex3f( -0.5f,  0.5f, -0.5f);					// Bottom
	glNormal3f(0, 0, -1); glVertex3f(  0.5f,  0.5f, -0.5f);					// Bottom
	glNormal3f(0, 0, -1); glVertex3f(  0.5f, -0.5f, -0.5f);					// Bottom
    glNormal3f(0, 0, -1); glVertex3f( -0.5f, -0.5f, -0.5f);					// Bottom

    glNormal3f(0, -1, 0); glVertex3f( -0.5f, -0.5f, -0.5f);					// Front
    glNormal3f(0, -1, 0); glVertex3f(  0.5f, -0.5f, -0.5f);					// Front
    glNormal3f(0, -1, 0); glVertex3f(  0.5f, -0.5f,  0.5f);					// Front
    glNormal3f(0, -1, 0); glVertex3f( -0.5f, -0.5f,  0.5f);					// Front

    glNormal3f(0, 1, 0); glVertex3f( -0.5f,  0.5f,  0.5f);					// Back
    glNormal3f(0, 1, 0); glVertex3f(  0.5f,  0.5f,  0.5f);					// Back
    glNormal3f(0, 1, 0); glVertex3f(  0.5f,  0.5f, -0.5f);					// Back
    glNormal3f(0, 1, 0); glVertex3f( -0.5f,  0.5f, -0.5f);					// Back

    glNormal3f(-1, 0, 0); glVertex3f( -0.5f, -0.5f, -0.5f);					// Left
    glNormal3f(-1, 0, 0); glVertex3f( -0.5f,  0.5f, -0.5f);					// Left
    glNormal3f(-1, 0, 0); glVertex3f( -0.5f,  0.5f,  0.5f);					// Left
    glNormal3f(-1, 0, 0); glVertex3f( -0.5f, -0.5f,  0.5f);					// Left

    glNormal3f( 1, 0, 0); glVertex3f(  0.5f, -0.5f, -0.5f);					// Right
    glNormal3f( 1, 0, 0); glVertex3f(  0.5f,  0.5f, -0.5f);					// Right
    glNormal3f( 1, 0, 0); glVertex3f(  0.5f,  0.5f,  0.5f);					// Right
    glNormal3f( 1, 0, 0); glVertex3f(  0.5f, -0.5f,  0.5f);					// Right

    glEnd();

    glEndList();

    return listId;
}

void compileAllBasicObjects(BasicObjects* bo)
{
	bo->unitBox = compileUnitBox();
}
