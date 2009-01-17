#pragma once

#include "Renderer.h"

class AranOpenGLRenderer : Renderer
{
public:
	virtual void setProjTransform(ArnMat& m);
	virtual void setViewTransform(ArnMat& m);
	virtual void setWorldTransform(ArnMat& m);

};
