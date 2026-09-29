// Copyright (C) 2002-2004 Nikolaus Gebhardt
// Adapted from Irrlicht 0.7 include/position2d.h (license: third_party/irrlicht-0.7/include/irrlicht.h).
// Recovered for Harvest's ox::core namespace; not the original source.

#ifndef OX_CORE_CPOSITION2D_H
#define OX_CORE_CPOSITION2D_H

namespace ox {
namespace core {

//! Simple class for holding 2d coordinates.
template <class T>
class CPosition2d
{
public:
    CPosition2d(T x, T y)
        : X(x), Y(y) {}

    CPosition2d()
        : X(0), Y(0) {}

    CPosition2d(const CPosition2d<T>& other)
        : X(other.X), Y(other.Y) {}

    bool operator==(const CPosition2d<T>& other) const
    {
        return X == other.X && Y == other.Y;
    }

    bool operator!=(const CPosition2d<T>& other) const
    {
        return X != other.X || Y != other.Y;
    }

    const CPosition2d<T>& operator+=(const CPosition2d<T>& other)
    {
        X += other.X;
        Y += other.Y;
        return *this;
    }

    const CPosition2d<T>& operator-=(const CPosition2d<T>& other)
    {
        X -= other.X;
        Y -= other.Y;
        return *this;
    }

    CPosition2d<T> operator-(const CPosition2d<T>& other) const
    {
        return CPosition2d<T>(X - other.X, Y - other.Y);
    }

    CPosition2d<T> operator+(const CPosition2d<T>& other) const
    {
        return CPosition2d<T>(X + other.X, Y + other.Y);
    }

    const CPosition2d<T>& operator=(const CPosition2d<T>& other)
    {
        X = other.X;
        Y = other.Y;
        return *this;
    }

    T X, Y;
};

} // end namespace core
} // end namespace ox

#endif
