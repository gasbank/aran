#ifdef WIN32
inline bool CPreciseTimer::SupportsHighResCounter() const
{
	return sm_bPerformanceCounter;
}
#endif
