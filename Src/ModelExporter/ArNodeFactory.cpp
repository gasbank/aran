#include "stdafx.h"
#include "ArNodeFactory.h"
#include "ArMesh.h"
#include "ArCamera.h"
#include "ArLight.h"

ArNodeFactory::ArNodeFactory(void)
{
}

ArNodeFactory::~ArNodeFactory(void)
{
}

ArNode* ArNodeFactory::create( ArNode::Type type, const char* name )
{
	switch (type) {
		case ArNode::eArMesh:
			return new ArMesh(name);
			break;
		case ArNode::eArCamera:
			return new ArCamera(name);
			break;
		case ArNode::eArLight:
			return new ArLight(name);
			break;
	}

	return NULL;
}