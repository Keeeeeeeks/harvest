// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#include "CSimplePress.h"
#include <cstring>
// The object has an iostream static initializer; the header that pulled it in is not identified yet.
#include <iostream>

namespace ox {
namespace algo {

unsigned char CSimplePress::m_staticIndices[0x8000];

CSimplePress::CSimplePress()
{
    Table = new unsigned char[0x8000];
    memset(Table, 0, 0x8000);
}

CSimplePress::~CSimplePress()
{
    if (Table)
        delete [] Table;
}

int CSimplePress::compress(unsigned char* out, unsigned char* in, int length)
{
    return compressData(out, in, length, Table);
}

int CSimplePress::compressData(unsigned char* out, unsigned char* in, int length, unsigned char* table)
{
    int count = 0;
    unsigned char mask = 0;
    int i = 1;
    int outPos = 0;
    int prev2 = 0;
    unsigned char c = in[0];
    int prev = 0;
    int bit = 0;
    unsigned char buffer[8];

    while (i <= length)
    {
        int hash = (prev2 << 7) ^ prev;
        unsigned char guess = table[hash];
        if (c == guess)
            mask ^= 1 << bit;
        else
        {
            table[hash] = c;
            buffer[count] = c;
            count++;
        }

        bit++;
        if (bit == 8)
        {
            out[outPos++] = mask;
            for (int j = 0; j < count; j++)
                out[outPos++] = buffer[j];
            count = 0;
            mask = 0;
            bit = 0;
        }

        prev2 = prev;
        prev = c;
        if (i < length)
            c = in[i];
        i++;
    }

    if (bit != 0)
    {
        out[outPos++] = mask;
        for (int j = 0; j < count; j++)
            out[outPos++] = buffer[j];
    }

    return outPos;
}

int CSimplePress::decompress(unsigned char* out, unsigned char* in, int length)
{
    return deCompressData(out, in, length, Table);
}

int CSimplePress::deCompressData(unsigned char* out, unsigned char* in, int length, unsigned char* table)
{
    unsigned char mask = in[0];
    unsigned char c;
    int prev2 = 0;
    int prev = 0;
    int i = 1;
    int outPos = 0;

    while (i <= length)
    {
        for (int bit = 0; bit < 8; bit++)
        {
            if (mask & (1 << bit))
                c = table[(prev2 << 7) ^ prev];
            else
            {
                if (i < length)
                    c = in[i];
                i++;
                if (i > length)
                    return outPos;
                table[(prev2 << 7) ^ prev] = c;
            }
            out[outPos++] = c;
            prev2 = prev;
            prev = c;
        }

        if (i < length)
            mask = in[i];
        i++;
    }

    return outPos;
}

int CSimplePress::compressData(unsigned char* out, unsigned char* in, int length)
{
    memset(m_staticIndices, 0, sizeof(m_staticIndices));
    return compressData(out, in, length, m_staticIndices);
}

int CSimplePress::deCompressData(unsigned char* out, unsigned char* in, int length)
{
    memset(m_staticIndices, 0, sizeof(m_staticIndices));
    return deCompressData(out, in, length, m_staticIndices);
}

} // end namespace algo
} // end namespace ox
