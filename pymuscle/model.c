/*
 * model.c - skeleton animations handling
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
 */

#include "model.h"

/*
 * Do a fast inversion of a rotation and translation matrix
 *
 */

void inv_mv(GLfloat ret[16], GLfloat mat[16] )
{
  ret[0 ] = mat[0];
  ret[1 ] = mat[4];
  ret[2 ] = mat[8];
  ret[3 ] = 0.0;
  ret[4 ] = mat[1];
  ret[5 ] = mat[5];
  ret[6 ] = mat[9];
  ret[7 ] = 0.0;
  ret[8 ] = mat[2];
  ret[9 ] = mat[6];
  ret[10] = mat[10];
  ret[11] = 0.0;
  ret[12] = -(mat[12] * ret[0] + mat[13] * ret[4] + mat[14] * ret[8]);
  ret[13] = -(mat[12] * ret[1] + mat[13] * ret[5] + mat[14] * ret[9]);
  ret[14] = -(mat[12] * ret[2] + mat[13] * ret[6] + mat[14] * ret[10]);
  ret[15] = 1.0;
}

/*
 * Multiplies two modelview matrices
 *
 */

void mul_mv(GLfloat mv[16], GLfloat mva[16], GLfloat mvb[16]) {

  mv[ 0]=mva[ 0]*mvb[ 0]+mva[ 4]*mvb[ 1]+mva[ 8]*mvb[ 2]+mva[12]*mvb[ 3];
  mv[ 1]=mva[ 1]*mvb[ 0]+mva[ 5]*mvb[ 1]+mva[ 9]*mvb[ 2]+mva[13]*mvb[ 3];
  mv[ 2]=mva[ 2]*mvb[ 0]+mva[ 6]*mvb[ 1]+mva[10]*mvb[ 2]+mva[14]*mvb[ 3];
  mv[ 3]=mva[ 3]*mvb[ 0]+mva[ 7]*mvb[ 1]+mva[11]*mvb[ 2]+mva[15]*mvb[ 3];
  mv[ 4]=mva[ 0]*mvb[ 4]+mva[ 4]*mvb[ 5]+mva[ 8]*mvb[ 6]+mva[12]*mvb[ 7];
  mv[ 5]=mva[ 1]*mvb[ 4]+mva[ 5]*mvb[ 5]+mva[ 9]*mvb[ 6]+mva[13]*mvb[ 7];
  mv[ 6]=mva[ 2]*mvb[ 4]+mva[ 6]*mvb[ 5]+mva[10]*mvb[ 6]+mva[14]*mvb[ 7];
  mv[ 7]=mva[ 3]*mvb[ 4]+mva[ 7]*mvb[ 5]+mva[11]*mvb[ 6]+mva[15]*mvb[ 7];
  mv[ 8]=mva[ 0]*mvb[ 8]+mva[ 4]*mvb[ 9]+mva[ 8]*mvb[10]+mva[12]*mvb[11];
  mv[ 9]=mva[ 1]*mvb[ 8]+mva[ 5]*mvb[ 9]+mva[ 9]*mvb[10]+mva[13]*mvb[11];
  mv[10]=mva[ 2]*mvb[ 8]+mva[ 6]*mvb[ 9]+mva[10]*mvb[10]+mva[14]*mvb[11];
  mv[11]=mva[ 3]*mvb[ 8]+mva[ 7]*mvb[ 9]+mva[11]*mvb[10]+mva[15]*mvb[11];
  mv[12]=mva[ 0]*mvb[12]+mva[ 4]*mvb[13]+mva[ 8]*mvb[14]+mva[12]*mvb[15];
  mv[13]=mva[ 1]*mvb[12]+mva[ 5]*mvb[13]+mva[ 9]*mvb[14]+mva[13]*mvb[15];
  mv[14]=mva[ 2]*mvb[12]+mva[ 6]*mvb[13]+mva[10]*mvb[14]+mva[14]*mvb[15];
  mv[15]=mva[ 3]*mvb[12]+mva[ 7]*mvb[13]+mva[11]*mvb[14]+mva[15]*mvb[15];

}

/*
 * Loads a model from file.
 * Returns a pointer to the model or NULL if there is an error
 *
 */

