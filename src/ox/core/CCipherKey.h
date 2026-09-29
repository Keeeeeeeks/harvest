// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#ifndef OX_CORE_CCIPHERKEY_H
#define OX_CORE_CCIPHERKEY_H

namespace ox {
namespace core {

//! Key bytes for a cipher.
class CCipherKey
{
public:
    CCipherKey(const void* key, int length);
    //! Parses a string of hex digits, two per byte.
    CCipherKey(const char* hexKey);
    virtual ~CCipherKey();

    int getKeyLength() const;
    const unsigned char* getKey() const;
    //! Does nothing in this build.
    void printKey(bool newLine) const;

private:
    int KeyLength;
    unsigned char* Key;
};

} // end namespace core
} // end namespace ox

#endif
