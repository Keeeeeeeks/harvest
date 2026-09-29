// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#include "CHiddenFloat.h"
#include "../algo/CRand.h"
#include "../io/CHelpIO.h"
// The object has an iostream static initializer; the header that pulled it in is not identified yet.
#include <iostream>

namespace ox {
namespace core {

CHiddenFloat::~CHiddenFloat()
{
    delete Key;
    delete Value;
}

CHiddenFloat::CHiddenFloat()
{
    Key = new int;
    Value = new int;
    *Key = algo::CRand::rand();
    *Value = *Key;
}

CHiddenFloat::CHiddenFloat(float value)
{
    Key = new int;
    Value = new int;
    *Key = algo::CRand::rand();
    setValue(value);
}

void CHiddenFloat::setValue(float value)
{
    if (algo::CRand::rand() % 5 == 0)
        *Key = algo::CRand::rand();
    *Value = *(int*)&value ^ *Key;
}

CHiddenFloat::CHiddenFloat(const CHiddenFloat& other)
{
    Key = new int;
    Value = new int;
    *Key = algo::CRand::rand();
    setValue(((CHiddenFloat&)other).getValue());
}

float CHiddenFloat::getValue()
{
    int value = *Value ^ *Key;
    return *(float*)&value;
}

float CHiddenFloat::modifyValue(float delta)
{
    float value = getValue() + delta;
    if (algo::CRand::rand() % 5 == 0)
        *Key = algo::CRand::rand();
    *Value = *(int*)&value ^ *Key;
    return value;
}

float CHiddenFloat::modifyValue(float delta, float min, float max)
{
    float value = getValue() + delta;
    value = value > max ? max : (value < min ? min : value);
    if (algo::CRand::rand() % 5 == 0)
        *Key = algo::CRand::rand();
    *Value = *(int*)&value ^ *Key;
    return value;
}

void CHiddenFloat::write(io::IWriteFile* file)
{
    io::CHelpIO::writeInt(file, *Key);
    io::CHelpIO::writeInt(file, *Value);
}

void CHiddenFloat::read(io::IReadFile* file)
{
    *Key = io::CHelpIO::readInt(file);
    *Value = io::CHelpIO::readInt(file);
}

} // end namespace core
} // end namespace ox
