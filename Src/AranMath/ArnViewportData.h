#pragma once

//
// Data structure for Direct3D compatibility. (D3DVIEWPORT9)
//
struct ArnViewportData {
#ifdef WIN32
	const D3DVIEWPORT9*		getConstDxPtr() const { return reinterpret_cast<const D3DVIEWPORT9*>(this); }
	D3DVIEWPORT9*			getDxPtr() { return reinterpret_cast<D3DVIEWPORT9*>(this); }
#endif

	DWORD					X;
	DWORD					Y;            /* Viewport Top left */
	DWORD					Width;
	DWORD					Height;       /* Viewport Dimensions */
	float					MinZ;         /* Min/max of clip Volume */
	float					MaxZ;
};
