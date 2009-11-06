#pragma once

class BwAppContext;

enum MessageHandleResult
{
	MHR_DO_NOTHING,
	MHR_EXIT_APP,
	MHR_PREV_SCENE,
	MHR_NEXT_SCENE,
	MHR_RELOAD_SCENE
};
