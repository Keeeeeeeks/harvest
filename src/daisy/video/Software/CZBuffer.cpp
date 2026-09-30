// Copyright (C) 2002-2004 Nikolaus Gebhardt
// Adapted from Irrlicht 0.7 source/Irrlicht/CZBuffer.cpp (license: third_party/irrlicht-0.7/include/irrlicht.h).
// Recovered for Harvest and verified against the Linux amd64 build; not the original source.

#include "CZBuffer.h"
// The object has an iostream static initializer; the header that pulled it in is not identified yet.
#include <iostream> // IWYU pragma: keep

namespace daisy {
namespace video {

CZBuffer::CZBuffer(const ox::core::CDimension2d<int>& size)
    : Buffer(0), Size(0, 0), TotalSize(0), BufferEnd(0)
{
    setSize(size);
}

CZBuffer::~CZBuffer()
{
    if (Buffer)
        delete [] Buffer;
}

void CZBuffer::clear()
{
    TZBufferType* p = Buffer;
    while (p != BufferEnd)
    {
        *p = 0;
        ++p;
    }
}

void CZBuffer::setSize(const ox::core::CDimension2d<int>& size)
{
    if (size == Size)
        return;

    Size = size;
    if (Buffer)
        delete [] Buffer;

    TotalSize = size.Width * size.Height;
    Buffer = new TZBufferType[TotalSize];
    BufferEnd = Buffer + TotalSize;
}

const ox::core::CDimension2d<int>& CZBuffer::getSize()
{
    return Size;
}

TZBufferType* CZBuffer::lock()
{
    return Buffer;
}

void CZBuffer::unlock()
{
}

IZBuffer* createZBuffer(const ox::core::CDimension2d<int>& size)
{
    return new CZBuffer(size);
}

} // end namespace video
} // end namespace daisy
