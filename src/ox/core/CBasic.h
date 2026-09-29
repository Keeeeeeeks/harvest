// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Declarations are added as the functions are recovered.

#ifndef OX_CORE_CBASIC_H
#define OX_CORE_CBASIC_H

#include "CString.h"

namespace ox {
namespace core {

//! Assorted helpers.
class CBasic
{
public:
    //! The current local time, formatted with strftime.
    static CString<char> getTimeString(char* format);
    //! A time_t value as local time, formatted with strftime.
    static CString<char> getTimeString(int time, char* format);
};

} // end namespace core
} // end namespace ox

#endif
