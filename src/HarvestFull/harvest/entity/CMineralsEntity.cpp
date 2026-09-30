// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

// The object has an iostream static initializer, set up before the game headers' statics; the
// header that pulled it in is not identified yet. The world grid offset is initialized before
// ENERGY_PROGRESS_COLOR, so CWorld.h comes first.
#include <iostream>
#include "harvest/game/CWorld.h"
#include "harvest/entity/CMineralsEntity.h"
#include "harvest/entity/CEntityManager.h"
#include "harvest/settings/CSystemConfig.h"
#include "ox/algo/CRand.h"
#include "ox/core/CBasic.h"
#include "ox/io/CHelpIO.h"
#include "ox/video/ISpriteAnimationState.h"
#include "ox/video/ISpritePackage.h"

namespace harvest {
namespace entity {

CMineralsEntity::CMineralsEntity(float x, float y, int size)
    : CEntity(g_nextEntityId++, 5, x, y), Sprite(0), Minerals(40), Size(size), HoggerId(-1)
{
    if (size == 2)
        Minerals.setValue(70);
    else if (size == 1)
        Minerals.setValue(25);

    setSprite();
}

CMineralsEntity::~CMineralsEntity()
{
    if (Sprite)
        Sprite->remove();
}

void CMineralsEntity::setSprite()
{
    if (Sprite)
        Sprite->remove();

    const char* const SPRITE_NAMES[3][3] =
    {
        { "Minerals", "MineralsSmall", "MineralsLarge" },
        { "Minerals2", "MineralsSmall2", "MineralsLarge2" },
        { "Minerals3", "MineralsSmall3", "MineralsLarge3" }
    };

    Sprite = gp_spritePackage->addNewAnimationState(
        ox::core::CString<char>(SPRITE_NAMES[game::gp_world->getPlanet()][Size]));
}

void CMineralsEntity::updateSprite(float frameDelta)
{
    if (Sprite)
        Sprite->update(frameDelta);
}

int CMineralsEntity::updateLogic(float frameDelta)
{
    return Minerals.getValue() <= 0;
}

int CMineralsEntity::onSpark(CSparkEntity* spark)
{
    return 0;
}

bool CMineralsEntity::wantsSpark()
{
    return false;
}

void CMineralsEntity::render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort)
{
    CEntity::renderSpriteFixed(camera, viewPort, Sprite);
}

ox::video::ISpriteAnimationState* CMineralsEntity::getCurrentDisplaySprite()
{
    return Sprite;
}

int CMineralsEntity::withdrawAmount(int amount)
{
    if (Minerals.getValue() <= amount)
    {
        amount = Minerals.getValue();
        Minerals.setValue(0);
    }
    else
        Minerals.modifyValue(-amount);

    return amount;
}

void CMineralsEntity::setHogStatus(int hoggerId)
{
    HoggerId = hoggerId;
}

int CMineralsEntity::getHoggerId()
{
    return HoggerId;
}

void CMineralsEntity::setRemainingMinerals(int amount)
{
    Minerals.setValue(amount);

    int size = 1;
    if (amount >= 30)
    {
        size = 0;
        if (amount >= 61)
            size = 2;
    }

    if (Size != size)
    {
        Size = size;
        setSprite();
    }
}

void CMineralsEntity::writeEntityData(ox::io::IWriteFile* file)
{
    Minerals.write(file);
    ox::io::CHelpIO::writeInt(file, Size);
    ox::io::CHelpIO::writeInt(file, HoggerId);
}

void CMineralsEntity::readEntityData(ox::io::IReadFile* file, int version)
{
    if (version >= 17)
        Minerals.read(file);
    else
    {
        Minerals.setValue(ox::io::CHelpIO::readInt(file));
        if (version < 4)
            return;
    }

    Size = ox::io::CHelpIO::readInt(file);
    HoggerId = ox::io::CHelpIO::readInt(file);
    setSprite();
}

ox::core::CString<wchar_t> CMineralsEntity::getInfoString()
{
    return settings::gp_systemConfig->getLocalizedText(L"entity:mineralsRemaining", Minerals.getValue());
}

void CMineralsEntity::fillAreaWithMinerals(const ox::core::CRect<float>& area, int count, game::CWorld* world)
{
    float width = area.LowerRightCorner.X - area.UpperLeftCorner.X;
    float height = area.LowerRightCorner.Y - area.UpperLeftCorner.Y;

    // the third planet gets fewer
    if (world->getPlanet() == 2)
        count = ox::core::max_(count * 2 / 3, 1);

    while (count > 0)
    {
        ox::core::CPosition2d<float> center;
        do
        {
            center = ox::core::CPosition2d<float>(
                area.UpperLeftCorner.X + 10.0f + ox::algo::CRand::rand() % (int)(width - 20.0f),
                area.UpperLeftCorner.Y + 10.0f + ox::algo::CRand::rand() % (int)(height - 20.0f));
        }
        while (!world->mayPlaceObjectHere(center, true));

        int cluster = ox::algo::CRand::rand() % 6 + 1;
        for (int i = 0; i < cluster && count > 0; ++i)
        {

            ox::core::CPosition2d<float> position;
            do
            {
                position = ox::core::CPosition2d<float>(center.X + ox::algo::CRand::rand() % 100 - 50.0f,
                    center.Y + ox::algo::CRand::rand() % 100 - 50.0f);
            }
            while (!world->mayPlaceObjectHere(position, true));

            int size = 1;
            if (ox::algo::CRand::rand() % 3)
            {
                size = 0;
                if (ox::algo::CRand::rand() % 5 == 0)
                    size = 2;
            }

            gp_entityManager->appendEntity(new CMineralsEntity(position.X, position.Y, size), 0);
            --count;
        }
    }
}

} // end namespace entity
} // end namespace harvest
