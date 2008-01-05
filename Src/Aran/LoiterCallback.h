#pragma once
#include "characteranimationcallback.h"

class LoiterCallback :
	public CharacterAnimationCallback
{
public:
	LoiterCallback(void);
	~LoiterCallback(void);

	virtual void DoCallback( void* pData /* in */, void* pResultData /* out */ );
	virtual void DoUnregisterCallback();
};
