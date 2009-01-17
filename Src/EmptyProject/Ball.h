#pragma once

HRESULT makeBallMesh(IDirect3DDevice9* pd3dDevice);
void renderBall(IDirect3DDevice9* pd3dDevice);
void makeBallBody(dWorldID world, dSpaceID space);
void releaseBallMesh();