// Recovered for Harvest from the Linux amd64 1.18 build; not the original source.

#include "CHelpIO.h"
// The original unit also emits an iostream static initializer.
#include <iostream>

namespace ox {
namespace io {

int CHelpIO::readInt(IReadFile* file)
{
    int value = 0;
    file->read(&value, sizeof(value));
    return value;
}

unsigned int CHelpIO::readUInt(IReadFile* file)
{
    unsigned int value = 0;
    file->read(&value, sizeof(value));
    return value;
}

float CHelpIO::readFloat(IReadFile* file)
{
    float value = 0;
    file->read(&value, sizeof(value));
    return value;
}

unsigned char CHelpIO::readByte(IReadFile* file)
{
    unsigned char value = 0;
    file->read(&value, sizeof(value));
    return value;
}

short CHelpIO::readShort(IReadFile* file)
{
    short value = 0;
    file->read(&value, sizeof(value));
    return value;
}

unsigned short CHelpIO::readUShort(IReadFile* file)
{
    unsigned short value = 0;
    file->read(&value, sizeof(value));
    return value;
}

void CHelpIO::readString(IReadFile* file, core::CString<char>& value)
{
    char buffer[256];
    buffer[255] = 0;
    int count = 0;
    char character;
    int read = file->read(&character, 1);
    value = "";
    while (character && read > 0)
    {
        buffer[count++] = character;
        if (count == 255)
        {
            value.append(buffer);
            count = 0;
        }
        read = file->read(&character, 1);
    }
    if (count > 0)
    {
        buffer[count] = 0;
        value.append(buffer);
    }
}

void CHelpIO::readStringWithLength(IReadFile* file, core::CString<char>& value)
{
    int length = readInt(file);
    if (length > 0)
    {
        char* buffer = new char[length + 1];
        file->read(buffer, length);
        buffer[length] = 0;
        value = buffer;
        delete [] buffer;
    }
}

void CHelpIO::readWideString(IReadFile* file, core::CString<wchar_t>& value)
{
    wchar_t buffer[256];
    buffer[255] = 0;
    int count = 0;
    wchar_t character = readUShort(file);
    value = L"";
    while (character)
    {
        buffer[count++] = character;
        if (count == 255)
        {
            value.append(buffer);
            count = 0;
        }
        character = readUShort(file);
    }
    if (count > 0)
    {
        buffer[count] = 0;
        value.append(buffer);
    }
}

void CHelpIO::writeInt(IWriteFile* file, int value)
{
    file->write(&value, sizeof(value));
}

void CHelpIO::writeUInt(IWriteFile* file, unsigned int value)
{
    file->write(&value, sizeof(value));
}

void CHelpIO::writeFloat(IWriteFile* file, float value)
{
    file->write(&value, sizeof(value));
}

void CHelpIO::writeByte(IWriteFile* file, unsigned char value)
{
    file->write(&value, sizeof(value));
}

void CHelpIO::writeShort(IWriteFile* file, short value)
{
    file->write(&value, sizeof(value));
}

void CHelpIO::writeUShort(IWriteFile* file, unsigned short value)
{
    file->write(&value, sizeof(value));
}

void CHelpIO::writeString(IWriteFile* file, const core::CString<char>& value)
{
    const char* character = value.c_str();
    while (*character)
    {
        file->write(character, 1);
        ++character;
    }
    file->write(character, 1);
}

void CHelpIO::writeWideString(IWriteFile* file, const core::CString<wchar_t>& value, bool terminate)
{
    const wchar_t* character = value.c_str();
    while (*character)
    {
        writeUShort(file, (unsigned short)*character);
        ++character;
    }
    if (terminate)
    {
        unsigned short end = 0;
        file->write(&end, sizeof(end));
    }
}

core::CString<char> CHelpIO::getNextFreeFilename(IFileSystem* fileSystem,
    const char* prefix, const char* suffix)
{
    core::CString<char> filename;
    int number = 0;
    while (true)
    {
        filename = prefix;
        if (number < 10)
            filename += "0";
        filename.append(number);
        filename += suffix;
        ++number;
        if (!fileSystem->existFile(filename.c_str(), false))
            return core::CString<char>(filename);
    }
}

} // end namespace io
} // end namespace ox
