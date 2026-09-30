// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: the scenario interface used by world initialization.

#ifndef HARVEST_GUI_CSTORYSCREEN_H
#define HARVEST_GUI_CSTORYSCREEN_H

namespace harvest {
namespace game {

class CWorld;

class IScenario
{
public:
    virtual ~IScenario();
    virtual int getDoodadSeed() const = 0;
    virtual void applyInitialExpansions(CWorld* world) = 0;
};

} // end namespace game
} // end namespace harvest

#endif
