// Copyright (C) 2002-2004 Nikolaus Gebhardt
// Adapted from Irrlicht 0.7 include/vector2d.h (license: third_party/irrlicht-0.7/include/irrlicht.h).
// Recovered for Harvest's ox::core namespace; not the original source. Partial: the geometry
// helpers other than getLength and normalize are not recovered yet.

#ifndef OX_CORE_CVECTOR2D_H
#define OX_CORE_CVECTOR2D_H

#include <math.h>

namespace ox {
namespace core {

//! 2d vector template class with lots of operators and methods.
template <class T>
class CVector2d
{
public:
    CVector2d() : X(0), Y(0) {}
    CVector2d(T nx, T ny) : X(nx), Y(ny) {}
    CVector2d(const CVector2d<T>& other) : X(other.X), Y(other.Y) {}

    // operators

    CVector2d<T> operator-() const { return CVector2d<T>(-X, -Y); }

    CVector2d<T>& operator=(const CVector2d<T>& other) { X = other.X; Y = other.Y; return *this; }

    CVector2d<T> operator+(const CVector2d<T>& other) const { return CVector2d<T>(X + other.X, Y + other.Y); }
    CVector2d<T>& operator+=(const CVector2d<T>& other) { X += other.X; Y += other.Y; return *this; }

    CVector2d<T> operator-(const CVector2d<T>& other) const { return CVector2d<T>(X - other.X, Y - other.Y); }
    CVector2d<T>& operator-=(const CVector2d<T>& other) { X -= other.X; Y -= other.Y; return *this; }

    CVector2d<T> operator*(const CVector2d<T>& other) const { return CVector2d<T>(X * other.X, Y * other.Y); }
    CVector2d<T>& operator*=(const CVector2d<T>& other) { X *= other.X; Y *= other.Y; return *this; }
    CVector2d<T> operator*(const T v) const { return CVector2d<T>(X * v, Y * v); }
    CVector2d<T>& operator*=(const T v) { X *= v; Y *= v; return *this; }

    CVector2d<T> operator/(const CVector2d<T>& other) const { return CVector2d<T>(X / other.X, Y / other.Y); }
    CVector2d<T>& operator/=(const CVector2d<T>& other) { X /= other.X; Y /= other.Y; return *this; }
    CVector2d<T> operator/(const T v) const { return CVector2d<T>(X / v, Y / v); }
    CVector2d<T>& operator/=(const T v) { X /= v; Y /= v; return *this; }

    bool operator==(const CVector2d<T>& other) const { return other.X == X && other.Y == Y; }
    bool operator!=(const CVector2d<T>& other) const { return other.X != X || other.Y != Y; }

    // functions

    void set(const T& nx, const T& ny) { X = nx; Y = ny; }
    void set(const CVector2d<T>& p) { X = p.X; Y = p.Y; }

    //! Returns the length of the vector
    double getLength() const { return sqrt(X * X + Y * Y); }

    //! Returns the clockwise angle in degrees, in the range 0..360.
    double getAngle() const
    {
        if (Y == 0.0) return X < 0.0 ? 180.0 : 0.0;
        else if (X == 0.0) return Y < 0.0 ? 90.0 : 270.0;
        double angle = Y / sqrt(X * X + Y * Y);
        angle = atan(sqrt(1 - angle * angle) / angle) * 57.295780181884766;
        if (X > 0.0 && Y > 0.0) return angle + 270;
        else if (X > 0.0 && Y < 0.0) return angle + 90;
        else if (X < 0.0 && Y < 0.0) return 90 - angle;
        else if (X < 0.0 && Y > 0.0) return 270 - angle;
        return angle;
    }

    //! Normalizes the vector, leaving a zero vector unchanged.
    CVector2d<T>& normalize()
    {
        T length = (T)getLength();
        if (length == 0)
            return *this;
        length = (T)1.0 / length;
        X *= length;
        Y *= length;
        return *this;
    }

    //! Rotates around center; the original helper takes degrees.
    void rotateBy(double degrees, const CVector2d<T>& center = CVector2d<T>())
    {
        degrees *= 0.017453290522098541;
        T cs = (T)cos(degrees);
        T sn = (T)sin(degrees);
        X -= center.X;
        Y -= center.Y;
        set(X * cs - Y * sn, X * sn + Y * cs);
        X += center.X;
        Y += center.Y;
    }

    T X, Y;
};

} // end namespace core
} // end namespace ox

#endif