MODEL_PTR loadModel(const char *filename) {

  FILE *pFile;
  MODEL_PTR model=NULL;
  GLuint i, j, k, num_verts, num_bones, num_tris, num_poses;

  // Attempt to open the file
  if(!(pFile = fopen(filename, "r"))) return NULL;

  //Read number of vertices, bones, triangles and poses
  if(fscanf(pFile, "%d %d %d %d",
	    &num_verts, &num_bones, &num_tris, &num_poses)!=4)
    return NULL;

  // Allocate memory for model
  if(!(model=(MODEL_PTR)malloc(sizeof(MODEL))))
    return NULL;

  //Assign data
  model->num_verts=num_verts;
  model->num_bones=num_bones;
  model->num_tris=num_tris;
  model->num_poses=num_poses+1;
  if(!(model->vertex=(VERTEX_PTR)malloc(sizeof(VERTEX)*num_verts)))
    return NULL;

  //Read vertex data
  for(i=0; i<num_verts; i++) {
    
    //Allocate memory for weights
    if(!(model->vertex[i].w=(GLfloat *)malloc(sizeof(GLfloat)*num_bones)))
      return NULL;

    //Initialise weights
    for(j=0; j<num_bones; j++)
      model->vertex[i].w[j]=0.0f;

    //Read absolute positions and weights
    if(fscanf(pFile, "%f %f %f",
	      &model->vertex[i].pos.x,
	      &model->vertex[i].pos.y, 
	      &model->vertex[i].pos.z)!=3) return NULL;
    if(fscanf(pFile, "%d", &k)!=1) return NULL;
    for(j=0; j<k; j++) {
      GLuint bone;
      GLfloat weight;
      if(fscanf(pFile, "%d %f", &bone, &weight )!=2) return NULL;
      model->vertex[i].w[bone]=weight;
    }
  }

  //Allocate memory for triangles
  if(!(model->tris
       =(GLuint *)malloc(sizeof(GLuint)*num_tris*3)))
    return NULL;

  //Read triangle data
  for(i=0; i<num_tris; i++) 
    if(fscanf(pFile, "%d %d %d",
      &model->tris[3*i], &model->tris[3*i+1], &model->tris[3*i+2])!=3) return NULL;

  //Allocate memory for texture coordinates
  if(!(model->texcoord
       =(VECTOR2D_PTR)malloc(sizeof(VECTOR2D)*num_tris*3)))
    return NULL;

  //Read texture coordinates data
  for(i=0; i<num_tris; i++) {
    if(fscanf(pFile, "%f %f",
      &model->texcoord[3*i].u, &model->texcoord[3*i].v)!=2) return NULL;
    if(fscanf(pFile, "%f %f",
      &model->texcoord[3*i+1].u, &model->texcoord[3*i+1].v)!=2) return NULL;
    if(fscanf(pFile, "%f %f",
      &model->texcoord[3*i+2].u, &model->texcoord[3*i+2].v)!=2) return NULL;
  }

  //Allocate memory for bone data
  if(!(model->bone
       =(BONE_PTR)malloc(sizeof(BONE)*num_bones)))
    return NULL;

  //Read bones data
  for(i=0; i<num_bones; i++) {
    if(fscanf(pFile, "%d", &j)!=1) return NULL;
    k=j-1;
    if(fscanf(pFile, "%d", &model->bone[k].parent)!=1) return NULL;
    model->bone[k].parent-=1;
    if(fscanf(pFile, "%f %f %f %f",
	      &model->bone[k].mva[ 0],
	      &model->bone[k].mva[ 1],
	      &model->bone[k].mva[ 2],
	      &model->bone[k].mva[ 3])!=4)
      return NULL;
    if(fscanf(pFile, "%f %f %f %f",
	      &model->bone[k].mva[ 4],
	      &model->bone[k].mva[ 5],
	      &model->bone[k].mva[ 6],
	      &model->bone[k].mva[ 7])!=4)
      return NULL;
    if(fscanf(pFile, "%f %f %f %f",
	      &model->bone[k].mva[ 8],
	      &model->bone[k].mva[ 9],
	      &model->bone[k].mva[10],
	      &model->bone[k].mva[11])!=4)
      return NULL;
    if(fscanf(pFile, "%f %f %f %f",
	      &model->bone[k].mva[12],
	      &model->bone[k].mva[13],
	      &model->bone[k].mva[14],
	      &model->bone[k].mva[15])!=4)
      return NULL;
  }

  //Compute relative transformation matrices
  for(i=0; i<num_bones; i++) {
    if(model->bone[i].parent!=-1) {
      GLfloat mv[16];
      inv_mv(mv, model->bone[model->bone[i].parent].mva);
      mul_mv(model->bone[i].mvr, mv, model->bone[i].mva);
    }
    else
      for(j=0; j<MV_SIZE; j++)
	model->bone[i].mvr[j]=model->bone[i].mva[j];
  }

  //Allocate poses
  model->pose=(POSE_PTR)malloc(sizeof(POSE)*model->num_poses);

  //The first pose is the standard one we start with
  model->pose[0].Q=
    (VECTOR4D_PTR)malloc(sizeof(VECTOR4D)*model->num_bones);
  model->pose[0].t=2.0f;
  for(i=0; i<model->num_bones; i++) {
    mv2quat(&model->bone[i].Q, model->bone[i].mvr);
    model->pose[0].Q[i].x=model->bone[i].Q.x;
    model->pose[0].Q[i].y=model->bone[i].Q.y;
    model->pose[0].Q[i].z=model->bone[i].Q.z;
    model->pose[0].Q[i].w=model->bone[i].Q.w;
  }
 
  //Lets check if there are other poses to load
  for(i=1; i<model->num_poses; i++) {

    model->pose[i].Q=
      (VECTOR4D_PTR)malloc(sizeof(VECTOR4D)*model->num_bones);

    if(fscanf(pFile, "%f", &model->pose[i].t)!=1)
      return NULL;

    for(j=0; j<model->num_bones; j++) {
      if(fscanf(pFile, "%f %f %f %f",
		&model->pose[i].Q[j].x,
		&model->pose[i].Q[j].y,
		&model->pose[i].Q[j].z,
		&model->pose[i].Q[j].w)!=4)
	return NULL;
    }
  }

  fclose(pFile);

  //Compute relative distances
  for(i=0; i<num_verts; i++) {
    if(!(model->vertex[i].dis
	 =(VECTOR3D_PTR)malloc(sizeof(VECTOR3D)*num_bones))) return NULL;

    for(j=0; j<num_bones; j++) {
      model->vertex[i].dis[j].x= 
	model->vertex[i].pos.x*model->bone[j].mva[0]+
	model->vertex[i].pos.y*model->bone[j].mva[1]+
	model->vertex[i].pos.z*model->bone[j].mva[2]-
	model->bone[j].mva[12];
      model->vertex[i].dis[j].y= 
	model->vertex[i].pos.x*model->bone[j].mva[4]+
	model->vertex[i].pos.y*model->bone[j].mva[5]+
	model->vertex[i].pos.z*model->bone[j].mva[6]-
	model->bone[j].mva[13];
      model->vertex[i].dis[j].z= 
	model->vertex[i].pos.x*model->bone[j].mva[8]+
	model->vertex[i].pos.y*model->bone[j].mva[9]+
	model->vertex[i].pos.z*model->bone[j].mva[10]-
	model->bone[j].mva[14];
    }
  }

  //Construct hierarchy
  model->hierarchy=(GLuint *)malloc(sizeof(GLuint)*model->num_bones);
  j=0;
  k=0;
  while(j<model->num_bones) {
    for(i=0; i<model->num_bones; i++)
      if(model->bone[i].parent==(k-1))
	model->hierarchy[j++]=i;
    k++;
  }

  return model;
}

