// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "os.h"
#include "irrString.h"
#include <stdio.h>

#if defined(WIN32) || defined(_XBOX)
// ----------------------------------------------------------------
// Windows specific functions
// ----------------------------------------------------------------

#ifdef WIN32
#include <windows.h>
#endif
#ifdef _XBOX
#include <xtl.h>
#endif

namespace irr
{
namespace os
{
	//! prints a debuginfo string
	void Printer::print(const c8* message)
	{
		c8* tmp = new c8[strlen(message) + 2];
		sprintf(tmp, "%s\n", message);
		OutputDebugString(tmp);
		printf(tmp);
		delete [] tmp;
	}

	LARGE_INTEGER HighPerformanceFreq;
	BOOL HighPerformanceTimerSupport = FALSE;

	void Timer::initTimer()
	{
		HighPerformanceTimerSupport = QueryPerformanceFrequency(&HighPerformanceFreq);
	}

    u32 Timer::getTime()
	{
		if (HighPerformanceTimerSupport)
		{
			LARGE_INTEGER nTime;
			QueryPerformanceCounter(&nTime);
			return u32((nTime.QuadPart) * 1000 / HighPerformanceFreq.QuadPart);
		}

		return GetTickCount();
	}

} // end namespace os


#else

// ----------------------------------------------------------------
// linux/ansi version
// ----------------------------------------------------------------

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

namespace irr
{
namespace os
{

	//! prints a debuginfo string
	void Printer::print(const c8* message)
	{
		printf("%s\n", message);
	}

	void Timer::initTimer()
	{
	}
	
	u32 Timer::getTime()
	{
		static timeval tv;
		gettimeofday(&tv, 0);
		return (u32)(tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	}

} // end namespace os

#endif // end linux / windows

namespace os
{
	// The platform independent implementation of the printer
	ILogger* Printer::Logger = 0;

	void Printer::log(const c8* message, ELOG_LEVEL ll)
	{
		if (Logger)
			Logger->log(message, ll);
	}

	void Printer::log(const c8* message, const c8* hint, ELOG_LEVEL ll)
	{
		if (!Logger)
			return;

		Logger->log(message, hint, ll);
	}

	void Printer::log(const wchar_t* message, ELOG_LEVEL ll)
	{
		if (Logger)
			Logger->log(message, ll);
	}


	// our Randomizer is not really os specific, so we
	// code one for all, which should work on every platform the same,
	// which is desireable.

	s32 Randomizer::seed = 0x0f0f0f0f;

	//! generates a pseudo random number
	s32 Randomizer::rand()
	{
		const s32 m = 2147483399;	// a non-Mersenne prime
		const s32 a = 40692;		// another spectral success story
		const s32 q = 52774;		// m/a
		const s32 r = 3791;			// m%a; again less than q

		seed = a * (seed%q) - r* (seed/q);
		if (seed<0) seed += m;

		return seed;
	}

	//! resets the randomizer
	void Randomizer::reset()
	{
		seed = 0x0f0f0f0f;
	}

} // end namespace os
} // end namespace irr

