// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: the virtual interface follows the vtables; members are not recovered yet.

#ifndef OX_ENTITY_COXENTITYMANAGER_H
#define OX_ENTITY_COXENTITYMANAGER_H

#include "../TArray.h"
#include "../core/CRect.h"

namespace ox {
namespace entity {

class COxEntity;
class ITestEntityFunction;
class ITestBestEntityFunction;

//! A reference to an entity by id that survives the entity's removal: the pointer is refreshed
//! from the id when the manager's entity lists have changed.
struct SEntityReference
{
    //! An empty reference; the id is set by its owner.
    SEntityReference()
        : Entity(0), UpdateCounter(0)
    {
    }

    COxEntity* Entity;
    int Id;
    //! The manager's update counter when Entity was last looked up.
    int UpdateCounter;
};

//! Holds entities in layers.
class COxEntityManager
{
public:
    virtual ~COxEntityManager();

    virtual void updateAllEntities(float frameDelta, const core::CRect<float>& visibleArea);
    virtual void appendEntity(COxEntity* entity, int layer);
    virtual COxEntity* locateEntity(int id, int layer);
    virtual COxEntity* findFirstEntity(int layer, ITestEntityFunction* test);
    virtual COxEntity* findBestEntity(int layer, ITestBestEntityFunction* test);
    virtual void findAllEntities(TArray<COxEntity*>& result, int layer, ITestEntityFunction* test);

    //! Refreshes a reference: drops it when its entity was killed, or looks it up again by id when
    //! the entity lists changed. An empty reference is looked up only when `locate` is set.
    void updateReference(SEntityReference& reference, int layer, bool locate);
    int getUpdateCounter() const;
};

} // end namespace entity
} // end namespace ox

#endif
