// PreciseTimer.h
#ifndef _PRECISETIMER_H_
#define _PRECISETIMER_H_


void PrintCurrentPreciseTime();

#ifdef WIN32
typedef __int64 __int64_t;
#endif

// ArnTimer class implements a stopwatch-style timer.
// We can access only a single time value; the current time displayed in the stopwatch.
// Note that when the timer is stopped, the time value will not be changed
// even though as time elapses.
class ArnTimer
{
public:
						ArnTimer() : m_bRunning(false) {}
	void				StartTimer() { assert(!m_bRunning); m_bRunning = true; StartTimer_Internal(); }
	void				RestartTimer() { assert(m_bRunning); RestartTimer_Internal(); }
	void				StopTimer() { assert(m_bRunning); m_bRunning = false; UpdateElapsed(); }
	double				GetTimeInSeconds() { UpdateElapsed(); return m_elapsed; }
	bool				isRunning() const { return m_bRunning; }

protected:
	void				setElapsed(double elapsed) { m_elapsed = elapsed; }

private:
	virtual void		UpdateElapsed() = 0; // Store the lap time.
	virtual void		StartTimer_Internal() = 0;
	virtual void		RestartTimer_Internal() = 0;

	bool				m_bRunning;
	double				m_elapsed;
};

#ifdef WIN32
//More precise Timer for measuring time intervals in microseconds.
//The performance of this Timer is dependent on the performance of the system.
class CPreciseTimer : public ArnTimer
{
public:
						CPreciseTimer();
	virtual void		PrintTime() const { ARN_THROW_NOT_IMPLEMENTED_ERROR };
	inline bool			SupportsHighResCounter() const;
private:
	virtual void		StartTimer_Internal();
	virtual void		RestartTimer_Internal() { ARN_THROW_NOT_IMPLEMENTED_ERROR }
	virtual void		UpdateElapsed(); //Auxiliary Function

	__int64_t			m_i64Start;
	__int64_t			m_i64Elapsed;
	__int64_t			m_i64Counts;
	LARGE_INTEGER		m_liCount;

	static bool			sm_bInit;
	static bool			sm_bPerformanceCounter;
	static __int64_t	sm_i64Freq;
};

#else

#include <unistd.h>
#include <time.h>

class CPreciseTimer : public ArnTimer
{
public:
						CPreciseTimer() {}
						~CPreciseTimer() {}

private:
	virtual void		StartTimer_Internal();
	virtual void		RestartTimer_Internal();
	virtual void		UpdateElapsed();

	timespec			m_tsStart;
	timespec			m_tsEnd;
};

#endif

#include "PreciseTimer.inl"

#endif // _PRECISETIMER_H_


