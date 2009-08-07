/*!
 * @file AranGl.h
 * @author Geoyeob Kim
 * @date 2009
 */
#pragma once

class ArnTexture;
class ArnTextureGl;
class ArnMesh;
class ArnMeshGl;
class ArnSceneGraph;

/*!
 * @brief AranGl 라이브러리 초기화
 */
ARANGL_API void						ArnInitializeGl();
/*!
 * @brief AranGl 라이브러리 해제
 */
ARANGL_API void						ArnCleanupGl();
ARANGL_API void						ArnDrawAxesGl(float size);
ARANGL_API void						ArnConfigureViewportProjectionMatrixGl(const ArnViewportData* viewportData, const ArnCamera* cam);
ARANGL_API void						ArnConfigureProjectionMatrixGl(const ArnViewportData* viewportData, const ArnCamera* cam);
ARANGL_API void						ArnConfigureLightGl(GLuint lightId, const ArnLight* light);
ARANGL_API void						ArnConfigureViewMatrixGl(ArnCamera* cam);
ARANGL_API GLuint					ArnCreateNormalizationCubeMapGl();
ARANGL_API void						ArnInitializeRenderableObjectsGl(ArnSceneGraph* sg);
ARANGL_API void						ArnSetupMaterialGl(const ArnMaterial* mtrl); // Bind OpenGL with the first texture of this material object
ARANGL_API void						ArnSceneGraphRenderGl(const ArnSceneGraph* sg);
ARANGL_API void						ArnRenderSphereGl();
ARANGL_API void						setTransformGl (const double pos[3], const double R[12]);
static void							ArnMeshRenderGl(const ArnMesh* mesh);
static void							ArnSkeletonRenderGl(const ArnSkeleton* skel);
static void							ArnBoneRenderGl(const ArnBone* bone);

//ARANGL_API void						ArnInitializeMaterialGl(const ArnMaterial* mtrl);
//ARANGL_API void						CreateArnTextureRenderableObjectGl(INOUT ArnTexture* tex);
//ARANGL_API void						CreateArnMeshRenderableObjectGl(INOUT ArnMesh* mesh);
