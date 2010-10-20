#include "camera.h"

/*
 * Setup camera
 */

void setup_camera(GLfloat dt)
{
    GLfloat sinE, cosE, sinA, cosA, fScale;
    GLfloat ViewMatrix[16];

	//Compute at-vector
	sinE = (float)sinf(eCam);
	cosE = (float)cosf(eCam);
	sinA = (float)sinf(aCam);
	cosA = (float)cosf(aCam);

	xAt = cosE * sinA;
	yAt = sinE;
	zAt = -cosE * cosA;

	At.x = xAt;
	At.y = yAt;
	At.z = zAt;

	//Compute right vector
	xRt = cosA;
	zRt = sinA;

	Rt.x = xRt;
	Rt.y = 0.0f;
	Rt.z = zRt;

	//Compute up-vector (right X at )
	// i    j   k
	//cosA  0   sinA
	// x    y   z
	xUp = -yAt*sinA;
	yUp = xAt*sinA-zAt*cosA;
	zUp = yAt*cosA;

	//Normalize
	fScale = 1.0f/sqrtf(yAt*yAt+yUp*yUp);

	xUp *= fScale;
	yUp *= fScale;
	zUp *= fScale;

	Up.x = xUp;
	Up.y = yUp;
	Up.z = zUp;

	//Compute camera position
	xCam += ( lCam * xRt + vCam * xUp + dCam * xAt ) * dt;
	yCam += (            + vCam * yUp + dCam * yAt ) * dt;
	zCam += ( lCam * zRt + vCam * zUp + dCam * zAt ) * dt;

	Cam.x = xCam;
	Cam.y = yCam;
	Cam.z = zCam;

	//Set view matrix and transform
	ViewMatrix[0] = xRt;
	ViewMatrix[1] = xUp;
	ViewMatrix[2] = -xAt;
	ViewMatrix[3] = 0.0f;
	ViewMatrix[4] = 0.0f;
	ViewMatrix[5] = yUp;
	ViewMatrix[6] = -yAt;
	ViewMatrix[7] = 0.0f;
	ViewMatrix[8] = zRt;
	ViewMatrix[9] = zUp;
	ViewMatrix[10] = -zAt;
	ViewMatrix[11] = 0.0f;
	ViewMatrix[12] = -( xCam*xRt            + zCam*zRt );
	ViewMatrix[13] = -( xCam*xUp + yCam*yUp + zCam*zUp );
	ViewMatrix[14] =  ( xCam*xAt + yCam*yAt + zCam*zAt );
	ViewMatrix[15] = 1.0f;

	glMultMatrixf(ViewMatrix);
}
