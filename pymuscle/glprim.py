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



def RenderAxis():
	axis=[[1,0,0],[0,1,0],[0,0,1]]
	glBegin(GL_LINES)
	for a in axis:
		glColor3f(a[0],a[1],a[2])
		glVertex3f(0,0,0)
		glVertex3f(a[0],a[1],a[2])
	glEnd()
	
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


