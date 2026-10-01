// Copyright (C) 2002-2004 Nikolaus Gebhardt
// Adapted from Irrlicht 0.7 for Harvest; not the original Daisy source.
// See third_party/irrlicht-0.7/readme.txt for the zlib/libpng license.

#ifndef DAISY_CFPSCOUNTER_H
#define DAISY_CFPSCOUNTER_H

namespace daisy {
namespace video {

class CFPSCounter {
public:
    CFPSCounter();
    int getFPS();
    void registerFrame(unsigned int now);

private:
    int fps;
    unsigned int startTime;
    unsigned int framesCounted;
};

} // end namespace video
} // end namespace daisy

#endif
