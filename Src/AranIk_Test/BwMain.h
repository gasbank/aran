#pragma once

class BwAppContext;

struct HitRecord
{
	GLuint numNames;	// Number of names in the name stack for this hit record
	GLuint minDepth;	// Minimum depth value of primitives (range 0 to 2^32-1)
	GLuint maxDepth;	// Maximum depth value of primitives (range 0 to 2^32-1)
	GLuint contents;	// Name stack contents
};

enum MessageHandleResult
{
	MHR_DO_NOTHING,
	MHR_EXIT_APP,
	MHR_PREV_SCENE,
	MHR_NEXT_SCENE,
	MHR_RELOAD_SCENE
};

void RenderScene(const BwAppContext& ac);