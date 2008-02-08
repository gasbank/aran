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
		std::cout << "Log file failed\n";
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
		fout << "CNTS : " << okayCount << " Okays, " << failCount << " Fails\n";
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
		std::cout << "Log file failed\n";
	}
	else
	{
		if (bAppend) NewLog("Start log file (Append)");
		else NewLog("Start log file");
	}
}

LOGMANAGER::LOGMANAGER(char *filename)
{
	LOGMANAGER(filename, false);
}

void LOGMANAGER::NewLog(char* message)
{
	NewLog(message, LOG_OKAY);
}

void LOGMANAGER::NewLog(char* message, bool bOkay)
{
	//fout << "-----------------------------------------------------------------------------------------------------------\n";
	fout << "***********\n";
	if (!bOkay)
	{
		fout << "MSG  : [FAILURE] " << message << std::endl;
		failCount++;
	}
	else
	{
		fout << "MSG  : [OKAY] " << message << std::endl;
		okayCount++;
	}
}

void LOGMANAGER::NewLog(char* sourcefilename, char* funcname, int line, char* message, bool bOkay)
{
	NewLog(message, bOkay);
	
	fout << "FILE : " << sourcefilename << " (" << line << ")\n";
	fout << "FUNC : " << funcname << std::endl;
}