/*
 * Deletes a model
 *
 */
 
void deleteModel(MODEL_PTR model) {

  GLuint i;

  for(i=0; i<model->num_verts; i++) {
    SAFE_DELETE_ARRAY(model->vertex[i].dis);
    SAFE_DELETE_ARRAY(model->vertex[i].w);
  }
  SAFE_DELETE_ARRAY(model->vertex);
  for(i=0; i<model->num_poses; i++) {
    SAFE_DELETE_ARRAY(model->pose[i].Q);
  }
  SAFE_DELETE_ARRAY(model->pose);
  SAFE_DELETE_ARRAY(model->texcoord);
  SAFE_DELETE_ARRAY(model->bone);
  SAFE_DELETE_ARRAY(model->tris);
  SAFE_DELETE_ARRAY(model->hierarchy);
  SAFE_DELETE_ARRAY(model);

  return;
}

/*
 * Updates a model
 *
 */

void updateModel(MODEL_PTR model) {

  GLuint i, j;
  GLfloat tmpx, tmpy, tmpz;

  //First update the modelview matrices, we start with the root alone
  for(i=0; i<MV_SIZE; i++)
    model->bone[model->hierarchy[0]].mva[i]=
      model->bone[model->hierarchy[0]].mvr[i];

  //Then continue for all the other bones
  for(i=1; i<model->num_bones; i++)
    mul_mv(model->bone[model->hierarchy[i]].mva,
	   model->bone[model->bone[model->hierarchy[i]].parent].mva,
	   model->bone[model->hierarchy[i]].mvr);

  //Now lets update the vertices position
  for(i=0; i<model->num_verts; i++) {

    //Reset accumulators
    tmpx=0.0f;
    tmpy=0.0f;
    tmpz=0.0f;

    for(j=0; j<model->num_bones; j++)
      if(model->vertex[i].w[j]) {
	tmpx+=model->vertex[i].w[j]*(
	  model->vertex[i].dis[j].x*model->bone[j].mva[0]+
	  model->vertex[i].dis[j].y*model->bone[j].mva[4]+
	  model->vertex[i].dis[j].z*model->bone[j].mva[8]+
	  model->bone[j].mva[12]);
	tmpy+=model->vertex[i].w[j]*(
	  model->vertex[i].dis[j].x*model->bone[j].mva[1]+
	  model->vertex[i].dis[j].y*model->bone[j].mva[5]+
	  model->vertex[i].dis[j].z*model->bone[j].mva[9]+
	  model->bone[j].mva[13]);
	tmpz+=model->vertex[i].w[j]*(
	  model->vertex[i].dis[j].x*model->bone[j].mva[2]+
	  model->vertex[i].dis[j].y*model->bone[j].mva[6]+
	  model->vertex[i].dis[j].z*model->bone[j].mva[10]+
	  model->bone[j].mva[14]);
      }

    //Modify only if vertex is affected
    if(tmpx)
      model->vertex[i].pos.x=tmpx;
    if(tmpy)
      model->vertex[i].pos.y=tmpy;
    if(tmpz)
      model->vertex[i].pos.z=tmpz;
  }

  return;
}
