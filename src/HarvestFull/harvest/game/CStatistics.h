// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_GAME_CSTATISTICS_H
#define HARVEST_GAME_CSTATISTICS_H

namespace harvest {
namespace game {

//! Game statistics, for the score screens.
class CStatistics
{
public:
    //! Adds delta to a game statistic; 3 counts the buildings completed.
    void reportAlienStatChange(int statistic, int alienType, float amount);
    void modifyGameStatValue(int stat, int delta);
    //! Adds delta to a statistic of the current level; 2 counts the minerals mined.
    void modifyLevelStatValue(int stat, float delta);
};

extern CStatistics* gp_statistics;

} // end namespace game
} // end namespace harvest

#endif
