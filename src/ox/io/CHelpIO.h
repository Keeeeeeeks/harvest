// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Declarations are added as the functions are recovered.

#ifndef OX_IO_CHELPIO_H
#define OX_IO_CHELPIO_H

#include "IReadFile.h"
#include "IWriteFile.h"
#include "../core/CString.h"

namespace ox {
namespace io {

//! Reads and writes values in files.
class CHelpIO
{
public:
    static int readInt(IReadFile* file);
    static float readFloat(IReadFile* file);
    static void writeInt(IWriteFile* file, int value);
    static void writeFloat(IWriteFile* file, float value);
    static void readString(IReadFile* file, core::CString<char>& value);
    static void writeString(IWriteFile* file, const core::CString<char>& value);
};

} // end namespace io
} // end namespace ox

#endif
