// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// The file name is inferred.

#ifndef OX_ENTITY_ITESTENTITYFUNCTION_H
#define OX_ENTITY_ITESTENTITYFUNCTION_H

namespace ox {
namespace entity {

class COxEntity;

//! A predicate an entity manager searches entities with. It has no virtual destructor.
class ITestEntityFunction
{
public:
    virtual bool testEntity(COxEntity* entity) = 0;
};

} // end namespace entity
} // end namespace ox

#endif
