#pragma once


#ifndef NULL
#define NULL 0
#endif

class Character;

class CharacterAnimationCallback
{
public:
	CharacterAnimationCallback(void);
	~CharacterAnimationCallback(void);

	virtual void DoCallback( void* pData /* in */, void* pResultData /* out */ );
	virtual void DoUnregisterCallback();
	void AttachCharacter( Character* pCharacter );
	Character* GetCharacter();

private:
	Character* pCharacter;
};
