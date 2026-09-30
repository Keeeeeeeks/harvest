// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#ifndef HARVEST_ENTITY_CSPARKMOVERENTITY_H
#define HARVEST_ENTITY_CSPARKMOVERENTITY_H

#include "CBuildingEntity.h"
#include "CEntityManager.h"

namespace harvest {
namespace entity {

//! Whether sparks dying at an overheated link use the large "SparkDeath" particle instead of "Poof".
extern bool g_useLargeSparkDeathParticle;

//! The energy link: passes sparks on to buildings in range, or along a waypoint to another link or a
//! construction site. Each spark heats it; overheated, it lets sparks die. A charged link explodes.
class CSparkMoverEntity : public CBuildingEntity
{
public:
    CSparkMoverEntity(float x, float y);
    virtual ~CSparkMoverEntity();

    virtual void writeEntityData(ox::io::IWriteFile* file);
    virtual void readEntityData(ox::io::IReadFile* file, int version);
    virtual void updateSprite(float frameDelta);
    virtual int updateLogic(float frameDelta);
    virtual void render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    //! Draws the waypoint link.
    virtual void renderGroundLayer(const ox::core::CPosition2d<float>& camera,
        const ox::core::CRect<int>& viewPort);
    virtual ox::video::ISpriteAnimationState* getCurrentDisplaySprite();

    virtual int onSpark(CSparkEntity* spark);
    virtual bool wantsSpark() { return true; }
    //! Takes sparks from anything but the link it forwards to.
    virtual bool acceptsSparkFrom(int id);

    virtual ox::core::CString<wchar_t> getInfoString();
    virtual ox::core::CString<wchar_t> getMiniStatString();

    virtual float getCollisionSize() const { return 8.0f; }

    virtual void handleRightClickAction(const ox::core::CPosition2d<float>& position);
    //! A charging link calls the free links in range; otherwise it clears its waypoint chain.
    virtual void handleDoubleClickSelection();
    virtual bool handleSelectionDraggedToEntity(CEntity* entity);
    virtual void handleSelectionDraggedToNothing();

    bool isWaypointed();
    //! Whether an alien has latched on and steals the sparks.
    bool isAlienWaypointed();
    bool isOverheated();
    //! Forwards sparks to an entity; alien marks an alien stealing them.
    void setSparkTargetId(int id, bool alien);
    int getWaypointId();
    //! Starts charging towards an explosion.
    void startCharging();

private:
    //! Refreshes the waypoint; a waypointed construction site that became a building ends it.
    void updateTargetReference();
    //! Clears the waypoints of this link and of the links it forwards to.
    void clearWaypointsForward();
    //! The next target in range that wants a spark and is not a link, or is a charging link.
    int selectRequiredSparkTarget();

    //! Cycles through the targets in range.
    int SparkIndex;
    ox::entity::SEntityReference Waypoint;
    bool AlienWaypoint;
    //! Whether the waypoint was last seen as a construction site.
    bool WaypointToConstruction;
    ox::core::CVector3d<float> WaypointPosition;
    ox::TArray<ox::entity::COxEntity*> SparkTargets;
    //! Rises by 1/30 per spark and falls by a unit per second; above 1 sparks die.
    float Heat;
    //! Seconds before a waypointed link sends a spark to a target in range again.
    float Cooldown;
    ox::video::ISpriteAnimationState* Sprite;
    SEnergyBeam LinkBeam;
    SEnergyBeam AlienBeam;
    bool Charging;
    //! Sparks taken while charging; the thirtieth sets off the explosion.
    int ChargeSparks;
    ox::video::IParticleState* ChargeParticle;
    //! Seconds until PeakHeat is sampled again.
    float HeatSampleTimer;
    float PeakHeat;
};

} // end namespace entity
} // end namespace harvest

#endif
