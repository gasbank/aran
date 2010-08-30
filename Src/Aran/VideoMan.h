// VideoMan.h
// 2007 Geoyeob Kim
// 2009 Geoyeob Kim
//
// Aran Project Video Manager
//

#pragma once

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) if((p)!=0) { (p)->Release(); (p) = 0; }
#endif

class Character;
class ArnCamera;
class ArnNode;
class ArnMesh;
class ArnAnimationController;
class ArnIpo;
class ArnSkinInfo;
class ArnSceneGraph;
class ArnLight;
class ArnTimer;
class ArnMatrix;
class ArnCamera;
class ArnGenericBuffer;
struct ArnViewportData;

static const DWORD ARNSHADER_DEBUG = 1;

class RenderLayer;

// General callbacks
typedef void    (*LPARNCALLBACKFRAMEMOVE)( double fTime, float fElapsedTime);
typedef void	(*ARNUPDATEFRAME)(double dTime, float fElapsedTime);
typedef void	(*ARNRENDERFRAME)();

class ARAN_API VideoMan : public Singleton<VideoMan>
{

public:
	virtual											~VideoMan();
	//static VideoMan*								create(RendererType type, int width, int height, int argc, char** argv);
	void											setUpdateFrameCallback(ARNUPDATEFRAME cb) { m_updateFrameCb = cb; }
	virtual void									setRenderFrameCallback(ARNRENDERFRAME cb) { m_renderFrameCb = cb; }
	virtual void									setReshapeCallback(void reshape(int, int)) = 0;
	virtual void									setKeyboardCallback(void keyboardCB(unsigned char, int, int)) = 0;
	virtual void									setMouseCallback(void mouseCB(int, int, int, int)) = 0;
	virtual void									clearFrameBuffer() = 0;
	virtual void									swapFrameBuffer() = 0;
	virtual void									setupViewMatrix() const = 0;
	virtual void									setupProjectionMatrix() const = 0;
	HRESULT											InitLight();
	virtual void									setLight(int lightId, const ArnLight* light) = 0;
	void											setupLightsFromSceneGraph();
	void											disableAllLights();
	virtual HRESULT									InitTexture() = 0;
	virtual HRESULT									InitShader() = 0;
	virtual HRESULT									InitAnimationController() = 0; // Global animation controller
	virtual HRESULT									InitMainCamera() = 0;
	virtual HRESULT									InitMaterial() = 0;
	virtual HRESULT									InitFont() = 0;
	virtual HRESULT									InitModels() = 0;
	virtual HRESULT									InitModelsAtEditor() = 0;
	virtual HRESULT									Show() = 0;
	virtual HRESULT									StartMainLoop() = 0;
	virtual void									DrawAtEditor(BOOL isReady, BOOL isRunning) = 0;
	virtual void									setWorldViewProjection( const ArnMatrix& matWorld, const ArnMatrix& matView, const ArnMatrix& matProj ) = 0;
	virtual void									renderSingleMesh(ArnMesh* mesh, const ArnMatrix& globalXform = ArnConsts::ARNMAT_IDENTITY) = 0;
	virtual void									renderSceneGraph();
	void											updateSceneGraph(double dTime, float fElapsedTime);
	const ArnMatrix*								getArcballResult() const { return &modelArcBallRotation; }
	ArnMaterialData*								getDefaultMaterial() { return &defaultMaterial; }
	BOOL											isRendering() const { return m_bRendering; }
	void											setAllowRenderNewFrame(bool val) { m_bAllowRenderNewFrame = val; }
	BOOL											isCloseNow() const { return closeNow; }
	void											setCloseNow(BOOL val) { closeNow = val; }
	BOOL											isOkayToDestruct() const { return okayToDestruct; }
	void											setOkayToDestruct(BOOL val) { okayToDestruct = val; }
	bool											PauseMainLoop();
	bool											ResumeMainLoop();
	void											Close();
	void											MoveMainCameraEye(float dx, float dy, float dz);
	void											ToggleLeftPattern();
	void											ToggleRightPattern();
	HRESULT											SetCamera(float x, float y, float z);
	HRESULT											SetCamera( ARN_NDD_CAMERA_CHUNK* pCamChunk );
	HRESULT											SetCamera(ArnCamera& arnCam);
	const ARN_CAMERA&								getCamera() const { return mainCamera; }
	void											SetWindowSize(int w, int h);
	int												GetWindowSizeW() const { return screenWidth; }
	int												GetWindowSizeH() const { return screenHeight; }
	void											SetHwnd(HWND hWnd);
	void											attachSceneGraph(ArnSceneGraph* sceneGraph);
	ArnSceneGraph*									detachSceneGraph();
	HRESULT											SetCurrentFrameIndex(int idx);
	virtual void									ScrollBy( ArnVec3* dScroll );
	size_t											registerRenderLayer(RenderLayer* pRL);
	BOOL											unregisterRenderLayerAt( unsigned int ui );
	size_t											unregisterAllRenderLayers();
	RenderLayer*									getRenderLayerAt(unsigned int ui);
	const ARN_CAMERA*								getMainCamera() const { return &mainCamera; }
    void                                            getScreenInfo(int& width, int& height) const { width = screenWidth; height = screenHeight; }
	const ArnMatrix*								getModelArcBallRotation() const { return &modelArcBallRotation; }
	void											renderMeshesOnly(ArnNode* node, const ArnMatrix& globalXform = ArnConsts::ARNMAT_IDENTITY);
	float											getFPS() const { return m_fFPS; }
	void											setFrameMoveCallback(LPARNCALLBACKFRAMEMOVE pCallback);
	void											setScreenSize(int width, int height) { screenWidth = width; screenHeight = height; }
	int												getScreenWidth() const { return screenWidth; }
	int												getScreenHeight() const { return screenHeight; }
	std::list<RenderLayer*>&						getRenderLayers() { return m_renderLayers; }
	double											getTime() const { return m_fTime; }
	float											getElapsedTime() const { return m_fElapsedTime; }
	void											resetModelArcBallRotation();
	void											setClearColor(const ArnColorValue4f& clearColor) { m_clearColor = clearColor; setClearColor_Internal(); }
	void											updateTime(); // Update the current time using internal timer.
	double											getLastRefreshedTime() const { return m_dLastRefreshedTime; }
	ArnSceneGraph*									getSceneGraph() { return m_sceneGraph; }
	void											updateFrame(double dTime, float fElapsedTime);
	void											renderFrame();
	bool											setRenderStarted();
	void											setRenderFinished();

protected:
													VideoMan();
	int												Draw();
	void											setModelArcBallRotation(const ArnMatrix& mat) { modelArcBallRotation = mat; }
	const ArnMatrix*								getModelArcBallRotationLast() const { return &modelArcBallRotationLast; }
	void											updateModelArcBallRotation() { modelArcBallRotationLast = modelArcBallRotation; }
	const ArnLightData&								getDefaultLight() const { return defaultLight; }
	void											resetWorldMatrix();
	const ArnMatrix&								getWorldMatrix() const { return matWorld; }
	const ArnMatrix&								getViewMatrix() const { return matView; }
	void											setViewMatrix(const ArnMatrix& view) { matView = view; }
	void											setWorldMatrix(const ArnMatrix& world) { matWorld = world; }
	void											setProjectionMatrix(const ArnMatrix& proj) { matProjection = proj; }
	const ArnMatrix&								getProjectionMatrix() const { return matProjection; }
	int												getTotalLightCount() const { return totalLightCount; }
	void											setTotalLightCount(int count) { totalLightCount = count; }
	const ArnMaterialData&							getDefaultMaterial() const { return defaultMaterial; }
	const ArnColorValue4f&							getClearColor() const { return m_clearColor; }

private:
	virtual HRESULT									InitLight_Internal() = 0;
	virtual void									setClearColor_Internal() = 0;
	DWORD											shaderFlags;
	ArnVec3											cameraVector;
	BOOL											cameraBounceDirection;
	BOOL											leftPattern;
	BOOL											rightPattern;
	bool											m_bRendering; // Indicates rendering is in its way...
	bool											m_bAllowRenderNewFrame; // Allow new start of rendering process
	BOOL											closeNow;
	ArnVec3											dungeonTranslation;
	DWORD											drawCount; // count for Draw() function call
	DWORD											curFrameIndex; // current animation keyframe index (editor)
	BOOL											okayToDestruct; // resolves multithread issue
	ARN_CAMERA										mainCamera;
	ArnLightData									defaultLight;
	ArnLightData									pointLight;
	ArnMatrix										matWorld;
	ArnMatrix										matView;
	ArnMatrix										matProjection;
	int												screenWidth;
	int												screenHeight;
	int												totalLightCount;
	ArnMaterialData									defaultMaterial;
	ArnMaterialData									rgbMaterial[3]; // red, green, blue with no texture
	std::list<RenderLayer*>							m_renderLayers;
	ArnMatrix										modelArcBallRotation;
	ArnMatrix										modelArcBallRotationLast;
	double											m_fTime;
	float											m_fElapsedTime;
	double											m_prevLastTime;
	double											m_dLastRefreshedTime;
	DWORD											m_prevUpdateFrames;
	float											m_fFPS;
	ArnTimer*										m_preciseTimer;
	ArnSceneGraph*									m_sceneGraph;
	ArnColorValue4f									m_clearColor;
	ARNUPDATEFRAME									m_updateFrameCb;
	ARNRENDERFRAME									m_renderFrameCb;
};

inline VideoMan& GetVideoManager() { return VideoMan::getSingleton(); }
// Template approach is also possible.
//template<typename T> T& ArnGet() { return T::getSingleton(); }

//////////////////////////////////////////////////////////////////////////

ARAN_API ArnMatrix* ArnGetProjectionMatrix(ArnMatrix* out, const ArnViewportData* viewportData, const ArnCamera* cam, bool rightHanded);
ARAN_API HRESULT	ArnIntersectGl( ArnMesh* pMesh, const ArnVec3* pRayPos, const ArnVec3* pRayDir, bool* pHit, unsigned int* pFaceIndex, FLOAT* pU, FLOAT* pV, FLOAT* pDist, ArnGenericBuffer* ppAllHits, unsigned int* pCountOfHits );
