// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: the scenario interface used by world initialization.

#ifndef HARVEST_GUI_CSTORYSCREEN_H
#define HARVEST_GUI_CSTORYSCREEN_H

#include "ox/core/CString.h"

namespace harvest {
namespace gui {

//! Campaign dialogue and credits, including display duration and alternating credit alignment.
class CDialogueItemInfo
{
public:
    CDialogueItemInfo(const wchar_t* name, const wchar_t* text, const char* portrait,
                      const char* sound, float time, bool alternate)
        : Name(name), Text(text), Portrait(portrait), Sound(sound), Time(time), Alternate(alternate) {}
    virtual ~CDialogueItemInfo() {}
    ox::core::CString<wchar_t> Name;
    ox::core::CString<wchar_t> Text;
    ox::core::CString<char> Portrait;
    ox::core::CString<char> Sound;
    float Time;
    bool Alternate;
};

} // end namespace gui
namespace game {

class CWorld;

class IScenario
{
public:
    virtual ~IScenario() {}
    virtual int getDoodadSeed() const = 0;
    virtual void applyInitialExpansions(CWorld* world) = 0;
};

} // end namespace game
} // end namespace harvest

#endif
