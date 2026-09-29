// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#include "CCipherKey.h"
#include <cstring>
// The object has an iostream static initializer; the header that pulled it in is not identified yet.
#include <iostream>

namespace ox {
namespace core {

CCipherKey::~CCipherKey()
{
    // scalar delete of the array, as in the Linux build (the Mac build has delete [])
    delete Key;
}

CCipherKey::CCipherKey(const void* key, int length)
{
    KeyLength = length;
    Key = new unsigned char[KeyLength];
    if (Key)
        memcpy(Key, key, KeyLength);
}

CCipherKey::CCipherKey(const char* hexKey)
{
    KeyLength = ((int)strlen(hexKey) + 1) >> 1;
    Key = new unsigned char[KeyLength];
    if (!Key)
        return;

    memset(Key, 0, KeyLength);

    unsigned char* key = Key;
    size_t offset = KeyLength % 2;
    bool low = offset == 1;
    for (size_t j = 0; hexKey[j]; ++j)
    {
        if (hexKey[j] >= '0' && hexKey[j] <= '9')
            key[(offset + j) / 2] |= (hexKey[j] - '0') << (low ? 0 : 4);
        else if (hexKey[j] >= 'a' && hexKey[j] <= 'f')
            key[(offset + j) / 2] |= (hexKey[j] - 'a' + 10) << (low ? 0 : 4);
        else
            key[(offset + j) / 2] |= (hexKey[j] - 'A' + 10) << (low ? 0 : 4);
        low = !low;
    }
}

int CCipherKey::getKeyLength() const
{
    return KeyLength;
}

const unsigned char* CCipherKey::getKey() const
{
    return Key;
}

void CCipherKey::printKey(bool newLine) const
{
}

} // end namespace core
} // end namespace ox
