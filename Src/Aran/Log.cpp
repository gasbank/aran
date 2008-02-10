// Log.cpp
// 2006 Geoyeob, Kim (First Revision: MapModel Project)
// 2008 Geoyoeb, Kim (Second Revision: Aran Project)


#include "stdafx.h"

#include "Log.h"

IMPLEMENT_SINGLETON(LOGMANAGER);


LOGMANAGER::LOGMANAGER()
{
	okayCount = 0;
	failCount = 0;

	fout.open("d3dlog.txt");
	if (!fout)
	{
		std::cout << _T( "Log file failed" ) << std::endl;
	}
	else
	{
		NewLog( _T( "Start log file" ) );
	}
}

LOGMANAGER::~LOGMANAGER()
{
	if (fout)
	{
		NewLog( _T( "Close log file" ) );
		fout << _T( "CNTS : " ) << okayCount << _T( " Okays, " ) << failCount << _T( " Fails" ) << std::endl;
		fout.close();
	}
}

LOGMANAGER::LOGMANAGER(char *filename, bool bAppend)
{
	okayCount = 0;
	failCount = 0;

	fout.open(filename, bAppend ? std::ios_base::app : std::ios_base::out);
	if (!fout)
	{
		std::cout << _T( "Log file failed" ) << std::endl;
	}
	else
	{
		if (bAppend) NewLog( _T( "Start log file (Append)" ) );
		else NewLog( _T( "Start log file" ) );
	}
}

LOGMANAGER::LOGMANAGER(char *filename)
{
	LOGMANAGER(filename, false);
}

void LOGMANAGER::NewLog(TCHAR* message)
{
	NewLog(message, LOG_OKAY);
}

void LOGMANAGER::NewLog(TCHAR* message, bool bOkay)
{
	//fout << "-----------------------------------------------------------------------------------------------------------\n";
	fout << "***********\n";
	if (!bOkay)
	{
		fout << _T( "MSG  : [FAILURE] " ) << message << std::endl;
		failCount++;
	}
	else
	{
		fout << _T( "MSG  : [OKAY] " ) << message << std::endl;
		okayCount++;
	}
}

void LOGMANAGER::NewLog(char* sourcefilename, char* funcname, int line, TCHAR* message, bool bOkay)
{
	NewLog(message, bOkay);
	
	fout << _T( "FILE : " ) << sourcefilename << " (" << line << ")" << std::endl;
	fout << _T( "FUNC : " ) << funcname << std::endl;
}


