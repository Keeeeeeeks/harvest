// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_IRR_C_TIMER_H_INCLUDED__
#define __C_IRR_C_TIMER_H_INCLUDED__

#include "ITimer.h"
#include "os.h"

namespace irr
{
	//! Device independent implementation of the timer
	class CTimer : public ITimer
	{
	public:

		CTimer()
		{
			os::Timer::initTimer();
		}

		//! returns current time in milliseconds. This value does not start
		//! with 0 when the application starts. For example in one implementation
		//! the value returne could be the amount of milliseconds which
		//! have elapsed since the system was started.
		virtual u32 getTime()
		{
			return os::Timer::getTime();
		}
	};

} // end namespace

#endif
