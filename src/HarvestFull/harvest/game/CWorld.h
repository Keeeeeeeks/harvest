// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_GAME_CWORLD_H
#define HARVEST_GAME_CWORLD_H

#include "ox/core/CHiddenInt.h"
#include "ox/core/CPosition2d.h"
#include "ox/core/CRect.h"
#include "ox/core/CVector2d.h"
#include "ox/core/CVector3d.h"

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
    virtual ~CWorld();

    //! Index of the planet: 0, 1 or 2.
    int getPlanet() const;
    int getGameMode() const;

    //! The area the player may currently build in.
    const ox::core::CRect<float>& getActualGameFieldSize() const;

    bool hasWorldExpandedAtLeastOnce();

    bool mayPlaceObjectHere(const ox::core::CPosition2d<float>& position, bool building);
    bool mayMoveHere(const ox::core::CPosition2d<float>& position);

    //! Adds the wind at a position over the frame to speed.
    void applyWind(const ox::core::CVector3d<float>& position, ox::core::CVector2d<float>& speed,
        float frameDelta) const;

private:
    // The layout is not recovered yet; this keeps Planet at its Linux amd64 offset (0x4c).
    unsigned char Unrecovered[0x4c - sizeof(void*)];

public:
    //! Read directly by particles, which only feel wind on planet 1; see getPlanet.
    int Planet;
};

extern CWorld* gp_world;
//! The player's minerals, and a negated copy that catches tampering.
extern ox::core::CHiddenInt* gp_mineralAmount;
extern ox::core::CHiddenInt* gp_negatedMineralAmount;

} // end namespace game
} // end namespace harvest

#endif
