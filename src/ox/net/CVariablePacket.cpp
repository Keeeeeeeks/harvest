// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#include "CVariablePacket.h"
#include "INetworkDevice.h"
#include <cstring>
// The object has an iostream static initializer; the header that pulled it in is not identified yet.
#include <iostream>

namespace ox {
namespace net {

CVariablePacketParser::CVariablePacketParser(const void* data)
    : Data((const char*)data), Pos(4)
{
    PacketId = ((const short*)data)[0];
    Size = ((const short*)data)[1];
}

CVariablePacketParser::~CVariablePacketParser()
{
}

int CVariablePacketParser::getPacketId()
{
    return PacketId;
}

int CVariablePacketParser::fetchInt()
{
    if (Pos + 4 > Size)
        return 0;

    const int* value = (const int*)(Data + Pos);
    Pos += 4;
    return *value;
}

unsigned char CVariablePacketParser::fetchByte()
{
    if (Pos >= Size)
        return 0;

    unsigned char value = Data[Pos];
    Pos += 1;
    return value;
}

short CVariablePacketParser::fetchShort()
{
    if (Pos + 2 > Size)
        return 0;

    const short* value = (const short*)(Data + Pos);
    Pos += 2;
    return *value;
}

float CVariablePacketParser::fetchFloat()
{
    if (Pos + 4 > Size)
        return 0;

    const float* value = (const float*)(Data + Pos);
    Pos += 4;
    return *value;
}

void CVariablePacketParser::fetchBytes(unsigned char* buffer, int size)
{
    if (Pos + size > Size)
        return;

    memcpy(buffer, Data + Pos, size);
    Pos += size;
}

void CVariablePacketParser::fetchString(core::CString<char>& string)
{
    if (Pos >= Size)
        return;

    string = Data + Pos;
    Pos += string.size() + 1;
}

void CVariablePacketParser::fetchFixedString(core::CString<char>& string, int length)
{
    if (Pos + length > Size)
        return;

    char* buffer = new char[length + 1];
    memcpy(buffer, Data + Pos, length);
    Pos += length;
    buffer[length] = 0;
    string = buffer;
    delete [] buffer;
}

void CVariablePacketParser::fetchWideString(core::CString<wchar_t>& string)
{
    if (Pos >= Size)
        return;

    const short* p = (const short*)(Data + Pos);
    string = L"";
    while (*p)
    {
        string.append((wchar_t)*p);
        ++p;
    }
    Pos += (string.size() + 1) * sizeof(short);
}

void CVariablePacketParser::fetchFixedWideString(core::CString<wchar_t>& string, int length)
{
    // checks length shorts but copies length wchar_ts, as the original does
    if (Pos + length * 2 > Size)
        return;

    wchar_t* buffer = new wchar_t[length + 1];
    memcpy(buffer, Data + Pos, length * sizeof(wchar_t));
    Pos += length * sizeof(wchar_t);
    buffer[length] = 0;
    string = buffer;
    delete [] buffer;
}

CVariablePacketBuilder::CVariablePacketBuilder(int packetId)
    : Data(0), Length(0), Capacity(0)
{
    short header[2];
    header[0] = packetId;
    header[1] = 0;
    appendData(header, 4);
}

void CVariablePacketBuilder::appendData(const void* data, int size)
{
    if (!Data)
    {
        Capacity = 16;
        if (size > Capacity)
            Capacity = size;
        Data = new char[Capacity];
        memcpy(Data, data, size);
        Length = size;
        return;
    }

    if (Length + size > Capacity)
    {
        int newCapacity = Capacity * 2;
        if (newCapacity < Length + size)
            newCapacity = Length + size;

        char* newData = new char[newCapacity];
        memcpy(newData, Data, Capacity);
        delete [] Data;
        Data = newData;
        Capacity = newCapacity;
    }

    memcpy(Data + Length, data, size);
    Length += size;
}

CVariablePacketBuilder::~CVariablePacketBuilder()
{
    delete [] Data;
}

void CVariablePacketBuilder::setHeader()
{
    ((short*)Data)[1] = Length;
}

int CVariablePacketBuilder::getPacketLength()
{
    return Length;
}

void CVariablePacketBuilder::sendPacket(INetworkDevice* device, int messageId, int flags)
{
    if (!Data)
        return;

    ((short*)Data)[1] = Length;
    device->sendMessage(messageId, Data, Length, flags);
}

char* CVariablePacketBuilder::getPacketData()
{
    return Data;
}

void CVariablePacketBuilder::appendInt(int value)
{
    appendData(&value, 4);
}

void CVariablePacketBuilder::appendFloat(float value)
{
    appendData(&value, 4);
}

void CVariablePacketBuilder::appendShort(short value)
{
    short data = value;
    appendData(&data, 2);
}

void CVariablePacketBuilder::appendByte(unsigned char value)
{
    appendData(&value, 1);
}

void CVariablePacketBuilder::appendString(const core::CString<char>& string)
{
    appendData(string.c_str(), string.size() + 1);
}

void CVariablePacketBuilder::appendWideString(const core::CString<wchar_t>& string)
{
    const wchar_t* p = string.c_str();
    while (*p)
    {
        appendShort((short)*p);
        ++p;
    }
    appendShort(0);
}

} // end namespace net
} // end namespace ox
