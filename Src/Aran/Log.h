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
	LOGMANAGER(const char* filename);
	LOGMANAGER(const char* filename, bool bAppend);

	void NewLog(const char* sourcefilename, const char* funcname, int line, const char* message, bool bOkay);
	void NewLog(const wchar_t* sourcefilename, const wchar_t* funcname, int line, const wchar_t* message, bool bOkay);
	void NewLog(const wchar_t* sourcefilename, int unknown, int line, const char* message, bool bOkay);
	void NewLog(const char* message, bool bOkay);
	void NewLog(const wchar_t* message, bool bOkay);
	void NewLog(const char* message);
	int GetFailCount() { return failCount; };

private:
	int okayCount;
	int failCount;
	std::ofstream fout;
	//std::wofstream fout;

	char debugBuf[256];
};

#define QUOTEME(x) #x

#ifndef _LogWrite

#ifdef WIN32
	#define _LogWrite(___msg,___okay)																	\
	{																									\
		LOGMANAGER::getSingleton().NewLog(_T(__FILE__), _T(__FUNCTION__), __LINE__, ___msg, ___okay);	\
	}
#else
	#define _LogWrite(___msg,___okay)															\
	{																							\
		LOGMANAGER::getSingleton().NewLog(__WFILE__, 0, __LINE__, ___msg, ___okay);	\
	}
#endif

#endif
