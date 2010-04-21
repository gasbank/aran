#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

OpenGL primitives shape definition
"""
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
from dynamics import *



def RenderAxis(size=1.):
	axis=[[size,0,0],[0,size,0],[0,0,size]]
	col=[[1,0,0],[0,1,0],[0,0,1]]
	glBegin(GL_LINES)
	for a,c in zip(axis,col):
		glColor3f(c[0],c[1],c[2])
		glVertex3f(0,0,0)
		glVertex3f(a[0],a[1],a[2])
	glEnd()

def RenderArrow(quadric, arrowlen, arrowtiplen, arrowthickness):
	glPushMatrix()
	gluCylinder(quadric, arrowthickness, arrowthickness, arrowlen, 32, 8);
	glTranslatef(0,0,arrowlen)

	# Rotate the bottom cap to flip the normal
	glPushMatrix()
	glRotate(180,1,0,0)
	gluDisk(quadric, 0, arrowthickness*3, 32, 1)
	glPopMatrix()
	
	gluCylinder(quadric, arrowthickness*3, 0, arrowtiplen, 32, 8);
	glPopMatrix()
	
def RenderFancyGlobalAxis(quadric, arrowlen, arrowtiplen, arrowthickness):
	glPushMatrix()
	glColor3f(0,0,1)
	RenderArrow(quadric, arrowlen, arrowtiplen, arrowthickness)
	glRotatef(90, 0, 1, 0)
	glColor3f(1,0,0)
	RenderArrow(quadric, arrowlen, arrowtiplen, arrowthickness)
	glRotatef(-90, 1, 0, 0)
	glColor3f(0,1,0)
	RenderArrow(quadric, arrowlen, arrowtiplen, arrowthickness)
	glPopMatrix()

# // build the display list.
def BuildList():
	cube = glGenLists(1);              # // generate storage for 2 lists, and return a pointer to the first.
	glNewList(cube, GL_COMPILE);       # // store this list at location cube, and compile it once.

	# // cube without the top;
	glBegin(GL_QUADS);			# // Bottom Face

	glTexCoord2f(1.0, 1.0); 
	glVertex3f(-1.0, -1.0, -1.0);	# // Top Right Of The Texture and Quad
	glTexCoord2f(0.0, 1.0); 
	glVertex3f( 1.0, -1.0, -1.0);	# // Top Left Of The Texture and Quad
	glTexCoord2f(0.0, 0.0); 
	glVertex3f( 1.0, -1.0,  1.0);	# // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0, 0.0); 
	glVertex3f(-1.0, -1.0,  1.0);	# // Bottom Right Of The Texture and Quad

	# // Front Face
	glTexCoord2f(0.0, 0.0); 
	glVertex3f(-1.0, -1.0,  1.0);	# // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0, 0.0); 
	glVertex3f( 1.0, -1.0,  1.0);	# // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0, 1.0); 
	glVertex3f( 1.0,  1.0,  1.0);	# // Top Right Of The Texture and Quad
	glTexCoord2f(0.0, 1.0); 
	glVertex3f(-1.0,  1.0,  1.0);	# // Top Left Of The Texture and Quad

	# // Back Face
	glTexCoord2f(1.0, 0.0); 
	glVertex3f(-1.0, -1.0, -1.0);	# // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0, 1.0); 
	glVertex3f(-1.0,  1.0, -1.0);	# // Top Right Of The Texture and Quad
	glTexCoord2f(0.0, 1.0); 
	glVertex3f( 1.0,  1.0, -1.0);	# // Top Left Of The Texture and Quad
	glTexCoord2f(0.0, 0.0); 
	glVertex3f( 1.0, -1.0, -1.0);	# // Bottom Left Of The Texture and Quad

	# // Right face
	glTexCoord2f(1.0, 0.0); 
	glVertex3f( 1.0, -1.0, -1.0);	# // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0, 1.0); 
	glVertex3f( 1.0,  1.0, -1.0);	# // Top Right Of The Texture and Quad
	glTexCoord2f(0.0, 1.0); 
	glVertex3f( 1.0,  1.0,  1.0);	# // Top Left Of The Texture and Quad
	glTexCoord2f(0.0, 0.0); 
	glVertex3f( 1.0, -1.0,  1.0);	# // Bottom Left Of The Texture and Quad

	# // Left Face
	glTexCoord2f(0.0, 0.0); 
	glVertex3f(-1.0, -1.0, -1.0);	# // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0, 0.0); 
	glVertex3f(-1.0, -1.0,  1.0);	# // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0, 1.0); 
	glVertex3f(-1.0,  1.0,  1.0);	# // Top Right Of The Texture and Quad
	glTexCoord2f(0.0, 1.0); 
	glVertex3f(-1.0,  1.0, -1.0);	# // Top Left Of The Texture and Quad

	glEnd();
	glEndList();

	print "List built."
	return cube


