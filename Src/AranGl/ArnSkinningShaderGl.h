#ifndef ARNSKINNINGSHADERGL_H
#define ARNSKINNINGSHADERGL_H

class ArnSkeleton;
class ArnMesh;

class ARANGL_API ArnSkinningShaderGl : public Singleton<ArnSkinningShaderGl>
{
public:
						ArnSkinningShaderGl();
						~ArnSkinningShaderGl();
	void				initShader();
	void				setShaderConstants(const ArnSkeleton* skel, const std::vector<VertexGroup>& vertGroup, const ArnMesh* skinnedMesh);
	GLhandleARB			getProgramObj() const { return m_programObj; }
	GLuint				getWeightsAttrId() const { return m_location_weights; }
	GLuint				getMatrixIndicesAttrId() const { return m_location_matrixIndices; }
	GLuint				getNumBonesAttrId() const { return m_location_numBones; }
protected:
private:
	GLhandleARB			m_programObj;
	GLhandleARB			m_vertexShader;
	GLuint				m_location_boneMatrices[32];
	GLuint				m_location_eyePosition;
	GLuint				m_location_lightVector;
	GLuint				m_location_inverseModelView;

	GLuint				m_location_weights;
	GLuint				m_location_matrixIndices;
	GLuint				m_location_numBones;
};

#endif // ARNSKINNINGSHADERGL_H
