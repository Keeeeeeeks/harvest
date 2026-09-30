// Copyright (C) 2002-2004 Nikolaus Gebhardt
// Adapted from Irrlicht 0.7 include/dimension2d.h (license: third_party/irrlicht-0.7/include/irrlicht.h).
// Recovered for Harvest's ox::core namespace; not the original source. The file name is inferred.

#ifndef OX_CORE_CDIMENSION2D_H
#define OX_CORE_CDIMENSION2D_H

namespace ox {
namespace core {

//! Specifies a 2 dimensional size.
template <class T>
class CDimension2d
{
public:
    CDimension2d()
        : Width(0), Height(0) {}

    CDimension2d(T width, T height)
        : Width(width), Height(height) {}

    CDimension2d(const CDimension2d<T>& other)
        : Width(other.Width), Height(other.Height) {}

    bool operator==(const CDimension2d<T>& other) const
    {
        return Width == other.Width && Height == other.Height;
    }

    bool operator!=(const CDimension2d<T>& other) const
    {
        return Width != other.Width || Height != other.Height;
    }

    const CDimension2d<T>& operator=(const CDimension2d<T>& other)
    {
        Width = other.Width;
        Height = other.Height;
        return *this;
    }

    T Width, Height;
};

} // end namespace core
} // end namespace ox

#endif
