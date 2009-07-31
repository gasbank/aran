#pragma once

//
// Data structure for Direct3D compatibility. (D3DVIEWPORT9)
//
struct ArnViewportData {
	DWORD					X;
	DWORD					Y;            /* Viewport Top left */
	DWORD					Width;
	DWORD					Height;       /* Viewport Dimensions */
	float					MinZ;         /* Min/max of clip Volume */
	float					MaxZ;
};
