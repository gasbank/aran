// InputMan.h
// 2007 Geoyeob Kim

#pragma once

typedef int BOOL;

#include <d3dx9.h>

class CharacterInterface;

struct Point2Int 
{
	int x, y;
};

class InputMan
{
private:
	BOOL isClicked;
	BOOL isRClicked;
	BOOL isDragging;
	Point2Int mouseCurPos;
	Point2Int mouseDownPos;
	Point2Int mouseUpPos;

	CharacterInterface* charInterface;
	
public:
	InputMan(void);
	~InputMan(void);

	void SetClicked(BOOL b) { this->isClicked = b; }
	void SetRClicked(BOOL b) { this->isRClicked = b; }
	void SetDragging(BOOL b) { this->isDragging = b; }

	void SetMouseCurPos(Point2Int p) { this->mouseCurPos = p; }
	void SetMouseCurPos(int x, int y) { this->mouseCurPos.x = x; this->mouseCurPos.y = y; }
	void SetMouseDownPos(Point2Int p) { this->mouseDownPos = p; }
	void SetMouseDownPos(int x, int y) { this->mouseDownPos.x = x; this->mouseDownPos.y = y; }
	void SetMouseUpPos(Point2Int p) { this->mouseUpPos = p; }
	void SetMouseUpPos(int x, int y) { this->mouseUpPos.x = x; this->mouseUpPos.y = y; }
	

	BOOL IsClicked() { return this->isClicked; }
	BOOL IsRClicked() { return this->isRClicked; }
	BOOL IsDragging() { return this->isDragging; }

	Point2Int GetMouseCurPos() { return this->mouseCurPos; }
	Point2Int GetMouseDownPos() { return this->mouseDownPos; }
	Point2Int GetMouseUpPos() { return this->mouseUpPos; }


	// Player character
	void AttachCharacterInterface( CharacterInterface* charInterface ) { this->charInterface = charInterface; }

	// Player character control
	void WalkCharacterForward( float amount = 1.0f );
	void WalkCharacterBackward( float amount = 1.0f );
	void TurnCharacterLeft( float amount = D3DXToRadian( 10 ) );
	void TurnCharacterRight( float amount = D3DXToRadian( 10 ) );
	const D3DXMATRIX* GetFinalTransform() const;
};
