// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#include "CHiddenInt.h"
#include "../algo/CRand.h"
#include "../io/CHelpIO.h"
// The object has an iostream static initializer; the header that pulled it in is not identified yet.
#include <iostream>

namespace ox {
namespace core {

CHiddenInt::~CHiddenInt()
{
    delete Key;
    delete Value;
}

CHiddenInt::CHiddenInt()
{
    Key = new int;
    Value = new int;
    *Key = algo::CRand::rand();
    *Value = *Key;
}

CHiddenInt::CHiddenInt(int value)
{
    Key = new int;
    Value = new int;
    *Key = algo::CRand::rand();
    setValue(value);
}

void CHiddenInt::setValue(int value)
{
    if (algo::CRand::rand() % 5 == 0)
        *Key = algo::CRand::rand();
    *Value = value ^ *Key;
}

CHiddenInt::CHiddenInt(const CHiddenInt& other)
{
    Key = new int;
    Value = new int;
    *Key = algo::CRand::rand();
    setValue(*other.Value ^ *other.Key);
}

int CHiddenInt::getValue()
{
    int value = *Value ^ *Key;
    setValue(value);
    return value;
}

int CHiddenInt::modifyValue(int delta)
{
    int value = (*Value ^ *Key) + delta;
    if (algo::CRand::rand() % 5 == 0)
        *Key = algo::CRand::rand();
    *Value = value ^ *Key;
    return value;
}

int CHiddenInt::modifyValue(int delta, int min, int max)
{
    int value = (*Value ^ *Key) + delta;
    value = value > max ? max : (value < min ? min : value);
    if (algo::CRand::rand() % 5 == 0)
        *Key = algo::CRand::rand();
    *Value = value ^ *Key;
    return value;
}

void CHiddenInt::write(io::IWriteFile* file)
{
    io::CHelpIO::writeInt(file, *Key);
    io::CHelpIO::writeInt(file, *Value);
}

void CHiddenInt::read(io::IReadFile* file)
{
    *Key = io::CHelpIO::readInt(file);
    *Value = io::CHelpIO::readInt(file);
}

} // end namespace core
} // end namespace ox
