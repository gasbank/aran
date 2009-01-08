#include "DXUT.h"
#include <ode/ode.h>


typedef struct {       // MyObject構造体
	dBodyID body;        // ボディ(剛体)のID番号（動力学計算用）
	dGeomID geom;        // ジオメトリのID番号(衝突検出計算用）
	dJointID joint;
	double l; // Length [m]
	double r; // Radius [m]
	double m; // Mass [kg]

	double x, y, z; // Position
	LPD3DXMESH mesh;
} MyObject;
