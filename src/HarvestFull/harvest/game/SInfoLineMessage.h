// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// The header path and field names are ours; the native structure owns four strings.
#ifndef HARVEST_GAME_SINFOLINEMESSAGE_H
#define HARVEST_GAME_SINFOLINEMESSAGE_H
#include "ox/core/CString.h"
namespace harvest {
namespace game {
struct SInfoLineMessage
{
    ox::core::CString<wchar_t> Name;
    ox::core::CString<wchar_t> Text;
    ox::core::CString<char> Portrait;
    ox::core::CString<char> Sound;
};
}
}
#endif
