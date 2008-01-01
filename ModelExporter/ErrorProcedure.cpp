// ErrorProcedure.cpp
// 2007 Geoyeob Kim
//
#include "Main.h"
#include "ErrorProcedure.h"

ErrorProcedure::ErrorProcedure(void)
{
}

ErrorProcedure::~ErrorProcedure(void)
{
}

void ErrorProcedure::ErrorProc(IGameError error)
{
	TCHAR* buf = GetLastIGameErrorText();

	if (error == 9)
	{
		// ignore 'no material' error
		return;
	}
	
	DebugPrint(_T("Error! Code = %d, Text = %s\n"), error, buf);

	if (error == 5)
	{
		DebugPrint(_T("  ==> Nothing will be exported.\n"));
	}
}