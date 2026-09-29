// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#ifndef OX_CORE_CHIDDENINT_H
#define OX_CORE_CHIDDENINT_H

namespace ox {
namespace io {
class IReadFile;
class IWriteFile;
} // end namespace io

namespace core {

//! An int kept in memory only xor-ed with a random key that changes from time to time, so it
//! cannot be found and edited by value.
class CHiddenInt
{
public:
    CHiddenInt();
    CHiddenInt(int value);
    CHiddenInt(const CHiddenInt& other);
    virtual ~CHiddenInt();

    void setValue(int value);
    //! Reads the value and stores it again, which may change the key.
    int getValue();

    //! Adds delta and returns the new value.
    int modifyValue(int delta);
    //! Adds delta, clamps to [min, max] and returns the new value.
    int modifyValue(int delta, int min, int max);

    void write(io::IWriteFile* file);
    void read(io::IReadFile* file);

private:
    int* Key;
    int* Value;
};

} // end namespace core
} // end namespace ox

#endif
