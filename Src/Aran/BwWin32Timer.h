#pragma once

#ifndef WIN32
#include <sys/time.h>
#endif

class ARAN_API BwWin32Timer
{
public:
	BwWin32Timer(void);
	~BwWin32Timer(void);

	void start();
	double getTicks();

private:
#ifdef WIN32
	struct			 							// Create A Structure For The Timer Information
	{
		__int64       frequency;							// Timer Frequency
		float         resolution;							// Timer Resolution
		unsigned long mm_timer_start;							// Multimedia Timer Start Value
		unsigned long mm_timer_elapsed;						// Multimedia Timer Elapsed Time
		bool		performance_timer;						// Using The Performance Timer?
		__int64       performance_timer_start;					// Performance Timer Start Value
		__int64       performance_timer_elapsed;					// Performance Timer Elapsed Time
	} timer;
										// Structure Is Named timer

#else
	timespec m_startTime;
#endif
	bool m_bInited;
};
