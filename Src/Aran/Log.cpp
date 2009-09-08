// Log.cpp
// 2006 Geoyeob, Kim (First Revision: MapModel Project)
// 2008 Geoyoeb, Kim (Second Revision: Aran Project)


#include "AranPCH.h"

#include "Log.h"

IMPLEMENT_SINGLETON(LOGMANAGER)


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
		NewLog("Start log file");
	}
}

LOGMANAGER::~LOGMANAGER()
{
	if (fout)
	{
		NewLog("Close log file");
		fout << _T( "CNTS : " ) << okayCount << _T( " Okays, " ) << failCount << _T( " Fails" ) << std::endl;
		fout.close();
	}
}

LOGMANAGER::LOGMANAGER(const char *filename, bool bAppend)
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
		if (bAppend) NewLog("Start log file (Append)");
		else NewLog("Start log file");
	}
}

LOGMANAGER::LOGMANAGER(const char *filename)
{
	LOGMANAGER(filename, false);
}

void LOGMANAGER::NewLog(const char* message, bool bOkay)
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

void LOGMANAGER::NewLog(const wchar_t* message, bool bOkay)
{
	// TODO
}

void LOGMANAGER::NewLog(const char* sourcefilename, const char* funcname, int line, const char* message, bool bOkay)
{
	NewLog(message, bOkay);

	fout << _T( "FILE : " ) << sourcefilename << " (" << line << ")" << std::endl;
	if (funcname)
		fout << _T( "FUNC : " ) << funcname << std::endl;


	// _stprintf_s(debugBuf, TCHARSIZE(debugBuf), _T("%s(%d) : %s\n"), sourcefilename, line, message);
	sprintf(debugBuf, "%s(%d) : %s\n", sourcefilename, line, message);
	OutputDebugStringA(debugBuf);
}

void LOGMANAGER::NewLog(const wchar_t* sourcefilename, const wchar_t* funcname, int line, const wchar_t* message, bool bOkay)
{
	// TODO
}

void LOGMANAGER::NewLog(const wchar_t* sourcefilename, int unknown, int line, const char* message, bool bOkay)
{
	// TODO
}

void LOGMANAGER::NewLog( const char* message )
{

}
