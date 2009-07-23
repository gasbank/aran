//PreciseTimer.cpp
#include "AranPCH.h"
#include "PreciseTimer.h"

#ifdef WIN32

bool CPreciseTimer::sm_bInit = false;
bool CPreciseTimer::sm_bPerformanceCounter;
__int64_t CPreciseTimer::sm_i64Freq;

void PrintCurrentPreciseTime()
{
	ARN_THROW_NOT_IMPLEMENTED_ERROR
}

CPreciseTimer::CPreciseTimer()
: m_i64Start(0)
, m_i64Elapsed(0)
{
	//Only if not already initialized
	if(false == sm_bInit)
	{
		//Initializing some static variables dependent on the system just once
		LARGE_INTEGER liFreq;
		if(TRUE == QueryPerformanceFrequency(&liFreq))
		{
			//Only if the system is supporting High Performance
			sm_i64Freq = ((__int64)liFreq.HighPart << 32) + (__int64)liFreq.LowPart;
			sm_bPerformanceCounter = true;
		}
		else
			sm_bPerformanceCounter = false;
		sm_bInit = true;
	}
}

void CPreciseTimer::StartTimer_Internal()
{
	if(true == sm_bPerformanceCounter)
	{
		QueryPerformanceCounter(&m_liCount);
		m_i64Start = ((__int64)m_liCount.HighPart << 32) + (__int64)m_liCount.LowPart;
		//Transform in microseconds
		(m_i64Start *= 1000000) /= sm_i64Freq;
	}
	else
		//Transform milliseconds to microseconds
		m_i64Start = (__int64)GetTickCount() * 1000;
}

//Auxiliary Function
void CPreciseTimer::UpdateElapsed()
{
	if(true == sm_bPerformanceCounter)
	{
		QueryPerformanceCounter(&m_liCount);
		m_i64Counts = ((__int64)m_liCount.HighPart << 32) + (__int64)m_liCount.LowPart;
		//Transform in microseconds
		(m_i64Counts *= 1000000) /= sm_i64Freq;
	}
	else
		//Transform milliseconds to microseconds
		m_i64Counts = (__int64)GetTickCount() * 1000;
	if(m_i64Counts > m_i64Start)
		m_i64Elapsed = m_i64Counts - m_i64Start;
	else
		//Eliminate possible number overflow (0x7fffffffffffffff is the maximal __int64 positive number)
		m_i64Elapsed = (0x7fffffffffffffff - m_i64Start) + m_i64Counts;

	setElapsed((double)m_i64Elapsed / 1e6);
}

#else


void PrintCurrentPreciseTime()
{
	timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	printf("Current Sec: %ld / Current Nanosec: %ld\n", ts.tv_sec, ts.tv_nsec);
}

void CPreciseTimer::StartTimer_Internal()
{
	clock_gettime(CLOCK_REALTIME, &m_tsStart);
}

void CPreciseTimer::RestartTimer_Internal()
{
	clock_gettime(CLOCK_REALTIME, &m_tsStart);
}

void CPreciseTimer::UpdateElapsed()
{
	if (isRunning())
	{
		clock_gettime(CLOCK_REALTIME, &m_tsEnd);
		__time_t ds = m_tsEnd.tv_sec - m_tsStart.tv_sec; // sec diff
		long int dns = m_tsEnd.tv_nsec - m_tsStart.tv_nsec; // nsec diff
		if (dns < 0)
		{
			--ds;
			dns += 1000000000;
		}
		assert(ds >= 0);
		setElapsed((double)ds + (double)dns/1000000000);
	}
}

#endif

