// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: other helpers in this header are not recovered yet.

#ifndef OX_ALGO_CARRAYFUNCTIONS_H
#define OX_ALGO_CARRAYFUNCTIONS_H

#include <iterator>

namespace ox {
namespace algo {

//! Returns the iterator moved by count positions, which may be negative.
template <class T>
T advanceIterator(T it, int count)
{
    std::advance(it, count);
    return it;
}

} // end namespace algo
} // end namespace ox

#endif
