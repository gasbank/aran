/*
 * camera.h - header file for camera.c
 * Copyright (c) 2007 Cesare Tirabassi <norsetto@ubuntu.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <GL/gl.h>
#include <math.h>
#include "vector.h"

//Globals

GLfloat xCam, yCam, zCam;	//Camera position
GLfloat xAt, yAt, zAt;		//Camera at-vector
GLfloat xUp, yUp, zUp;		//Camera up-vector
GLfloat xRt, zRt;		//Camera right vector (at X up) NB yRt is always 0 by definition!

VECTOR3D Cam, At, Up, Rt;

GLfloat eCam, aCam;		//Camera elevation and azimuth angles
				//eCam = aCam = 0 correspond to a look-at vector = -Zw = (0,0,-1)
GLfloat lCam, vCam, dCam;	//lateral, vertical and directional motion of camera (differential)

//Function prototypes
void setup_camera( GLfloat dt );

#endif //CAMERA_H
