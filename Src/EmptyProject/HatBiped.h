#pragma once

void makeBipedBody(dWorldID world, dSpaceID space);
void renderBiped(IDirect3DDevice9* pd3dDevice);
void makeBipedMesh(IDirect3DDevice9* pd3dDevice);
void releaseBipedMesh();