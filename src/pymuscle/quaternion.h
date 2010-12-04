/*
 * quaternion.h - header file for quaternion.c
 * Copyright (c) 2008 Cesare Tirabassi <norsetto@ubuntu.com>
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

#ifndef QUATERNION_H
#define QUATERNION_H

#include <math.h>
#include <GL/gl.h>

#include "vector.h"

//Globals

#define Q_TOL 0.000001f

// Function prototypes

void euler2quat(VECTOR4D_PTR Q, GLfloat roll, GLfloat pitch, GLfloat yaw);
void rot2quat(VECTOR4D_PTR Q, GLfloat angle, VECTOR3D_PTR axis);
void mv2quat(VECTOR4D_PTR Q, GLfloat mv[16]);
void quat2rot(GLfloat* angle, VECTOR3D_PTR axis, VECTOR4D_PTR Q);
void quat2mv(GLfloat mv[16], VECTOR4D_PTR Q);
void q_mul(VECTOR4D_PTR Qr, VECTOR4D_PTR Q1, VECTOR4D_PTR Q2);
void slerp(VECTOR4D_PTR Q, VECTOR4D_PTR Q1, VECTOR4D_PTR Q2, GLfloat t);

#endif //QUATERNION_H
