#pragma once


void makeArmBody(dWorldID world, dSpaceID space);
void renderArm(IDirect3DDevice9* pd3dDevice);
HRESULT makeArmMesh(IDirect3DDevice9* pd3dDevice);
void releaseArmMesh();
void PcontrolArm();
void KeyboardProcArm( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );