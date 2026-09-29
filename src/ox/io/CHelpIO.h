// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Declarations are added as the functions are recovered.

#ifndef OX_IO_CHELPIO_H
#define OX_IO_CHELPIO_H

#include "IReadFile.h"
#include "IWriteFile.h"

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
};

} // end namespace io
} // end namespace ox

#endif
