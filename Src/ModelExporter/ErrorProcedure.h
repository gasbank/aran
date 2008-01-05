#pragma once
#include <tchar.h>
#include "igameerror.h"

class ErrorProcedure : public IGameErrorCallBack
{
public:
	ErrorProcedure(void);
	~ErrorProcedure(void);

	void ErrorProc(IGameError error);
};
