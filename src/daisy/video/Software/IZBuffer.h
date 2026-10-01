// Copyright (C) 2002-2004 Nikolaus Gebhardt
// Adapted from Irrlicht 0.7 source/Irrlicht/IZBuffer.h (license: third_party/irrlicht-0.7/include/irrlicht.h).
// Recovered for Harvest's daisy namespace; not the original source.

#ifndef DAISY_VIDEO_IZBUFFER_H
#define DAISY_VIDEO_IZBUFFER_H

#include "ox/IUnknown.h"
#include "ox/core/CDimension2d.h"

namespace daisy {
namespace video {

typedef signed short TZBufferType;

class IZBuffer : public ox::IUnknown
{
public:
    virtual ~IZBuffer() {}
    virtual void clear() = 0;
    virtual void setSize(const ox::core::CDimension2d<int>& size) = 0;
    virtual const ox::core::CDimension2d<int>& getSize() = 0;
    virtual TZBufferType* lock() = 0;
    virtual void unlock() = 0;
};

IZBuffer* createZBuffer(const ox::core::CDimension2d<int>& size);

} // end namespace video
} // end namespace daisy

#endif
