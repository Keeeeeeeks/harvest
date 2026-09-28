// Copyright (C) 2002-2004 Nikolaus Gebhardt
// Adapted from Irrlicht 0.7 include/IWriteFile.h (license: third_party/irrlicht-0.7/include/irrlicht.h).
// Recovered for Harvest's ox::io namespace; not the original source. Virtual order follows the
// Mac and Linux 1.18 vtables; getStdioFile is an Oxeye addition.

#ifndef OX_IO_IWRITEFILE_H
#define OX_IO_IWRITEFILE_H

#include <cstdio>
#include "../IUnknown.h"

namespace ox {
namespace io {

//! Interface providing write access to a file.
class IWriteFile : public IUnknown
{
public:
    virtual ~IWriteFile() {};

    //! Writes an amount of bytes to the file.
    virtual int write(const void* buffer, int sizeToWrite) = 0;

    //! Changes the position in the file. Returns true on success.
    virtual bool seek(int finalPos, bool relativeMovement = false) = 0;

    //! Returns the current position in the file in bytes.
    virtual int getPos() = 0;

    //! Returns the file name as a zero-terminated string.
    virtual const char* getFileName() = 0;

    // Return type provisional: a pointer-sized null in the memory implementation.
    virtual FILE* getStdioFile() = 0;
};

} // end namespace io
} // end namespace ox

#endif
