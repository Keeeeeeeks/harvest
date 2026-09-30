// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#ifndef OX_ALGO_CSIMPLEPRESS_H
#define OX_ALGO_CSIMPLEPRESS_H

namespace ox {
namespace algo {

//! A predictor compressor: a table indexed by the previous two bytes guesses the next one. Each
//! group of eight bytes becomes a mask byte (a set bit for every correct guess) followed by the
//! bytes that were guessed wrong.
class CSimplePress
{
public:
    CSimplePress();
    virtual ~CSimplePress();

    //! Compresses with this object's table, which carries over between calls. Returns the output size.
    int compress(unsigned char* out, unsigned char* in, int length);
    static int compressData(unsigned char* out, unsigned char* in, int length, unsigned char* table);

    //! Decompresses with this object's table. Returns the output size.
    int decompress(unsigned char* out, unsigned char* in, int length);
    static int deCompressData(unsigned char* out, unsigned char* in, int length, unsigned char* table);

    //! Compress and decompress with a cleared shared table.
    static int compressData(unsigned char* out, unsigned char* in, int length);
    static int deCompressData(unsigned char* out, unsigned char* in, int length);

private:
    unsigned char* Table;

    static unsigned char m_staticIndices[0x8000];
};

} // end namespace algo
} // end namespace ox

#endif
