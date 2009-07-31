#pragma once

class ArnTexture;
class ArnTextureGl;
class ArnMesh;
class ArnMeshGl;
class ArnSceneGraph;

ARANGL_API void						ArnDrawAxesGl(float size);
ARANGL_API void						ArnConfigureViewportProjectionMatrixGl(const ArnViewportData* viewportData, const ArnCamera* cam);
ARANGL_API void						ArnConfigureProjectionMatrixGl(const ArnViewportData* viewportData, const ArnCamera* cam);
ARANGL_API void						ArnConfigureLightGl(GLuint lightId, const ArnLight* light);
ARANGL_API void						ArnConfigureViewMatrixGl(ArnCamera* cam);
ARANGL_API GLuint					ArnCreateNormalizationCubeMapGl();
ARANGL_API void						ArnInitializeRenderableObjectsGl(ArnSceneGraph* sg);
ARANGL_API void						ArnSkeletonRenderGl(const ArnSkeleton* skel);
ARANGL_API void						ArnBoneRenderGl(const ArnBone* bone);
ARANGL_API void						ArnSetupMaterialGl(const ArnMaterial* mtrl); // Bind OpenGL with the first texture of this material object

//ARANGL_API void						ArnInitializeMaterialGl(const ArnMaterial* mtrl);
//ARANGL_API void						CreateArnTextureRenderableObjectGl(INOUT ArnTexture* tex);
//ARANGL_API void						CreateArnMeshRenderableObjectGl(INOUT ArnMesh* mesh);
