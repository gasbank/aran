/*
 * quaternion.c - quaternion handling routines
 *
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

#include "quaternion.h"

/*
 * Builds a quaternion from 3 euler angles (1-2-3)
 *
 */

void euler2quat(VECTOR4D_PTR Q, GLfloat roll, GLfloat pitch, GLfloat yaw) {

  GLfloat cosR = cosf(roll*0.5f);
  GLfloat cosP = cosf(pitch*0.5f);
  GLfloat cosY = cosf(yaw*0.5f);

  GLfloat sinR = sinf(roll*0.5f);
  GLfloat sinP = sinf(pitch*0.5f);
  GLfloat sinY = sinf(yaw*0.5f);
	
  Q->x = sinR * cosP * cosY - cosR * sinP * sinY;
  Q->y = cosR * sinP * cosY + sinR * cosP * sinY;
  Q->z = cosR * cosP * sinY - sinR * sinP * cosY;
  Q->w = cosR * cosP * cosY + sinR * sinP * sinY;
}

/*
 * Builds a quaternion from rotation axis and angle
 *
 */

void rot2quat(VECTOR4D_PTR Q, GLfloat angle, VECTOR3D_PTR axis) {

  GLfloat sa = sinf(angle*0.5f);

  Q->x = axis->x * sa;
  Q->y = axis->y * sa;
  Q->z = axis->z * sa;
  Q->w = cosf(angle*0.5f);
}

/*
 * Builds a quaternion from a modelview matrix
 *
 */

void mv2quat(VECTOR4D_PTR Q, GLfloat mv[16]) {

  GLfloat s= mv[0]+mv[5]+mv[10]+mv[15];

  if( s > 1.0f ) {

    s = sqrtf(s);
    Q->w = s*0.5f;
    s = 0.5f/s;
    Q->x = (mv[9] - mv[6]) * s;
    Q->y = (mv[2] - mv[8]) * s;
    Q->z = (mv[4] - mv[1]) * s;

  }
  else {

    GLuint greatest=0;

    if(mv[5]>mv[0])
	greatest=1;
    if(mv[10]>mv[5])
	greatest=2;

    switch(greatest) {
    case 0:
      s = sqrtf(mv[0]-mv[5]-mv[10]+mv[15]);
      if( fabs(s) > Q_TOL )
	s = 0.5f/s;
      else
	s = 0.5f;

      Q->x = 0.5f * s;
      Q->y = (mv[1] + mv[4]) * s;
      Q->z = (mv[2] + mv[8]) * s;
      Q->w = (mv[6] + mv[9]) * s;
      break;
    case 1:
      s = sqrtf(mv[5]-mv[0]-mv[10]+mv[15]);
      if( fabs(s) > Q_TOL )
	s = 0.5f/s;
      else
	s = 0.5f;

      Q->y = 0.5f * s;
      Q->x = (mv[1] + mv[4]) * s;
      Q->w = (mv[2] + mv[8]) * s;
      Q->z = (mv[6] + mv[9]) * s;
      break;
    case 2:
      s = sqrtf(mv[10]-mv[0]-mv[5]+mv[15]);
      if( fabs(s) > Q_TOL )
	s = 0.5f/s;
      else
	s = 0.5f;

      Q->z = 0.5f * s;
      Q->w = (mv[1] + mv[4]) * s;
      Q->x = (mv[2] + mv[8]) * s;
      Q->y = (mv[6] + mv[9]) * s;
    }
  }
}

/*
 * Builds a rotation axis and angle from a quaternion
 *
 */

void quat2rot(GLfloat* angle, VECTOR3D_PTR axis, VECTOR4D_PTR Q) {

  GLfloat ca = Q->w;
  *angle = 2.0f * acosf( ca );
  GLfloat sa = sqrtf( 1.0f - ca * ca );

  if ( fabs(sa) > Q_TOL )
    sa = 1.0f/sa;
  else
    sa = 1.0f;

  axis->x = Q->x * sa;
  axis->y = Q->y * sa;
  axis->z = Q->z * sa;
}

/*
 * Builds a modelview matrix from a quaternion
 *
 */

void quat2mv(GLfloat mv[16], VECTOR4D_PTR Q) {

  GLfloat xx = Q->x * Q->x;
  GLfloat xy = Q->x * Q->y;
  GLfloat xz = Q->x * Q->z;
  GLfloat xw = Q->x * Q->w;

  GLfloat yy = Q->y * Q->y;
  GLfloat yz = Q->y * Q->z;
  GLfloat yw = Q->y * Q->w;

  GLfloat zz = Q->z * Q->z;
  GLfloat zw = Q->z * Q->w;

  mv[ 0] = 1.0f - 2.0f * ( yy + zz );
  mv[ 1] =        2.0f * ( xy - zw );
  mv[ 2] =        2.0f * ( xz + yw );

  mv[ 4] =        2.0f * ( xy + zw );
  mv[ 5] = 1.0f - 2.0f * ( xx + zz );
  mv[ 6] =        2.0f * ( yz - xw );

  mv[ 8] =        2.0f * ( xz - yw );
  mv[ 9] =        2.0f * ( yz + xw );
  mv[10] = 1.0f - 2.0f * ( xx + yy );

}

/*
 * Multiplies two quaternions
 *
 */

void q_mul(VECTOR4D_PTR Qr, VECTOR4D_PTR Q1, VECTOR4D_PTR Q2) {

  GLfloat A = (Q1->x + Q1->z)*(Q2->x + Q2->y);
  GLfloat B = (Q1->x - Q1->z)*(Q2->x - Q2->y);
  GLfloat C = (Q1->w + Q1->y)*(Q2->w - Q2->z);
  GLfloat D = (Q1->w - Q1->y)*(Q2->w + Q2->z);

  Qr->w = (Q1->z - Q1->y)*(Q2->y - Q2->z) + (-A - B + C + D)*0.5f;
  Qr->x = (Q1->w + Q1->x)*(Q2->w + Q2->x) - ( A + B + C + D)*0.5f;
  Qr->y = (Q1->w - Q1->x)*(Q2->y + Q2->z) + ( A - B + C - D)*0.5f;
  Qr->z = (Q1->y + Q1->z)*(Q2->w - Q2->x) + ( A - B - C + D)*0.5f;

}

/*
 * Spherical linear interpolation between two quaternions
 *
 */

void slerp(VECTOR4D_PTR Q, VECTOR4D_PTR Q1, VECTOR4D_PTR Q2, GLfloat t) {

  GLboolean flip = GL_FALSE;
  GLfloat a, ca, sa;
  GLfloat s0, s1;
	
  ca = Q1->x*Q2->x + Q1->y*Q2->y + Q1->z*Q2->z + Q1->w*Q2->w;
	
  if (ca < 0.0f) {
    ca = -ca;
    flip = GL_TRUE;
  }
  
  if ((1.0f - ca) > Q_TOL) {
    a = acosf(ca);
    sa = sinf(a);
    s0 = sinf((1.0f - t) * a) / sa;
    s1 = sinf(t * a) / sa;
  }
  else {        
    s0 = 1.0f - t;
    s1 = t;
  }
  
  if(flip)
    s1 = -s1;

  Q->x = s0 * Q1->x + s1 * Q2->x;
  Q->y = s0 * Q1->y + s1 * Q2->y;
  Q->z = s0 * Q1->z + s1 * Q2->z;
  Q->w = s0 * Q1->w + s1 * Q2->w;
}
