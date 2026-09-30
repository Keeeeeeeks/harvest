// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

// The object has an iostream static initializer, set up before the game headers' statics; the
// header that pulled it in is not identified yet. The world grid offset is initialized before
// ENERGY_PROGRESS_COLOR, so CWorld.h comes first.
#include <iostream>
#include "harvest/game/CWorld.h"
#include "harvest/entity/CSparkEntity.h"
#include "harvest/entity/CEntityManager.h"
#include "ox/io/CHelpIO.h"
#include "ox/video/ISpriteAnimationState.h"
#include "ox/video/ISpritePackage.h"

namespace harvest {
namespace entity {

CSparkEntity::CSparkEntity(float x, float y, int targetId, int sourceId)
    : CEntity(g_nextEntityId++, 2, x, y), SourceId(sourceId), GridPosition(-1)
{
    Target.Id = targetId;
    Position.Z = 18.0f;

    Sprite = gp_spritePackage->addNewAnimationState(ox::core::CString<char>("Spark"));
    Target.Entity = findEntity(targetId);
}

CSparkEntity::~CSparkEntity()
{
    if (Sprite)
        Sprite->remove();
}

void CSparkEntity::updateSprite(float frameDelta)
{
    if (Sprite)
        Sprite->update(frameDelta);
}

int CSparkEntity::updateLogic(float frameDelta)
{
    if (Target.Id <= 0)
        return 1;

    if (Target.Entity)
    {
        if (gp_entityManager->hasBuildingListChanged() && GridPosition >= 0)
        {
            Target.Entity = gp_entityManager->locateEntityInGrid(Target.Id, GridPosition, 0);
            if (Target.Entity)
                Target.UpdateCounter = gp_entityManager->getUpdateCounter();
        }
        else
            gp_entityManager->updateReference(Target, 0, true);
    }
    else
    {
        gp_entityManager->updateReference(Target, 0, true);
        if (Target.Entity)
            GridPosition = gp_entityManager->calculateGridPosition(Target.Entity->getPosition());
    }

    if (!Target.Entity)
    {
        gp_entityManager->appendEntity(new CParticleEntity(Position.X, Position.Y, 1.0f, 0, "Poof"), 4);
        return 1;
    }

    ox::core::CVector3d<float> targetPosition = Target.Entity->getPosition();
    ox::core::CVector3d<float> move(targetPosition.X - Position.X, targetPosition.Y - Position.Y,
        ((CEntity*)Target.Entity)->getSparkHeight() - Position.Z);
    double distance = move.getLengthSQ();
    move.normalize();
    move *= frameDelta * 100.0f;
    Position += move;

    if (move.getLengthSQ() >= distance)
    {
        int next = ((CEntity*)Target.Entity)->onSpark(this);
        if (!next)
            return 1;

        if (next == -1)
        {
            int source = SourceId;
            SourceId = next;
            Target.Id = source;
        }
        else
        {
            SourceId = Target.Id;
            Target.Id = next;
        }

        Target.Entity = findEntity(Target.Id);
        Target.UpdateCounter = gp_entityManager->getUpdateCounter();
        if (Target.Entity)
            GridPosition = gp_entityManager->calculateGridPosition(Target.Entity->getPosition());
    }

    return 0;
}

void CSparkEntity::render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort)
{
    float y = Position.Y;
    Position.Y -= Position.Z;
    CEntity::renderSprite(camera, viewPort, Sprite);
    Position.Y = y;
}

int CSparkEntity::getSourceId() const
{
    return SourceId;
}

ox::entity::COxEntity* CSparkEntity::findEntity(int id)
{
    return gp_entityManager->locateEntity(id, 0);
}

void CSparkEntity::writeEntityData(ox::io::IWriteFile* file)
{
    ox::io::CHelpIO::writeInt(file, Target.Id);
    ox::io::CHelpIO::writeInt(file, SourceId);
    ox::io::CHelpIO::writeFloat(file, Position.Z);
}

void CSparkEntity::readEntityData(ox::io::IReadFile* file, int version)
{
    Target.Id = ox::io::CHelpIO::readInt(file);
    SourceId = ox::io::CHelpIO::readInt(file);
    if (version >= 10)
        Position.Z = ox::io::CHelpIO::readFloat(file);
}

} // end namespace entity
} // end namespace harvest
