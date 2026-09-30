// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
#ifndef HARVEST_ENTITY_CDROPSHIPENTITY_H
#define HARVEST_ENTITY_CDROPSHIPENTITY_H
#include "CHarvestEntity.h"
#include "CEntityManager.h"
namespace harvest {
namespace entity {
//! The dropship circles the battlefield, fires missiles and guns, then lands and takes off.
//! Layout member names are ours.
class CDropshipEntity : public CEntity
{
public:
    CDropshipEntity(float x, float y, bool takeoff);
    virtual ~CDropshipEntity();
    virtual void writeEntityData(ox::io::IWriteFile* file);
    virtual void readEntityData(ox::io::IReadFile* file, int version);
    virtual void updateSprite(float frameDelta);
    virtual int updateLogic(float frameDelta);
    virtual void render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    virtual void renderGroundLayer(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    virtual ox::video::ISpriteAnimationState* getCurrentDisplaySprite();
    virtual int onSpark(CSparkEntity* spark);
    virtual bool wantsSpark();
    virtual ox::core::CString<wchar_t> getInfoString();
    virtual ox::core::CString<wchar_t> getMiniStatString();
    virtual float getCollisionSize() const { return 1.0f; }
    virtual int getSellValue() const { return 0; }
    void loadSprite(bool takeoff);
    bool turnTowardsTarget(float frameDelta);
    void locateNewAlienTarget();
    void locateNewAlienBulletTarget();
    void setDropshipState(int state);
    int getDropshipState();
private:
    ox::video::ISpriteAnimationState* Sprites[77];
    int SpriteIndex;
    float Angle;
    float Speed;
    float SpriteAngle;
    int State;
    float StateTime;
    float SoundPitch;
    ox::entity::SEntityReference Target;
    ox::core::CVector3d<float> TargetPosition;
    int LandingSteps;
    ox::entity::SEntityReference BulletTarget;
    float BulletCooldown;
    float MissileCooldown;
    float SalvoCooldown;
    int SalvoCount;
    float LaughCooldown;
    int LaughIndex;
    SEnergyBeam Beam;
    bool UnrecoveredFlag;
};
//! A fast shot leaving a sprite beam behind it; its impact damages aliens within 20 units.
class CDropshipBulletEntity : public CEntity
{
public:
    CDropshipBulletEntity(float damage, const ox::core::CVector3d<float>& start,
        const ox::core::CVector3d<float>& target);
    virtual ~CDropshipBulletEntity();
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
private:
    ox::core::CVector3d<float> TargetPosition;
    bool Hit;
    SEnergyBeam Beam;
    float LifeTime;
    float Damage;
};
} // end namespace entity
} // end namespace harvest
#endif
