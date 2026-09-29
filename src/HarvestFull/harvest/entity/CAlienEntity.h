// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_ENTITY_CALIENENTITY_H
#define HARVEST_ENTITY_CALIENENTITY_H

#include "CHarvestEntity.h"

namespace harvest {
namespace entity {

//! Localization keys of the alien types; the last five are unused placeholders.
static const wchar_t* const ALIEN_KEY_NAMES[] =
{
    L"aliennames:default",
    L"aliennames:shielder",
    L"aliennames:tiny",
    L"aliennames:summoner",
    L"aliennames:looker",
    L"aliennames:miner",
    L"aliennames:stealer",
    L"aliennames:magneto",
    L"aliennames:mega",
    L"aliennames:asdf",
    L"aliennames:asdf",
    L"aliennames:asdf",
    L"aliennames:asdf",
    L"aliennames:asdf"
};

class CAlienEntity : public CEntity
{
public:
    CAlienEntity(float x, float y, int alienType);
    virtual ~CAlienEntity();

private:
    // The layout is not recovered yet; this keeps the Linux amd64 object size (0x2f0).
    unsigned char Unrecovered[0x2f0 - sizeof(CEntity)];
};

} // end namespace entity
} // end namespace harvest

#endif
