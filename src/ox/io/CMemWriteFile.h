// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Oxeye's growable in-memory counterpart of CMemReadFile. Irrlicht 0.7 has no such class.

#ifndef OX_IO_CMEMWRITEFILE_H
#define OX_IO_CMEMWRITEFILE_H

#include "IWriteFile.h"

namespace ox {
namespace io {

//! Writes to a block of memory that grows as needed.
class CMemWriteFile : public IWriteFile
{
public:
    CMemWriteFile();
    virtual ~CMemWriteFile();

    virtual int write(const void* buffer, int sizeToWrite);
    virtual bool seek(int finalPos, bool relativeMovement = false);
    virtual int getPos();
    virtual const char* getFileName() { return "MemFile"; }
    virtual FILE* getStdioFile() { return 0; }

    //! Number of bytes written so far (the highest position reached).
    int getSize();

    //! Number of bytes allocated.
    int getAvailableSize();

    //! The written data.
    char* getData();

private:
    char* Buffer;
    int Allocated;
    int Size;
    int Pos;
};

} // end namespace io
} // end namespace ox

#endif
