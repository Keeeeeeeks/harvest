// Copyright (C) 2002-2004 Nikolaus Gebhardt
// Adapted from Irrlicht 0.7 source/Irrlicht/CMemoryReadFile.cpp (license: third_party/irrlicht-0.7/include/irrlicht.h).
// Recovered for Harvest and verified against the Linux amd64 build; not the original source.

#include "CMemReadFile.h"
#include <cstring>
// The object has an iostream static initializer; the header that pulled it in is not identified yet.
#include <iostream>

namespace ox {
namespace io {

CMemReadFile::CMemReadFile(void* memory, int len, bool d)
    : Buffer(memory), Len(len), Pos(0), deleteMemoryWhenDropped(d)
{
}

CMemReadFile::~CMemReadFile()
{
    if (deleteMemoryWhenDropped)
        delete [] (char*)Buffer;
}

int CMemReadFile::read(void* buffer, int sizeToRead)
{
    int amount = sizeToRead;
    if (Pos + amount > Len)
        amount -= Pos + amount - Len;

    if (amount < 0)
        amount = 0;

    char* p = (char*)Buffer;
    memcpy(buffer, p + Pos, amount);

    Pos += static_cast<unsigned int>(amount);

    return amount;
}

bool CMemReadFile::seek(int finalPos, bool relativeMovement)
{
    if (relativeMovement)
    {
        if (Pos + finalPos > Len)
            return false;

        Pos += finalPos;
    }
    else
    {
        if ((unsigned) finalPos > Len)
            return false;

        Pos = finalPos;
    }

    return true;
}

int CMemReadFile::getSize()
{
    return Len;
}

int CMemReadFile::getPos()
{
    return Pos;
}

void* CMemReadFile::getCurrentPointer()
{
    return (char*)Buffer + Pos;
}

int CMemReadFile::getRemainingSize()
{
    return Len - Pos;
}

} // end namespace io
} // end namespace ox
