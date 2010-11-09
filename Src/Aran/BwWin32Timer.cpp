#include "AranPCH.h"
#include "BwWin32Timer.h"
#ifdef WIN32
	#include <mmsystem.h>
#endif

BwWin32Timer::BwWin32Timer(void)
: m_bInited(false)
{
}

BwWin32Timer::~BwWin32Timer(void)
{
}

void BwWin32Timer::start()
{
#ifdef WIN32
	memset(&timer, 0, sizeof(timer));					// Clear Our Timer Structure

	// Check To See If A Performance Counter Is Available
	// If One Is Available The Timer Frequency Will Be Updated
	if (!QueryPerformanceFrequency((LARGE_INTEGER *) &timer.frequency))
	{
		// No Performace Counter Available
		timer.performance_timer	= FALSE;				// Set Performance Timer To FALSE
		timer.mm_timer_start	= timeGetTime();			// Use timeGetTime() To Get Current Time
		timer.resolution	= 1.0f/1000.0f;				// Set Our Timer Resolution To .001f
		timer.frequency		= 1000;					// Set Our Timer Frequency To 1000
		timer.mm_timer_elapsed	= timer.mm_timer_start;			// Set The Elapsed Time To The Current Time
	}
	else
	{
		// Performance Counter Is Available, Use It Instead Of The Multimedia Timer
		// Get The Current Time And Store It In performance_timer_start
		QueryPerformanceCounter((LARGE_INTEGER *) &timer.performance_timer_start);
		timer.performance_timer		= TRUE;				// Set Performance Timer To TRUE
		// Calculate The Timer Resolution Using The Timer Frequency
		timer.resolution		= (float) (((double)1.0f)/((double)timer.frequency));
		// Set The Elapsed Time To The Current Time
		timer.performance_timer_elapsed	= timer.performance_timer_start;
	}
#else
	// LINUX code goes here ...
	clock_gettime(CLOCK_REALTIME, &m_startTime);
#endif
	m_bInited = true;
}

double BwWin32Timer::getTicks()
{
	assert(m_bInited);
#ifdef WIN32
	__int64 time;								// time Will Hold A 64 Bit Integer

	if (timer.performance_timer)						// Are We Using The Performance Timer?
	{
		QueryPerformanceCounter((LARGE_INTEGER *) &time);		// Grab The Current Performance Time
		// Return The Current Time Minus The Start Time Multiplied By The Resolution And 1000 (To Get MS)
		return ( (double) ( time - timer.performance_timer_start) * timer.resolution) * 1000.0;
	}
	else
	{
		// Return The Current Time Minus The Start Time Multiplied By The Resolution And 1000 (To Get MS)
		return( (double) ( timeGetTime() - timer.mm_timer_start) * timer.resolution) * 1000.0;
	}
#else
	// LINUX code goes here ...
	return 0;
#endif
}
