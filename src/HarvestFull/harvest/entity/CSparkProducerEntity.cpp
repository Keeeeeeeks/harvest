// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

// The object has an iostream static initializer, set up before the game headers' statics; the
// header that pulled it in is not identified yet. The world grid offset is initialized before
// ENERGY_PROGRESS_COLOR, so CWorld.h comes first.
#include <iostream>
#include "harvest/game/CWorld.h"
#include "harvest/entity/CSparkProducerEntity.h"
#include "harvest/entity/CEntityManager.h"
#include "harvest/entity/CSparkEntity.h"
#include "harvest/game/CLuaManager.h"
#include "harvest/settings/CSystemConfig.h"
#include "ox/algo/CRand.h"
#include "ox/io/CHelpIO.h"
#include "ox/video/ISpriteAnimationState.h"
#include "ox/video/ISpritePackage.h"

namespace harvest {
namespace entity {

CSparkProducerEntity::CSparkProducerEntity(float x, float y)
    : CBuildingEntity(g_nextEntityId++, 0, x, y), SparkTimer(1.5f), SparkIndex(0), Power(6000.0f),
      Expired(false)
{
    Sprite = gp_spritePackage->addNewAnimationState(ox::core::CString<char>("SparkProducer"));
    SparkTimer *= (ox::algo::CRand::rand() % 1000) * 0.001f;
}

CSparkProducerEntity::~CSparkProducerEntity()
{
    if (Sprite)
        Sprite->remove();
}

void CSparkProducerEntity::updateSprite(float frameDelta)
{
    if (Sprite)
        Sprite->update(frameDelta);
}

int CSparkProducerEntity::updateLogic(float frameDelta)
{
    if (Power > 0)
    {
        if (Power <= 0)
            Power = 0;
        SparkTimer -= frameDelta;
        Color = ox::video::SColor(0xffffffff);
    }
    else
    {
        SparkTimer -= frameDelta * 0.3f;
        Color = ox::video::SColor(0xff80a880);
        Expired = true;
    }

    if (SparkTimer <= 0)
    {
        SparkTimer += 1.5f;
        int target = findSparkTarget(0, SparkIndex);
        if (target > 0)
        {
            CSparkEntity* spark = new CSparkEntity(Position.X, Position.Y, target, Id);
            gp_entityManager->appendEntity(spark, 2);
            gp_entityManager->appendEntity(new CParticleEntity(Position.X, Position.Y + 1.0f, 0.0f, 0,
                "ProducerGlow"), 4);
            if (Sprite)
                Sprite->setFlag(1, false);
            if (game::gp_luaManager)
                game::gp_luaManager->hookEnergySparkCreated(spark->getId(), this);
        }
    }

    return 0;
}

int CSparkProducerEntity::onSpark(CSparkEntity* spark)
{
    return findSparkTarget(0, SparkIndex);
}

ox::core::CString<wchar_t> CSparkProducerEntity::getInfoString()
{
    if (Power <= 0)
        return settings::gp_systemConfig->getLocalizedText(L"entity:solarDecreased");
    return settings::gp_systemConfig->getLocalizedText(L"entity:solarFullPower");
}

ox::core::CString<wchar_t> CSparkProducerEntity::getMiniStatString()
{
    if (Power <= 0)
        return settings::gp_systemConfig->getLocalizedText(L"entity:solarExpired");
    return ox::core::CString<wchar_t>(L"");
}

void CSparkProducerEntity::writeEntityData(ox::io::IWriteFile* file)
{
    ox::io::CHelpIO::writeFloat(file, SparkTimer);
    ox::io::CHelpIO::writeInt(file, SparkIndex);
    Sprite->write(file);
    ox::io::CHelpIO::writeFloat(file, Power);
}

void CSparkProducerEntity::readEntityData(ox::io::IReadFile* file, int version)
{
    SparkTimer = ox::io::CHelpIO::readFloat(file);
    SparkIndex = ox::io::CHelpIO::readInt(file);

    if (Sprite)
    {
        if (version >= 6)
            Sprite->read(file);
        else
            Sprite->setFlag(1, true);
    }

    if (version >= 8)
        Power = ox::io::CHelpIO::readFloat(file);
}

void CSparkProducerEntity::render(const ox::core::CPosition2d<float>& camera,
    const ox::core::CRect<int>& viewPort)
{
    CEntity::renderSpriteFixed(camera, viewPort, Sprite);
}

ox::video::ISpriteAnimationState* CSparkProducerEntity::getCurrentDisplaySprite()
{
    return Sprite;
}

} // end namespace entity
} // end namespace harvest
