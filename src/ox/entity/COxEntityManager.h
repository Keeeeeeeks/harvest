// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// The virtual interface and member layout follow the Mac and Linux native builds.

#ifndef OX_ENTITY_COXENTITYMANAGER_H
#define OX_ENTITY_COXENTITYMANAGER_H

#include "../TArray.h"
#include "../core/CRect.h"
#include <list>

namespace ox {
namespace entity {

class COxEntity;
class ITestEntityFunction;
class ITestBestEntityFunction;

//! A reference to an entity by id that survives the entity's removal: the pointer is refreshed
//! from the id when the manager's entity lists have changed.
struct SEntityReference
{
    //! An empty reference.
    SEntityReference()
        : Entity(0), Id(-1), UpdateCounter(0)
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
    COxEntityManager(int layers);
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
    const std::list<COxEntity*>& getEntityList(int layer) const;
    const TArray<COxEntity*>& getRenderList();

protected:
    int NumLayers;
    std::list<COxEntity*>* EntityLists;
    std::list<COxEntity*>* PendingEntities;
    bool* ListChanged;
    TArray<COxEntity*> RenderList;
    int UpdateCounter;
    bool Updating;
};

} // end namespace entity
} // end namespace ox

#endif
