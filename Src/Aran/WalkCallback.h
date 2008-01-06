#pragma once

#include "CharacterAnimationCallback.h"

class WalkCallback
	: public CharacterAnimationCallback
{
public:
	WalkCallback(void);
	~WalkCallback(void);

	virtual void DoCallback( void* pData /* in */, void* pResultData /* out */ );
	virtual void DoUnregisterCallback();


};
