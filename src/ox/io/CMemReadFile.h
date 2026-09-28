#ifndef OX_IO_CMEMREADFILE_H
#define OX_IO_CMEMREADFILE_H

namespace ox { namespace io {

// Partial Linux amd64 declaration for the matching pilot. The 24-byte base
// subobject consists of the vptr and 16 bytes whose types are not recovered yet.
// Do not instantiate this class: constructors, bases and other methods remain
// unrecovered. Member offsets are established from the Linux implementation.
// Unimplemented methods reserve observed virtual slots; their return types
// are provisional. This pilot proves only the three implemented method bodies.
class CMemReadFile {
public:
    virtual ~CMemReadFile();
    virtual int read(void* buffer, int sizeToRead);
    virtual int readLine(char* buffer, int size);
    virtual bool seek(int finalPos, bool relativeMovement = false);
    virtual int getSize();
    virtual int getPos();
    virtual const char* getFileName();
    virtual unsigned int getModifiedDate();
    virtual void* getCurrentPointer();
    virtual int getRemainingSize();

private:
    unsigned char unrecoveredBase[16];
    void* Buffer;                         // 0x18
    unsigned int Len;                     // 0x20
    unsigned int Pos;                     // 0x24
    bool deleteMemoryWhenDropped;         // 0x28
};

} }
#endif
