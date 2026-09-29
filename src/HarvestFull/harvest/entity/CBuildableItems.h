// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_ENTITY_CBUILDABLEITEMS_H
#define HARVEST_ENTITY_CBUILDABLEITEMS_H

namespace harvest {
namespace entity {

//! The buildings the player can build, standard and creative.
class CBuildableItems
{
public:
    int getIndexForEntityType(int entityType);
    //! The building id ("SPARKPRODUCER", a creative building's name) of an item.
    const char* getEntityId(int index);
};

extern CBuildableItems* gp_buildableItems;

} // end namespace entity
} // end namespace harvest

#endif
