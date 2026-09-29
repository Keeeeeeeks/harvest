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

//! Holds entities in layers.
class COxEntityManager
{
public:
    virtual ~COxEntityManager();

    virtual void updateAllEntities(float frameDelta, const core::CRect<float>& visibleArea);
    virtual void appendEntity(COxEntity* entity, int layer);
    virtual COxEntity* locateEntity(int layer, int id);
    virtual COxEntity* findFirstEntity(int layer, ITestEntityFunction* test);
    virtual COxEntity* findBestEntity(int layer, ITestBestEntityFunction* test);
    virtual void findAllEntities(TArray<COxEntity*>& result, int layer, ITestEntityFunction* test);
};

} // end namespace entity
} // end namespace ox

#endif
