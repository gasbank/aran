#pragma once
#include "IGame/igameerror.h"

class ErrorProcedure : public IGameErrorCallBack
{
public:
	ErrorProcedure(void);
	~ErrorProcedure(void);

	void ErrorProc(IGameError error);
};
