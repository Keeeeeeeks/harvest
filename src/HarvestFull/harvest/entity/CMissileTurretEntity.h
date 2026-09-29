// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_ENTITY_CMISSILETURRETENTITY_H
#define HARVEST_ENTITY_CMISSILETURRETENTITY_H

#include "CBuildingEntity.h"
#include "CEntityManager.h"
#include <vector>

namespace harvest {
namespace entity {

class CAlienEntity;

//! A turret that fires missiles; entity types 8, 13 and 14.
class CMissileTurretEntity : public CBuildingEntity
{
public:
    CMissileTurretEntity(int type, float x, float y);
    virtual ~CMissileTurretEntity();

    virtual void writeEntityData(ox::io::IWriteFile* file);
    virtual void readEntityData(ox::io::IReadFile* file, int version);
    virtual void updateSprite(float frameDelta);
    virtual int updateLogic(float frameDelta);
    virtual void render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    virtual void renderGroundLayer(const ox::core::CPosition2d<float>& camera,
        const ox::core::CRect<int>& viewPort) {}
    virtual ox::video::ISpriteAnimationState* getCurrentDisplaySprite();
    virtual int onSpark(CSparkEntity* spark);
    virtual bool wantsSpark();
    virtual ox::core::CString<wchar_t> getInfoString();
    virtual ox::core::CString<wchar_t> getMiniStatString();
    virtual ox::core::CString<wchar_t> getOperatorString();
    virtual float getCollisionSize() const { return 18.0f; }
    int getRequiredSparks();
    CAlienEntity* findPriorityAlien();
    void addKillCount(int kills) { Kills += kills; }

    static CAlienEntity* findPriorityAlien(const ox::core::CVector3d<float>& position, int prioritySet,
        float squaredRange, float minimumSquaredRange, ox::core::CVector3d<float>* nearest);

private:
    int Sparks;
    float ReloadTime;
    int SpriteIndex;
    int Kills;
    int BurstCount;
    float BurstTime;
    float IdleTime;
    bool Launching;
    ox::core::CPosition2d<float> TargetPosition;
    int TargetId;
    ox::video::ISpriteAnimationState* Sprites[3];
};

//! An alien and its squared distance from a missile blast.
struct SAlienDistancePair
{
    CAlienEntity* Alien;
    float SquaredDistance;
};

struct SAlienDistanceSorter
{
    bool operator()(const SAlienDistancePair& a, const SAlienDistancePair& b) const
    {
        return a.SquaredDistance < b.SquaredDistance;
    }
};

//! A missile fired by a missile turret.
class CMissileEntity : public CEntity
{
public:
    CMissileEntity(float x, float y, const ox::core::CPosition2d<float>&, int, int, int);
    virtual ~CMissileEntity();

    virtual void writeEntityData(ox::io::IWriteFile* file);
    virtual void readEntityData(ox::io::IReadFile* file, int version);
    virtual void updateSprite(float frameDelta) {}
    virtual int updateLogic(float frameDelta);
    virtual bool addToRenderList(const ox::core::CRect<float>& visibleArea);
    virtual void render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    virtual void renderGroundLayer(const ox::core::CPosition2d<float>& camera,
        const ox::core::CRect<int>& viewPort) {}
    virtual int onSpark(CSparkEntity* spark) { return 0; }
    virtual bool wantsSpark() { return false; }
    void initializeMissileType();
    void setSpeed(float x, float y, float z);
    void disableRetargeting() { Retargeting = false; }
    void updateSpeed(const ox::core::CVector3d<float>& direction, float frameDelta, float acceleration,
        float speedLimit, float drag, float verticalDrag);

private:
    ox::core::CVector3d<float> TargetPosition;
    ox::core::CVector3d<float> Speed;
    int OwnerId;
    int MissileType;
    float LifeTime;
    bool Retargeting;
    ox::entity::SEntityReference Target;
    ox::video::IParticleState* Particle;
};

//! A blast fired by a tempest turret.
class CTempestBlastEntity : public CEntity
{
public:
    CTempestBlastEntity(const ox::core::CVector3d<float>& position, const ox::core::CVector3d<float>&, int, int);
    virtual ~CTempestBlastEntity();

    virtual void writeEntityData(ox::io::IWriteFile* file);
    virtual void readEntityData(ox::io::IReadFile* file, int version);
    virtual void updateSprite(float frameDelta);
    virtual int updateLogic(float frameDelta);
    virtual bool addToRenderList(const ox::core::CRect<float>& visibleArea);
    virtual void render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    virtual void renderGroundLayer(const ox::core::CPosition2d<float>& camera,
        const ox::core::CRect<int>& viewPort) {}
    virtual int onSpark(CSparkEntity* spark) { return 0; }
    virtual bool wantsSpark() { return false; }
    void deleteBeams();
    void createBeams(const ox::core::CVector3d<float>& start, const ox::core::CVector3d<float>& end, int phase);

private:
    ox::core::CVector3d<float> TargetPosition;
    int TargetId;
    int OwnerId;
    float BeamTime;
    float LifeTime;
    bool FirstUpdate;
    ox::video::ISpriteAnimationState* Sprite;
    std::vector<SEnergyBeam*> Beams;
};

} // end namespace entity
} // end namespace harvest

#endif
