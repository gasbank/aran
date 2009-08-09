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
 * @remark 반드시 OpenGL 컨텍스트가 생성된 후에 호출해야 합니다.
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
ARANGL_API void						ArnRenderBoundingBox(const boost::array<ArnVec3, 8>& bb);
static void							ArnMeshRenderGl(const ArnMesh* mesh);
static void							ArnSkeletonRenderGl(const ArnSkeleton* skel);
static void							ArnBoneRenderGl(const ArnBone* bone);
