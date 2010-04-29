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
	glPushAttrib(GL_LIGHTING_BIT)
	glDisable(GL_LIGHTING)
	glLineWidth(1.5)
	glBegin(GL_LINES)
	for a,c in zip(axis,col):
		glColor3f(c[0],c[1],c[2])
		glVertex3f(0,0,0)
		glVertex3f(a[0],a[1],a[2])
	glEnd()
	glPopAttrib()

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

def DrawMuscleFiber2(quadric, origin, insertion, radius1, radius2, dc):
	direction = insertion - origin
	length = linalg.norm(direction)
	direction = direction / length
	DrawMuscleFiber(quadric, origin, direction, length, radius1, radius2, dc)
	
def DrawMuscleFiber(quadric, origin, direction, length, radius1, radius2, dc):
	"""
	Origin and direction should be passed in global coordinates.
	"""
	#r1 = radius1/length
	#r2 = radius2/length
	
	r1,r2 = radius1, radius2
	#r1 = r2 = 0.02
	
	l1 = length*0.1
	l2 = length*0.1
	l3 = length*0.15
	l4 = length-(l1+l2+l3)*2
	assert(l4>0)
	direction = direction / linalg.norm(direction)
	
	glColor3f(dc[0],dc[1],dc[2]) # RED!
	glPushMatrix()

	rotAngle = acos(dot([0,0,1], direction))
	if abs(rotAngle - math.pi) < 1e-5:
		rotAxis = array([1,0,0])
	else:
		rotAxis = cross([0,0,1], direction)
	
	glTranslate(origin[0], origin[1], origin[2])
	glRotatef(rotAngle/math.pi*180, rotAxis[0], rotAxis[1], rotAxis[2])
	
	    
	glTranslate(0,0,length-l1)
	gluCylinder(quadric, r1, 0, l1, 32, 2)
	glTranslatef(0,0,-l2)
	gluCylinder(quadric, r1, r1, l2, 32, 2)
	glTranslatef(0,0,-l3)
	gluCylinder(quadric, r2, r1, l3, 32, 2)
	glTranslatef(0,0,-l4)
	gluCylinder(quadric, r2, r2, l4, 32, 2)
	glTranslatef(0,0,-l3)
	gluCylinder(quadric, r1, r2, l3, 32, 2)
	glTranslatef(0,0,-l2)
	gluCylinder(quadric, r1, r1, l2, 32, 2)
	glTranslatef(0,0,-l1)
	gluCylinder(quadric, 0, r1, l1, 32, 2)
	
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


