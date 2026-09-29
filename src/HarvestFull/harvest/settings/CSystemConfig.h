// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_SETTINGS_CSYSTEMCONFIG_H
#define HARVEST_SETTINGS_CSYSTEMCONFIG_H

#include "ox/core/CString.h"

namespace harvest {
namespace settings {

class CSystemConfig
{
public:
    //! The text for a localization key, with the arguments filled in.
    ox::core::CString<wchar_t> getLocalizedText(const wchar_t* key, ...);
};

extern CSystemConfig* gp_systemConfig;

} // end namespace settings
} // end namespace harvest

#endif
