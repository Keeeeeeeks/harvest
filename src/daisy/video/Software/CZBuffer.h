// Copyright (C) 2002-2004 Nikolaus Gebhardt
// Adapted from Irrlicht 0.7 source/Irrlicht/CZBuffer.h (license: third_party/irrlicht-0.7/include/irrlicht.h).
// Recovered for Harvest's daisy namespace; not the original source.

#ifndef DAISY_VIDEO_CZBUFFER_H
#define DAISY_VIDEO_CZBUFFER_H

#include "IZBuffer.h"

namespace daisy {
namespace video {

class CZBuffer : public IZBuffer
{
public:
    CZBuffer(const ox::core::CDimension2d<int>& size);
    virtual ~CZBuffer();
    virtual void clear();
    virtual void setSize(const ox::core::CDimension2d<int>& size);
    virtual const ox::core::CDimension2d<int>& getSize();
    virtual TZBufferType* lock();
    virtual void unlock();

private:
    TZBufferType* Buffer;
    TZBufferType* BufferEnd;
    ox::core::CDimension2d<int> Size;
    int TotalSize;
};

} // end namespace video
} // end namespace daisy

#endif
