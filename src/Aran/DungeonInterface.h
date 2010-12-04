#pragma once

class ArnVec3;

class DungeonInterface
{
public:
	DungeonInterface(void);
	~DungeonInterface(void);

	virtual void ScrollBy( const ArnVec3* dScroll ) = 0;
};
