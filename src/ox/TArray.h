// Recovered for Harvest; not the original source. The file name is inferred: the Mac debug map
// records no header for ox::TArray. Mac destroys a TArray through std::vector's destructor, so it
// adds no state of its own.

#ifndef OX_TARRAY_H
#define OX_TARRAY_H

#include <vector>

namespace ox {

template <class T>
class TArray : public std::vector<T>
{
};

} // end namespace ox

#endif
