// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// The file name is inferred.

#ifndef OX_ALGO_SPOINTERSORTFUNCTOR_H
#define OX_ALGO_SPOINTERSORTFUNCTOR_H

namespace ox {
namespace algo {

//! Orders the pointed-to objects, rather than their addresses.
template <class T>
struct SPointerSortFunctor
{
    bool operator()(T a, T b) const { return *a < *b; }
};

} // end namespace algo
} // end namespace ox

#endif
