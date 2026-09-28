// Recovered for Harvest; not the original source. Partial: only what recovered units use.

#ifndef OX_CORE_CSTRINGFUNCTIONS_H
#define OX_CORE_CSTRINGFUNCTIONS_H

#include <cstdlib>
#include "CString.h"
#include "../TArray.h"

namespace ox {
namespace core {

class CStringFunctions
{
public:
    //! Converts a wide string to the current locale's multibyte encoding.
    static CString<char> wideToAnsi(const CString<wchar_t>& str)
    {
        int size = str.size() + 1;
        char* ansi = new char[size];
        const wchar_t* src = str.c_str();
        wchar_t* wide = new wchar_t[size];
        for (int i = 0; i < size; ++i)
            wide[i] = src[i];
        wcstombs(ansi, wide, size);
        // scalar delete of an array, as in the Linux build
        delete wide;
        ansi[size - 1] = 0;
        CString<char> result = ansi;
        delete [] ansi;
        // an explicit copy: both builds copy result instead of constructing it in place
        return CString<char>(result);
    }

    static CString<wchar_t> ansiToWide(const CString<char>& str);
    static CString<wchar_t> millisecondsToWide(float milliseconds, bool showHours);
};

//! Splits str at every occurrence of separator into parts.
template <class T>
void splitString(TArray<CString<T> >& parts, const CString<T>& str, const CString<T>& separator);

} // end namespace core
} // end namespace ox

#endif
