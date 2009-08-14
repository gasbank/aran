#ifndef VIDEOMANGL_H
#define VIDEOMANGL_H

#include "VideoMan.h"
#include "glInfo.h"

class ARANGL_API VideoManGl : public VideoMan
{
public:
	virtual											~VideoManGl() {}
	static VideoManGl*								create(int argc, char** argv, int width, int height);

	virtual void									setReshapeCallback(void reshape(int, int));
	virtual void									setKeyboardCallback(void keyboardCB(unsigned char, int, int));
	virtual void									setMouseCallback(void mouseCB(int, int, int, int));
	virtual void									clearFrameBuffer();
	virtual void									swapFrameBuffer();
	virtual void									setupProjectionMatrix() const;
	virtual void									setupViewMatrix() const;
	virtual void									setLight(int lightId, const ArnLight* light);
	virtual HRESULT									InitTexture() { ARN_THROW_NOT_IMPLEMENTED_ERROR }
	virtual HRESULT									InitShader() { ARN_THROW_NOT_IMPLEMENTED_ERROR }
	virtual HRESULT									InitAnimationController() { ARN_THROW_NOT_IMPLEMENTED_ERROR }
	virtual HRESULT									InitMainCamera() { ARN_THROW_NOT_IMPLEMENTED_ERROR }
	virtual HRESULT									InitLight() { ARN_THROW_NOT_IMPLEMENTED_ERROR }
	virtual HRESULT									InitMaterial() { ARN_THROW_NOT_IMPLEMENTED_ERROR }
	virtual HRESULT									InitFont() { ARN_THROW_NOT_IMPLEMENTED_ERROR }
	virtual HRESULT									InitModels() { ARN_THROW_NOT_IMPLEMENTED_ERROR }
	virtual HRESULT									InitModelsAtEditor() { ARN_THROW_NOT_IMPLEMENTED_ERROR }
	virtual HRESULT									Show() { ARN_THROW_NOT_IMPLEMENTED_ERROR }
	virtual HRESULT									StartMainLoop();
	virtual void									DrawAtEditor(BOOL isReady, BOOL isRunning) { ARN_THROW_NOT_IMPLEMENTED_ERROR }
	virtual int										Draw()  { ARN_THROW_NOT_IMPLEMENTED_ERROR }
	virtual void									setWorldViewProjection( const ArnMatrix& matWorld, const ArnMatrix& matView, const ArnMatrix& matProj );
    virtual void									renderSingleMesh(ArnMesh* mesh, const ArnMatrix& globalXform = ArnConsts::ARNMAT_IDENTITY) { ARN_THROW_NOT_IMPLEMENTED_ERROR }
	void											setupGlInfo();
	bool											setupVboSupport() const;
private:
													VideoManGl();
	virtual HRESULT									InitLight_Internal() { ARN_THROW_NOT_IMPLEMENTED_ERROR }
	virtual void									setClearColor_Internal() { glClearColor(getClearColor().r, getClearColor().g, getClearColor().b, getClearColor().a); }

	glInfo											m_glInfo;
	bool											m_glInfoValid;
	bool											m_vboSupported;

};

#endif // VIDEOMANGL_H