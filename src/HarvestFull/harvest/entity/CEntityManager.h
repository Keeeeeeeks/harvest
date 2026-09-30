// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Member names for the cached searches are ours; public method names follow the Mac symbols.

#ifndef HARVEST_ENTITY_CENTITYMANAGER_H
#define HARVEST_ENTITY_CENTITYMANAGER_H

#include "ox/core/CPosition2d.h"
#include "ox/core/CVector3d.h"
#include "ox/entity/COxEntityManager.h"
#include "ox/video/ISpriteAnimationState.h"
#include "ox/video/SColor.h"
#include <list>

namespace harvest {
namespace entity {

class CEntity;
class CAlienEntity;
class CMineralsEntity;
class CFindClickableBuilding;
class CFindRandomEntityInRange;
class CFindShootableAlienInRange;
class CFindBuildingInRange;
class CFindRangeLineBuildings;

//! A beam drawn from Start to End with sprites at both ends, above all entities.
struct SEnergyBeam
{
    SEnergyBeam(float width)
        : Width(width), Color(0xffffffff), Beam(0), StartSprite(0), StartScale(1.0f), EndSprite(0),
          EndScale(1.0f)
    {
    }

    virtual ~SEnergyBeam()
    {
        if (Beam)
            Beam->remove();
        if (StartSprite)
            StartSprite->remove();
        if (EndSprite)
            EndSprite->remove();
    }

    float Width;
    ox::core::CPosition2d<float> Start;
    ox::core::CPosition2d<float> End;
    ox::video::SColor Color;
    //! Stretched from Start to End.
    ox::video::ISpriteAnimationState* Beam;
    ox::video::ISpriteAnimationState* StartSprite;
    float StartScale;
    ox::video::ISpriteAnimationState* EndSprite;
    float EndScale;
};

//! The game's entity manager. It also sorts entities into a grid of cells for range searches.
class CEntityManager : public ox::entity::COxEntityManager
{
public:
    CEntityManager();
    virtual ~CEntityManager();
    void update(float frameDelta, const ox::core::CRect<float>& visibleArea);
    bool readEntities(ox::io::IReadFile* file, int version);
    bool writeEntities(ox::io::IWriteFile* file);
    bool hasBuildingListChanged() const;
    int getNumBuildings() const;
    int getNumAliens() const;
    const ox::core::CRect<float>& getBuildingsBoundingBox() const;
    void renderEntities(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    bool isBuildingPlacementOk(ox::core::CPosition2d<float>& position, float collisionSize);
    void findAllGridEntitiesInRange(ox::TArray<ox::entity::COxEntity*>& result, int layer, CFindRandomEntityInRange* test);
    void getAllEntitiesInRange(ox::TArray<ox::entity::COxEntity*>& result, const ox::core::CPosition2d<float>& position, float squaredRange, int layer, int entityType);
    CEntity* findRandomAlienInRange(const ox::core::CPosition2d<float>& position, float squaredRange, float minimumSquaredRange);
    CEntity* findAnyEntityInRange(const ox::core::CPosition2d<float>& position, float squaredRange, int layer, int entityType);
    float getRangeSearchResultDistance();
    void getAllRangeLineBuildings(ox::TArray<ox::entity::COxEntity*>& result, const ox::core::CPosition2d<float>& position, int entityType);
    CEntity* updateClickableReference(int id, CEntity* entity);
    CEntity* addBuilding(int type, float x, float y);
    CMineralsEntity* addMinerals(int size, float x, float y);
    CAlienEntity* addAlien(int type, float x, float y);
    CEntity* findBuildingInRange(const ox::core::CPosition2d<float>& position, float squaredRange);

    //! The grid cell of a world coordinate, clamped to the grid.
    int calculateGridCoordinateClamp(float coordinate);
    void updateGridEntity(CEntity* entity, const ox::core::CVector3d<float>& oldPosition, int searchLayer);
    void addGridEntity(CEntity* entity, int searchLayer);
    void removeGridEntity(CEntity* entity, const ox::core::CVector3d<float>& position, int searchLayer);
    //! The grid cell index of a position.
    int calculateGridPosition(const ox::core::CVector3d<float>& position);
    CEntity* locateEntityInGrid(int id, int gridPosition, int layer);
    CEntity* findRandomEntityInRange(const ox::core::CPosition2d<float>& position, float squaredRange, int layer,
        int entityType);
    //! Queues a beam to be drawn above all entities this frame.
    void insertTopLevelEnergyBeam(SEnergyBeam* beam);
    void renderEnergyBeam(SEnergyBeam* beam, const ox::core::CPosition2d<float>& camera,
        const ox::core::CRect<int>& viewPort);
    //! The entity the player clicks at a world position.
    CEntity* findClickableEntity(const ox::core::CPosition2d<float>& position);

private:
    ox::TArray<SEnergyBeam*> TopLevelEnergyBeams;
    CFindClickableBuilding* ClickableSearch;
    CFindRandomEntityInRange* RandomSearch;
    CFindShootableAlienInRange* ShootableSearch;
    CFindBuildingInRange* BuildingSearch;
    CFindRangeLineBuildings* RangeLineSearch;
    bool BuildingsChanged;
    int NumBuildings;
    unsigned int NumEntities;
    ox::core::CRect<float> BuildingsBoundingBox;

public:
    //! Five search layers of grid cells, each indexed y * 18 + x.
    std::list<CEntity*> Grid[5][18 * 18];
};

extern CEntityManager* gp_entityManager;
//! The id the next spark gets.
extern int g_nextEntityId;

} // end namespace entity
} // end namespace harvest

#endif
