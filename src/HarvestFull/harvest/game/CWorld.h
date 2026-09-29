// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_GAME_CWORLD_H
#define HARVEST_GAME_CWORLD_H

#include "ox/core/CPosition2d.h"
#include "ox/core/CRect.h"

namespace harvest {
namespace game {

// Every game unit initializes this constant at startup, so its initializer is not a constant
// expression to GCC; an inline function call reproduces that. The name and the function are ours.
inline float getWorldGridOffset()
{
    return 4096.0f;
}

//! Added to world coordinates to index the entity grid.
static const float WORLD_GRID_OFFSET = getWorldGridOffset();

//! The planet surface the game is played on.
class CWorld
{
public:
    //! Index of the planet: 0, 1 or 2.
    int getPlanet() const;

    //! The area the player may currently build in.
    const ox::core::CRect<float>& getActualGameFieldSize() const;

    bool hasWorldExpandedAtLeastOnce();

    bool mayPlaceObjectHere(const ox::core::CPosition2d<float>& position, bool building);
};

extern CWorld* gp_world;

} // end namespace game
} // end namespace harvest

#endif
