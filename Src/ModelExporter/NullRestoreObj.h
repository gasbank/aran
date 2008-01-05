#pragma once

#include "hold.h"

class NullRestoreObj : public RestoreObj
{
public:
	NullRestoreObj(void);
	~NullRestoreObj(void);

	virtual void Restore(int) {}
	virtual void Redo() {}
	virtual int Size() { return 10; }
};
