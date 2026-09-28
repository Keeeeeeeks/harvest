// Copyright (C) 2002-2004 Nikolaus Gebhardt
// Adapted from Irrlicht 0.7 include/IUnknown.h (license: third_party/irrlicht-0.7/include/irrlicht.h).
// Recovered for Harvest's ox:: namespace; not the original source.

#ifndef OX_IUNKNOWN_H
#define OX_IUNKNOWN_H

namespace ox {

//! Base class of reference-counted engine objects.
class IUnknown
{
public:
    IUnknown()
        : ReferenceCounter(1), DebugName(0)
    {
    }

    virtual ~IUnknown()
    {
    }

    void grab() { ++ReferenceCounter; }

    bool drop()
    {
        --ReferenceCounter;
        if (!ReferenceCounter)
        {
            delete this;
            return true;
        }
        return false;
    }

    const char* getDebugName() const
    {
        return DebugName;
    }

protected:
    void setDebugName(const char* newName)
    {
        DebugName = newName;
    }

private:
    int ReferenceCounter;
    const char* DebugName;
};

} // end namespace ox

#endif
