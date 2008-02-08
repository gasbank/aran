// Log.h
// 2006 Geoyeob, Kim (First Revision: MapModel Project)
// 2008 Geoyoeb, Kim (Second Revision: Aran Project)

// Log management class
#pragma once

#ifndef LOG_OKAY
#define LOG_OKAY true
#endif
#ifndef LOG_FAIL
#define LOG_FAIL false
#endif



class LOGMANAGER : public Singleton<LOGMANAGER>
{
public:
	LOGMANAGER();
	~LOGMANAGER();
	LOGMANAGER(char* filename);
	LOGMANAGER(char* filename, bool bAppend);

	void NewLog(char* sourcefilename, char* funcname, int line, char* message, bool bOkay);
	void NewLog(char* message, bool bOkay);
	void NewLog(char* message);
	int GetFailCount() { return failCount; };

private:
	int okayCount;
	int failCount;
	std::ofstream fout;
};

#ifndef _LogWrite
#define _LogWrite(___msg,___okay) \
	LOGMANAGER::getSingleton().NewLog(__FILE__, __FUNCTION__, __LINE__, ___msg, ___okay)
#endif
