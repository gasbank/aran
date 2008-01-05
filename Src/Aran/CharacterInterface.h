#pragma once

struct D3DXMATRIX;

class CharacterInterface
{
public:
	CharacterInterface(void);
	~CharacterInterface(void);

	virtual void ChangeTranslation(float dx, float dy, float dz) = 0;		// position
	virtual void ChangeOrientation(float dx, float dy, float dz) = 0;		// rotation
	virtual const D3DXMATRIX* GetFinalTransform() const = 0;
};
