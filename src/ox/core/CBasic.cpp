// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#include "CBasic.h"
#include <ctime>
// The object has an iostream static initializer; the header that pulled it in is not identified yet.
#include <iostream>

namespace ox {
namespace core {

CString<char> CBasic::getTimeString(char* format)
{
    return getTimeString(time(0), format);
}

CString<char> CBasic::getTimeString(int time, char* format)
{
    time_t t = time;
    char buffer[64];
    strftime(buffer, 63, format, localtime(&t));
    return CString<char>(buffer);
}

} // end namespace core
} // end namespace ox
