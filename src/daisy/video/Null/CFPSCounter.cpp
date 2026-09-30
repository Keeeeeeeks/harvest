// Copyright (C) 2002-2004 Nikolaus Gebhardt
// Adapted from Irrlicht 0.7 for Harvest; not the original Daisy source.
// See third_party/irrlicht-0.7/readme.txt for the zlib/libpng license.

#include "CFPSCounter.h"
// The object has an iostream initializer; the original including header is unidentified.
#include <iostream>

namespace daisy {
namespace video {

CFPSCounter::CFPSCounter()
    : fps(0), startTime(0), framesCounted(100)
{
}

int CFPSCounter::getFPS()
{
    return fps;
}

void CFPSCounter::registerFrame(unsigned int now)
{
    framesCounted++;
    unsigned int milliseconds = now - startTime;
    if (milliseconds > 2000)
    {
        fps = (int)((float)framesCounted / ((float)milliseconds / 1000.0f));
        startTime = now;
        framesCounted = 0;
    }
}

} // end namespace video
} // end namespace daisy
