// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#ifndef HARVEST_ENTITY_CMINERENTITY_H
#define HARVEST_ENTITY_CMINERENTITY_H

#include "CBuildingEntity.h"
#include "CEntityManager.h"

namespace harvest {
namespace entity {

class CMineralsEntity;

//! The mineral harvester. A spark powers one round of mining: it locks on to a free deposit in range,
//! draws its laser for 4.2 seconds and withdraws one mineral. Without deposits left it glows.
class CMineralGatherEntity : public CBuildingEntity
{
public:
    CMineralGatherEntity(float x, float y);
    virtual ~CMineralGatherEntity();

    virtual void writeEntityData(ox::io::IWriteFile* file);
    virtual void readEntityData(ox::io::IReadFile* file, int version);
    virtual void updateSprite(float frameDelta);
    virtual int updateLogic(float frameDelta);
    virtual void render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    virtual void renderGroundLayer(const ox::core::CPosition2d<float>& camera,
        const ox::core::CRect<int>& viewPort) {}
    virtual ox::video::ISpriteAnimationState* getCurrentDisplaySprite();

    //! Takes a spark when idle, else sends it back.
    virtual int onSpark(CSparkEntity* spark);
    virtual bool wantsSpark();

    virtual ox::core::CString<wchar_t> getInfoString();
    virtual ox::core::CString<wchar_t> getMiniStatString();

    virtual float getCollisionSize() const { return 15.0f; }

    bool isAbleToHarvest();
    bool hasMoreMinerals();
    //! Starts again when new deposits are in range after it ran out.
    void notifyMineralsAppeared();

private:
    //! A deposit in range that no other harvester claims. Marks the harvester out of minerals when
    //! there is none in range at all.
    CMineralsEntity* locateFreeMinerals();

    //! Powered by a spark and mining.
    bool Active;
    float MiningTime;
    //! Seconds before it looks for deposits again.
    float Cooldown;
    //! Counts down from a finished round; fades the building's colour.
    float IdleTimer;
    bool OutOfMinerals;
    int MineralsMined;
    ox::entity::SEntityReference Target;
    ox::video::IParticleState* MiningParticle;
    ox::video::ISpriteAnimationState* Sprite;
    SEnergyBeam Beam;
};

} // end namespace entity
} // end namespace harvest

#endif
