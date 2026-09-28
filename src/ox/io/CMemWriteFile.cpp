// Recovered for Harvest and verified against the Linux amd64 build; not the original source.

#include "CMemWriteFile.h"
#include <cstring>
// The object has an iostream static initializer; the header that pulled it in is not identified yet.
#include <iostream>

namespace ox {
namespace io {

CMemWriteFile::CMemWriteFile()
    : Buffer(0), Allocated(0), Size(0), Pos(0)
{
}

CMemWriteFile::~CMemWriteFile()
{
    delete [] Buffer;
}

int CMemWriteFile::write(const void* buffer, int sizeToWrite)
{
    if (!Buffer || Pos + sizeToWrite > Allocated)
    {
        int newSize = Allocated * 3 / 2;
        if (Allocated + sizeToWrite > newSize)
            newSize = Allocated + sizeToWrite;

        char* newBuffer = new char[newSize];
        if (Buffer)
        {
            memcpy(newBuffer, Buffer, Allocated);
            delete [] Buffer;
        }

        Buffer = newBuffer;
        Allocated = newSize;
    }

    memcpy(Buffer + Pos, buffer, sizeToWrite);
    Pos += sizeToWrite;
    // the expansion of Irrlicht's core::max_(Pos, Size); an `if` store compiles differently
    Size = Pos < Size ? Size : Pos;

    return 0;
}

bool CMemWriteFile::seek(int finalPos, bool relativeMovement)
{
    if (relativeMovement)
    {
        // operand order matters: `Pos + finalPos > Size` compiles to a different compare
        if (Size < Pos + finalPos)
            Pos = Size;
        else if (Pos + finalPos < 0)
            Pos = 0;
        else
            Pos += finalPos;
    }
    else
    {
        if (finalPos > Size)
            Pos = Size;
        else if (finalPos < 0)
            Pos = 0;
        else
            Pos = finalPos;
    }

    return true;
}

int CMemWriteFile::getPos()
{
    return Pos;
}

int CMemWriteFile::getSize()
{
    return Size;
}

int CMemWriteFile::getAvailableSize()
{
    return Allocated;
}

char* CMemWriteFile::getData()
{
    return Buffer;
}

} // end namespace io
} // end namespace ox
