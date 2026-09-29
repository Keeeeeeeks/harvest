// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source. The file name is
// inferred: the Mac debug map records no header for these classes.

#ifndef OX_NET_CVARIABLEPACKET_H
#define OX_NET_CVARIABLEPACKET_H

#include "../core/CString.h"

namespace ox {
namespace net {

class INetworkDevice;

//! Reads a packet: a short packet id and a short total length, then the appended values.
class CVariablePacketParser
{
public:
    CVariablePacketParser(const void* data);
    virtual ~CVariablePacketParser();

    int getPacketId();

    int fetchInt();
    unsigned char fetchByte();
    short fetchShort();
    float fetchFloat();
    void fetchBytes(unsigned char* buffer, int size);
    void fetchString(core::CString<char>& string);
    void fetchFixedString(core::CString<char>& string, int length);
    void fetchWideString(core::CString<wchar_t>& string);
    void fetchFixedWideString(core::CString<wchar_t>& string, int length);

private:
    const char* Data;
    int Pos;
    int Size;
    int PacketId;
};

//! Builds a packet in a growable buffer, starting with the id and length header.
class CVariablePacketBuilder
{
public:
    CVariablePacketBuilder(int packetId);
    virtual ~CVariablePacketBuilder();

    void appendData(const void* data, int size);

    //! Writes the current length into the header.
    void setHeader();
    int getPacketLength();
    void sendPacket(INetworkDevice* device, int messageId, int flags);
    char* getPacketData();

    void appendInt(int value);
    void appendFloat(float value);
    void appendShort(short value);
    void appendByte(unsigned char value);
    void appendString(const core::CString<char>& string);
    //! Appends each character as a short, then a zero.
    void appendWideString(const core::CString<wchar_t>& string);

private:
    char* Data;
    int Length;
    int Capacity;
};

} // end namespace net
} // end namespace ox

#endif
