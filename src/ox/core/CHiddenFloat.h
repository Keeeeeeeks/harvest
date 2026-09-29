// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#ifndef OX_CORE_CHIDDENFLOAT_H
#define OX_CORE_CHIDDENFLOAT_H

namespace ox {
namespace io {
class IReadFile;
class IWriteFile;
} // end namespace io

namespace core {

//! A float kept in memory only xor-ed with a random key that changes from time to time, so it
//! cannot be found and edited by value.
class CHiddenFloat
{
public:
    CHiddenFloat();
    CHiddenFloat(float value);
    CHiddenFloat(const CHiddenFloat& other);
    virtual ~CHiddenFloat();

    void setValue(float value);
    float getValue();

    //! Adds delta and returns the new value.
    float modifyValue(float delta);
    //! Adds delta, clamps to [min, max] and returns the new value.
    float modifyValue(float delta, float min, float max);

    void write(io::IWriteFile* file);
    void read(io::IReadFile* file);

private:
    int* Key;
    int* Value;
};

} // end namespace core
} // end namespace ox

#endif
