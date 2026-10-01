// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#ifndef OX_IO_CHELPIO_H
#define OX_IO_CHELPIO_H

#include "IReadFile.h"
#include "IWriteFile.h"
#include "../core/CString.h"
#include "IFileSystem.h"

namespace ox {
namespace io {

//! Reads and writes values in files.
class CHelpIO
{
public:
    static int readInt(IReadFile* file);
    static unsigned int readUInt(IReadFile* file);
    static float readFloat(IReadFile* file);
    static unsigned char readByte(IReadFile* file);
    static short readShort(IReadFile* file);
    static unsigned short readUShort(IReadFile* file);
    static void readString(IReadFile* file, core::CString<char>& value);
    static void readStringWithLength(IReadFile* file, core::CString<char>& value);
    static void readWideString(IReadFile* file, core::CString<wchar_t>& value);
    static void writeInt(IWriteFile* file, int value);
    static void writeUInt(IWriteFile* file, unsigned int value);
    static void writeFloat(IWriteFile* file, float value);
    static void writeByte(IWriteFile* file, unsigned char value);
    static void writeShort(IWriteFile* file, short value);
    static void writeUShort(IWriteFile* file, unsigned short value);
    static void writeString(IWriteFile* file, const core::CString<char>& value);
    static void writeWideString(IWriteFile* file, const core::CString<wchar_t>& value, bool terminate);
    static core::CString<char> getNextFreeFilename(IFileSystem* fileSystem,
        const char* prefix, const char* suffix);
};

} // end namespace io
} // end namespace ox

#endif
