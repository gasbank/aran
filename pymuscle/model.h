/*
 * model.h - header file for model.c
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

#ifndef MODEL_H
#define MODEL_H

#include <math.h>
#include <stdio.h>
#include <malloc.h>
#include <GL/gl.h>

#include "vector.h"
#include "quaternion.h"
#include "common.h"

//Globals

#define MV_SIZE 16

// Data types
typedef struct BONE_TYPE
{
  GLfloat mvr[MV_SIZE]; //relative modelview matrix
  GLfloat mva[MV_SIZE]; //absolute modelview matrix
  VECTOR4D Q; //associated quaternion
  GLuint parent; //pointer to parent element

} BONE, *BONE_PTR;

typedef struct VERTEX_TYPE
{ 
  VECTOR3D pos; //eye-space vertex position
  VECTOR3D_PTR dis; //position relative to bones
  GLfloat* w; //weight relative to bones

} VERTEX, *VERTEX_PTR;

typedef struct POSE_TYPE
{ 
  VECTOR4D_PTR Q; //Bone quaternions
  GLfloat t; //Time to reach pose

} POSE, *POSE_PTR;

typedef struct MODEL_TYPE
{
  GLuint num_verts, num_bones, num_tris, num_poses;
  VERTEX_PTR vertex; //vertices
  VECTOR2D_PTR texcoord; //texture coordinates
  BONE_PTR bone; //bones
  GLuint* tris; //triangle indices
  POSE_PTR pose; //poses
  GLuint* hierarchy;

} MODEL, *MODEL_PTR;

// Function prototypes

MODEL_PTR loadModel(const char* filename);
void deleteModel(MODEL_PTR model);
void updateModel(MODEL_PTR model);
void mul_mv(GLfloat* mv, GLfloat* mva, GLfloat* mvb);

#endif //MODEL_H
