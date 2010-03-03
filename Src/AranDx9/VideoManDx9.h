#ifndef VIDEOMANDX9_H
#define VIDEOMANDX9_H

// VideoManDx9: VideoMan subclass for DirectX 9 (D3D9) library
// Only compiled in Win32 environment
#ifdef WIN32

#include "VideoMan.h"

class ModelReader;
class InputMan;

class ARANDX9_API VideoManDx9 : public VideoMan
{
public:
	virtual											~VideoManDx9() {}
	static VideoManDx9*								create() { return new VideoManDx9(); }
	virtual void									setReshapeCallback(void reshape(int, int));
	virtual void									setKeyboardCallback(void keyboardCB(unsigned char, int, int));
	virtual void									setMouseCallback( void mouseCB(int, int, int, int) );
	virtual void									setLight(int lightId, const ArnLight* light);
	virtual void									clearFrameBuffer();
	virtual void									swapFrameBuffer();
	virtual void									setupProjectionMatrix() const;
	virtual void									setupViewMatrix() const;
	HRESULT											RenderModel(ModelReader* pMR, const ArnMatrix* worldTransformMatrix = 0);
	HRESULT											RenderModel1(ModelReader* pMR, const ArnMatrix* worldTransformMatrix = 0) const;
	HRESULT											RenderModel2(ModelReader *pMR, const ArnMatrix* worldTransformMatrix = 0);
	virtual void									setWorldViewProjection( const ArnMatrix& matWorld, const ArnMatrix& matView, const ArnMatrix& matProj );
	HRESULT											InitWindow(TCHAR* szClassName, void* msgProc, int width, int height);
	void											SetDrawingModelAtEditor(ModelReader* pMR);
	void											setClassName(TCHAR* szClassName);
	const TCHAR*									getClassName() const { return szClassName; }
	virtual LPDIRECT3DDEVICE9						GetDev() const { return lpD3DDevice; }
	virtual void									SetDev(LPDIRECT3DDEVICE9 dev) { lpD3DDevice = dev; }
	void											AttachInputMan(InputMan* inputMan);
	InputMan*										GetInputMan();
private:
													VideoManDx9();
	HRESULT											InitD3D(BOOL isMultithreaded = FALSE);
	HRESULT											InitTestVertexBuffer();
	HRESULT											InitBoxVertexBuffer();
	HRESULT											InitSkinnedMeshVertexBuffer();
	HRESULT											InitCustomMesh();
	void											ChangeInTestVB(D3DCOLOR color);
	virtual HRESULT									InitTexture();
	virtual HRESULT									InitShader();
	virtual HRESULT									InitAnimationController(); // Global animation controller
	virtual HRESULT									InitMainCamera();
	virtual HRESULT									InitLight();
	virtual HRESULT									InitMaterial();
	virtual HRESULT									InitFont();
	virtual HRESULT									InitModels();
	virtual HRESULT									InitModelsAtEditor();
	virtual HRESULT									Show();
	virtual HRESULT									StartMainLoop();
	virtual void									DrawAtEditor(BOOL isReady, BOOL isRunning);
	virtual int										Draw();
	virtual void									renderSingleMesh(ArnMesh* mesh, const ArnMatrix& globalXform = ArnConsts::ARNMAT_IDENTITY);
	virtual HRESULT									InitLight_Internal();
	HRESULT											TurnModelLightOn(ModelReader *pMR, ArnMatrix* worldTransformMatrix = 0);
	virtual void									setClearColor_Internal();

	InputMan*										pInputMan;
	ModelReader*									pDrawingModel; // indicates drawing model at editor
	ModelReader*									mr1;
	ModelReader*									mr2;
	ModelReader*									mr3;
	ModelReader*									mrHouse;
	ModelReader*									mrMan;
	ModelReader*									gamebryo;
	ModelReader*									middlesnake;
	ModelReader*									highpoly;
	ModelReader*									mrRocky;
	ModelReader*									mrDungeon;		// Dungeon(level) test
	std::vector<float>								testFloatArray;
	TCHAR											szClassName[64];
	WNDCLASS										wndClass;
	HWND											hWnd;
	HWND											hLoadingWnd;
	LPDIRECT3D9										lpD3D;
	LPDIRECT3DDEVICE9								lpD3DDevice;
	LPDIRECT3DTEXTURE9								lpTex1;
	LPD3DXFONT										lpFont;
	LPD3DXBUFFER									lpCompiledFragments;
	LPDIRECT3DVERTEXSHADER9							lpVertexShader;
	LPD3DXCONSTANTTABLE								lpConstantTable;
	LPD3DXEFFECT									lpEffect;
	LPD3DXEFFECT									lpEffectSkinning;
	LPD3DXMESH										lpCustomMesh;
	LPD3DXMESH										lpCustomSkinnedMesh;
	ArnSkinInfo*									lpSkinInfo;
	D3DXMESHCONTAINER								meshContainer;
	D3DXFRAME										frame1;
	D3DXFRAME										frame2; // bones
	ArnAnimationController*							lpAnimationController;
	ArnIpo*								lpDefaultAnimationSet;
	LPDIRECT3DVERTEXBUFFER9							lpTestVB;
	LPDIRECT3DVERTEXBUFFER9							lpTestVB2;
	LPDIRECT3DVERTEXBUFFER9							lpBoxVB;
	LPDIRECT3DVERTEXBUFFER9							lpSkinnedVB;
	void*											pVBVertices;
	static const DWORD								MY_COLOR_VERTEX_FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
};

inline VideoManDx9& GetVideoManagerDx9 () { return reinterpret_cast<VideoManDx9 &> (VideoMan::getSingleton ()); }

//////////////////////////////////////////////////////////////////////////

ARANDX9_API HRESULT
ArnIntersectDx9(
				LPD3DXMESH pMesh,
				const ArnVec3* pRayPos,
				const ArnVec3* pRayDir, 
				bool* pHit,              // True if any faces were intersected
				DWORD* pFaceIndex,        // index of closest face intersected
				FLOAT* pU,                // Barycentric Hit Coordinates    
				FLOAT* pV,                // Barycentric Hit Coordinates
				FLOAT* pDist,             // Ray-Intersection Parameter Distance
				ArnGenericBuffer* ppAllHits,    // Array of D3DXINTERSECTINFOs for all hits (not just closest) 
				DWORD* pCountOfHits);     // Number of entries in AllHits array


#endif

#endif // VIDEOMANDX9_H
