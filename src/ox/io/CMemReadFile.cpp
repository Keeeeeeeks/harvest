// Copyright (C) 2002-2004 Nikolaus Gebhardt
// Adapted from Irrlicht 0.7 CMemoryReadFile.cpp and verified against Harvest's
// Linux amd64 code. This is an altered recovery, not the original source.
// License notice: third_party/irrlicht-0.7/include/irrlicht.h.

#include "CMemReadFile.h"
#include <cstring>

namespace ox { namespace io {

bool CMemReadFile::seek(int finalPos, bool relativeMovement)
{
    if (relativeMovement) {
        if (Pos + finalPos > Len)
            return false;
        Pos += finalPos;
    } else {
        if (static_cast<unsigned int>(finalPos) > Len)
            return false;
        Pos = finalPos;
    }
    return true;
}

int CMemReadFile::getRemainingSize()
{
    return Len - Pos;
}

int CMemReadFile::read(void* buffer, int sizeToRead)
{
    int amount = sizeToRead;
    if (Pos + amount > Len)
        amount -= Pos + amount - Len;
    if (amount < 0)
        amount = 0;
    std::memcpy(buffer, static_cast<char*>(Buffer) + Pos, amount);
    Pos += static_cast<unsigned int>(amount);
    return amount;
}

} }
