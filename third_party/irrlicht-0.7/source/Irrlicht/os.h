// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __IRR_OS_H_INCLUDED__
#define __IRR_OS_H_INCLUDED__

#include "irrTypes.h"
#include "ILogger.h"

namespace irr
{

namespace os
{
	class Printer
	{
	public:
		// prints out a string to the console out stdout or debug log or whatever
		static void print(const c8* message);
		static void log(const c8* message, ELOG_LEVEL ll = ELL_INFORMATION);
		static void log(const c8* message, const c8* hint, ELOG_LEVEL ll = ELL_INFORMATION);
		static void log(const wchar_t* message, ELOG_LEVEL ll = ELL_INFORMATION);
		static ILogger* Logger;
	};


	class Randomizer
	{
	public:

		//! resets the randomizer
		static void reset();

		//! generates a pseudo random number
		static s32 rand();

	private:

		static s32 seed;
	};




	class Timer
	{
	public:

		//! returns the current time in milliseconds
		static u32 getTime();
		static void initTimer();
	};

} // end namespace os
} // end namespace irr


#endif

