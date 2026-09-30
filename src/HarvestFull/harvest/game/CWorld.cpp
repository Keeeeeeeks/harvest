// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: scenery, collision avoidance, expansion, wind, camera bounds and save data.

#include <iostream>
#include <math.h>
#include "CWorld.h"
#include "CLuaManager.h"
#include "harvest/entity/CHarvestEntity.h"
#include "harvest/entity/CMineralsEntity.h"
#include "ox/event/IEventReceiver.h"
#include "ox/core/CMath.h"
#include "ox/core/CBasic.h"
#include "ox/io/CHelpIO.h"

namespace harvest {
namespace game {

CWorld* gp_world = 0;
ox::core::CHiddenInt* gp_mineralAmount = 0;
ox::core::CHiddenInt* gp_negatedMineralAmount = 0;
static const ox::core::CRect<float> STARTING_AREA(450, 390, 630, 570);
static const int DOODAD_TYPES[10] = { 2, 2, 2, 2, 2, 3, 4, 1, 0, 0 };

CWorld::CWorld(int gameMode, int planet)
    : VisibleGameField(0, 0, 1024, 1024), ActualGameField(0, 0, 1024, 1024),
      TargetGameField(0, 0, 1024, 1024), InitialWorld(true), Planet(planet), GameMode(gameMode),
      DoodadGridWidth(0), DoodadGridHeight(0), DoodadGrid(0), WindClock(0),
      WeatherState0(0), WeatherState1(0), WeatherState2(0), WeatherState3(0)
{
    for (int i = 0; i < 2; ++i) GroundSprites[i] = 0;
    for (int i = 0; i < 23; ++i) DoodadSprites[i] = 0;
    Random.setCurrent(ox::algo::CRand::rand());
}

CWorld::~CWorld()
{
    for (int i = 0; i < 2; ++i) if (GroundSprites[i]) GroundSprites[i]->remove();
    for (int i = 0; i < 23; ++i) if (DoodadSprites[i]) DoodadSprites[i]->remove();
    for (unsigned int i = 0; i < Doodads.size(); ++i) delete Doodads[i];
    delete[] DoodadGrid;
    for (unsigned int i = 0; i < WindPuffs.size(); ++i) delete WindPuffs[i];
}

void CWorld::changeViewSize(const ox::core::CDimension2d<int>& size)
{
    ViewSize.Width = size.Width;
    ViewSize.Height = size.Height;
}

int CWorld::getPlanet() const { return Planet; }
int CWorld::getGameMode() const { return GameMode; }
const ox::core::CRect<float>& CWorld::getActualGameFieldSize() const { return ActualGameField; }
const ox::core::CRect<float>& CWorld::getVisibleGameFieldSize() const { return VisibleGameField; }

bool CWorld::hasWorldExpandedAtLeastOnce()
{
    if (GameMode != 0) return true;
    return TargetGameField.getWidth() > 2048 || TargetGameField.getHeight() > 2048;
}

bool CWorld::worldChangesSizeInThisGameMode() const
{
    if (GameMode == 3 || GameMode == 5) return !InitialWorld;
    return true;
}

void CWorld::constrainViewPos(ox::core::CPosition2d<float>& position)
{
    if (ViewSize.Width > VisibleGameField.getWidth())
        position.X = (VisibleGameField.getWidth() - ViewSize.Width) * .5f;
    else
    {
        if (position.X <= VisibleGameField.UpperLeftCorner.X) position.X = VisibleGameField.UpperLeftCorner.X;
        else if (position.X + ViewSize.Width >= VisibleGameField.LowerRightCorner.X)
            position.X = VisibleGameField.LowerRightCorner.X - ViewSize.Width;
    }
    // The native oversize-height case writes X, rather than Y.
    if (ViewSize.Height > VisibleGameField.getHeight())
        position.X = (VisibleGameField.getHeight() - ViewSize.Height) * .5f;
    else
    {
        if (position.Y <= VisibleGameField.UpperLeftCorner.Y) position.Y = VisibleGameField.UpperLeftCorner.Y;
        else if (position.Y + ViewSize.Height >= VisibleGameField.LowerRightCorner.Y)
            position.Y = VisibleGameField.LowerRightCorner.Y - ViewSize.Height;
    }
}

bool CWorld::checkCollisionWithDoodad(const SDoodad* doodad, const ox::core::CPosition2d<float>& position)
{
    if (!doodad->Bounds.isPointInside(position)) return false;
    float x = position.X - doodad->Position.X;
    float y = (position.Y - doodad->Position.Y) * 1.5f;
    float radius = DoodadCollisionRadii[doodad->Type];
    return x * x + y * y <= radius * radius;
}

bool CWorld::mayMoveHere(const ox::core::CPosition2d<float>& position)
{
    if (DoodadGrid && DoodadGridArea.isPointInside(position))
    {
        int x = (int)((position.X - DoodadGridArea.UpperLeftCorner.X) * .00390625f);
        int y = (int)((position.Y - DoodadGridArea.UpperLeftCorner.Y) * .00390625f);
        int cellIndex = y * DoodadGridWidth + x;
        for (unsigned int i = 0; i < DoodadGrid[cellIndex].size(); ++i)
            if (checkCollisionWithDoodad(DoodadGrid[cellIndex][i], position)) return false;
    }
    return true;
}

bool CWorld::mayPlaceObjectHere(const ox::core::CPosition2d<float>& position, bool building)
{
    if (building && STARTING_AREA.isPointInside(position)) return false;
    if (DoodadGrid && DoodadGridArea.isPointInside(position))
    {
        int x = (int)((position.X - DoodadGridArea.UpperLeftCorner.X) * .00390625f);
        int y = (int)((position.Y - DoodadGridArea.UpperLeftCorner.Y) * .00390625f);
        int cellIndex = y * DoodadGridWidth + x;
        for (unsigned int i = 0; i < DoodadGrid[cellIndex].size(); ++i)
            if (checkCollisionWithDoodad(DoodadGrid[cellIndex][i], position)) return false;
    }
    return true;
}

float CWorld::getCollisionTangent(const ox::core::CPosition2d<float>& position)
{
    if (DoodadGrid && DoodadGridArea.isPointInside(position))
    {
        int x = (int)((position.X - DoodadGridArea.UpperLeftCorner.X) * .00390625f);
        int y = (int)((position.Y - DoodadGridArea.UpperLeftCorner.Y) * .00390625f);
        int cellIndex = y * DoodadGridWidth + x;
        for (unsigned int i = 0; i < DoodadGrid[cellIndex].size(); ++i)
        {
            SDoodad* doodad = DoodadGrid[cellIndex][i];
            if (checkCollisionWithDoodad(doodad, position))
            {
                float dx = position.X - doodad->Position.X;
                float dy = (position.Y - doodad->Position.Y) * 1.5f;
                if (dx == 0) return dy < 0 ? 4.71238899f : 1.57079637f;
                float angle = atanf(dy / dx);
                if (dx < 0) angle += 3.14159274f;
                return angle;
            }
        }
    }
    return 0;
}

ox::core::CPosition2d<float> CWorld::findRendezvousPoint(
    const ox::core::CPosition2d<float>& position, const ox::core::CVector2d<float>& movement)
{
    if (!DoodadGrid || !DoodadGridArea.isPointInside(position))
        return ox::core::CPosition2d<float>(0, 0);
    int x = (int)((position.X - DoodadGridArea.UpperLeftCorner.X) * .00390625f);
    int y = (int)((position.Y - DoodadGridArea.UpperLeftCorner.Y) * .00390625f);
    int cellIndex = y * DoodadGridWidth + x;
    for (unsigned int i = 0; i < DoodadGrid[cellIndex].size(); ++i)
    {
        if (!checkCollisionWithDoodad(DoodadGrid[cellIndex][i], position)) continue;
        float angle = ox::core::CMath::getAngleIY(DoodadGrid[cellIndex][i]->Position, position);
        if (angle <= 0 && angle > -.785398185f)
            angle = ox::core::abs_(movement.X) > ox::core::abs_(movement.Y) ? 4.71238899f : 0;
        else if (angle <= 1.57079637f && angle > 0)
            angle = ox::core::abs_(movement.X) > ox::core::abs_(movement.Y) ? 1.57079637f : 0;
        else if (angle <= 3.14159274f && angle > 1.57079637f)
            angle = ox::core::abs_(movement.X) > ox::core::abs_(movement.Y) ? 1.57079637f : 3.14159274f;
        else if (angle <= 4.71238899f && angle > 3.14159274f)
            angle = ox::core::abs_(movement.X) > ox::core::abs_(movement.Y) ? 4.71238899f : 3.14159274f;
        else
            angle = ox::core::abs_(movement.X) > ox::core::abs_(movement.Y) ? 4.71238899f : 0;
        float radius = DoodadCollisionRadii[DoodadGrid[cellIndex][i]->Type];
        return ox::core::CPosition2d<float>(DoodadGrid[cellIndex][i]->Position.X + cos((double)angle) * (radius + 40),
            DoodadGrid[cellIndex][i]->Position.Y + sin((double)angle) * (radius * .691999972f + 40));
    }
    return ox::core::CPosition2d<float>(position);
}

void CWorld::applyWind(const ox::core::CVector3d<float>& position,
    ox::core::CVector2d<float>& speed, float frameDelta) const
{
    if (Planet != 1) return;
    for (unsigned int i = 0; i < WindPuffs.size(); ++i)
    {
        float distance = ox::core::CMath::getEstimateDistance(
            ox::core::CPosition2d<float>(WindPuffs[i]->Position.X, WindPuffs[i]->Position.Y),
            ox::core::CPosition2d<float>(position.X, position.Y));
        if (distance < 200)
        {
            float amount = (1.0f - distance / 200.0f) * frameDelta;
            speed.X += WindPuffs[i]->Speed.X * amount;
            speed.Y += WindPuffs[i]->Speed.Y * amount;
        }
    }
    speed.X -= 5.0f * frameDelta;
}

void CWorld::createDoodad(const ox::core::CPosition2d<float>& position, int type)
{
    SDoodad* doodad = new SDoodad;
    doodad->Type = type;
    doodad->Position = position;
    float halfHeight = DoodadSizes[type].Height >> 1;
    float halfWidth = DoodadSizes[type].Width >> 1;
    doodad->Bounds = ox::core::CRect<float>(position.X - halfWidth, position.Y - halfHeight,
        position.X + halfWidth, position.Y + halfHeight);
    Doodads.push_back(doodad);
}

void CWorld::placeDoodads(const ox::core::CRect<float>& area, int count)
{
    for (int i = 0; i < count; ++i)
    {
        int type = 0;
        if (Planet == 2)
        {
            int choice = Random.nextInt(10);
            if ((unsigned int)choice <= 9) type = DOODAD_TYPES[choice];
            type = i == 0 ? 0 : type;
        }
        else if (Planet == 0) type = Random.nextInt(6) + 5;
        else if (Planet == 1) type = Random.nextInt(12) + 11;
        ox::core::CPosition2d<float> position;
        bool valid = false;
        for (int attempt = 0; attempt < 4 && !valid; ++attempt)
        {
            position.X = area.UpperLeftCorner.X + Random.nextInt((int)area.getWidth());
            position.Y = area.UpperLeftCorner.Y + Random.nextInt((int)area.getHeight());
            float halfHeight = DoodadSizes[type].Height >> 1;
            float halfWidth = DoodadSizes[type].Width >> 1;
            ox::core::CRect<float> bounds(position.X - halfWidth, position.Y - halfHeight,
                position.X + halfWidth, position.Y + halfHeight);
            if (bounds.isRectCollided(STARTING_AREA)) continue;
            valid = true;
            if (Planet == 2)
                for (unsigned int j = 0; j < Doodads.size(); ++j)
                    if (Doodads[j]->Bounds.isRectCollided(bounds))
                    {
                        valid = false;
                        break;
                    }
        }
        if (valid) createDoodad(position, type);
    }
    recreateDoodadGrid();
}

void CWorld::recreateDoodadGrid()
{
    delete[] DoodadGrid;
    DoodadGrid = 0;
    if (Planet != 2) return;
    DoodadGridArea = TargetGameField;
    DoodadGridArea.UpperLeftCorner -= ox::core::CPosition2d<float>(512, 512);
    DoodadGridArea.LowerRightCorner += ox::core::CPosition2d<float>(512, 512);
    DoodadGridWidth = (int)(DoodadGridArea.getWidth() * .00390625f);
    DoodadGridHeight = (int)(DoodadGridArea.getHeight() * .00390625f);
    DoodadGrid = new ox::TArray<SDoodad*>[DoodadGridWidth * DoodadGridHeight];
    for (int y = 0; y < DoodadGridHeight; ++y)
    {
        for (int x = 0; x < DoodadGridWidth; ++x)
        {
            ox::core::CRect<float> cellArea;
            cellArea.UpperLeftCorner.Y = y * 256.0f + DoodadGridArea.UpperLeftCorner.Y;
            cellArea.UpperLeftCorner.X = x * 256.0f + DoodadGridArea.UpperLeftCorner.X;
            cellArea.LowerRightCorner.X = cellArea.UpperLeftCorner.X + 256;
            cellArea.LowerRightCorner.Y = cellArea.UpperLeftCorner.Y + 256;
            for (unsigned int i = 0; i < Doodads.size(); ++i)
            {
                const ox::core::CRect<float>& bounds = Doodads[i]->Bounds;
                if (bounds.isRectCollided(cellArea))
                    DoodadGrid[y * DoodadGridWidth + x].push_back(Doodads[i]);
            }
        }
    }
}

bool CWorld::write(ox::io::IWriteFile* file)
{
    ox::io::CHelpIO::writeFloat(file, TargetGameField.UpperLeftCorner.X);
    ox::io::CHelpIO::writeFloat(file, TargetGameField.UpperLeftCorner.Y);
    ox::io::CHelpIO::writeFloat(file, TargetGameField.LowerRightCorner.X);
    ox::io::CHelpIO::writeFloat(file, TargetGameField.LowerRightCorner.Y);
    ox::io::CHelpIO::writeFloat(file, VisibleGameField.UpperLeftCorner.X);
    ox::io::CHelpIO::writeFloat(file, VisibleGameField.UpperLeftCorner.Y);
    ox::io::CHelpIO::writeFloat(file, VisibleGameField.LowerRightCorner.X);
    ox::io::CHelpIO::writeFloat(file, VisibleGameField.LowerRightCorner.Y);
    ox::io::CHelpIO::writeFloat(file, ActualGameField.UpperLeftCorner.X);
    ox::io::CHelpIO::writeFloat(file, ActualGameField.UpperLeftCorner.Y);
    ox::io::CHelpIO::writeFloat(file, ActualGameField.LowerRightCorner.X);
    ox::io::CHelpIO::writeFloat(file, ActualGameField.LowerRightCorner.Y);
    ox::io::CHelpIO::writeInt(file, Planet);
    ox::io::CHelpIO::writeInt(file, Doodads.size());
    for (unsigned int i = 0; i < (unsigned int)Doodads.size(); ++i)
    {
        ox::io::CHelpIO::writeInt(file, Doodads[i]->Type);
        ox::io::CHelpIO::writeFloat(file, Doodads[i]->Position.X);
        ox::io::CHelpIO::writeFloat(file, Doodads[i]->Position.Y);
    }
    return true;
}

ox::core::CRect<float> CWorld::expandWorld(int direction, float boundary, bool populate)
{
    ox::core::CRect<float> sceneryArea;
    ox::core::CRect<float> addedArea;
    switch (direction)
    {
    case 0:
        sceneryArea = ox::core::CRect<float>(TargetGameField.UpperLeftCorner.X, boundary - 512,
            TargetGameField.LowerRightCorner.X, TargetGameField.UpperLeftCorner.Y - 512);
        addedArea = ox::core::CRect<float>(TargetGameField.UpperLeftCorner.X, boundary,
            TargetGameField.LowerRightCorner.X, TargetGameField.UpperLeftCorner.Y);
        TargetGameField.UpperLeftCorner.Y = boundary;
        break;
    case 1:
        sceneryArea = ox::core::CRect<float>(boundary - 512, TargetGameField.UpperLeftCorner.Y,
            TargetGameField.UpperLeftCorner.X - 512, TargetGameField.LowerRightCorner.Y);
        addedArea = ox::core::CRect<float>(boundary, TargetGameField.UpperLeftCorner.Y,
            TargetGameField.UpperLeftCorner.X, TargetGameField.LowerRightCorner.Y);
        TargetGameField.UpperLeftCorner.X = boundary;
        break;
    case 2:
        sceneryArea = ox::core::CRect<float>(TargetGameField.UpperLeftCorner.X, TargetGameField.LowerRightCorner.Y + 512,
            TargetGameField.LowerRightCorner.X, boundary + 512);
        addedArea = ox::core::CRect<float>(TargetGameField.UpperLeftCorner.X, TargetGameField.LowerRightCorner.Y,
            TargetGameField.LowerRightCorner.X, boundary);
        TargetGameField.LowerRightCorner.Y = boundary;
        break;
    case 3:
        sceneryArea = ox::core::CRect<float>(TargetGameField.LowerRightCorner.X + 512, TargetGameField.UpperLeftCorner.Y,
            boundary + 512, TargetGameField.LowerRightCorner.Y);
        addedArea = ox::core::CRect<float>(TargetGameField.LowerRightCorner.X, TargetGameField.UpperLeftCorner.Y,
            boundary, TargetGameField.LowerRightCorner.Y);
        TargetGameField.LowerRightCorner.X = boundary;
        break;
    }
    placeDoodads(sceneryArea, ((int)sceneryArea.getWidth() / 512) * ((int)sceneryArea.getHeight() / 512) * 8);
    if (populate)
        for (float y = addedArea.UpperLeftCorner.Y; y < addedArea.LowerRightCorner.Y; y += 512)
            for (float x = addedArea.UpperLeftCorner.X; x < addedArea.LowerRightCorner.X; x += 512)
            {
                ox::core::CRect<float> cell(x, y, x + 512, y + 512);
                float distance = ox::core::CMath::getEstimateDistance(
                    ox::core::CPosition2d<float>((cell.UpperLeftCorner.X + cell.LowerRightCorner.X) * .5f,
                        (cell.UpperLeftCorner.Y + cell.LowerRightCorner.Y) * .5f),
                    ox::core::CPosition2d<float>(512, 512)) * .001953125f;
                int minerals = (int)(30 - (distance * 4 + distance * .5f * distance));
                // The native bonus applies to the inclusive range -24..-16.
                if ((unsigned int)(minerals + 24) < 9 || minerals > 20) minerals = 20;
                if (minerals < 2) minerals = 2;
                entity::CMineralsEntity::fillAreaWithMinerals(cell, minerals, this);
            }
    if (GameMode == 0 && (TargetGameField.getWidth() > 2048 || TargetGameField.getHeight() > 2048))
    {
        ox::event::SEvent event;
        event.EventType = ox::event::EET_USER_EVENT;
        event.UserEvent.UserData1 = 22;
        event.UserEvent.UserData2 = 27;
        event.UserEvent.UserData3 = 0;
        event.UserEvent.UserPointer = 0;
        ox::event::gp_subscriberList->OnEvent(event);
    }
    if (TargetGameField.getWidth() * TargetGameField.getHeight() >= 10485760)
    {
        ox::event::SEvent event;
        event.EventType = ox::event::EET_USER_EVENT;
        event.UserEvent.UserData1 = 21;
        event.UserEvent.UserData2 = 15;
        event.UserEvent.UserData3 = 0;
        event.UserEvent.UserPointer = 0;
        ox::event::gp_subscriberList->OnEvent(event);
        ox::event::SEvent secondEvent;
        secondEvent.EventType = ox::event::EET_USER_EVENT;
        secondEvent.UserEvent.UserData1 = 21;
        secondEvent.UserEvent.UserData2 = 16;
        secondEvent.UserEvent.UserData3 = 0;
        secondEvent.UserEvent.UserPointer = 0;
        ox::event::gp_subscriberList->OnEvent(secondEvent);
    }
    if (gp_luaManager) gp_luaManager->hookMapExpanded(addedArea.UpperLeftCorner.X, addedArea.UpperLeftCorner.Y,
        addedArea.LowerRightCorner.X, addedArea.LowerRightCorner.Y);
    return ox::core::CRect<float>(addedArea);
}

void CWorld::expandWorldFromCurrent(int direction, bool populate)
{
    switch (direction)
    {
    case 0: expandWorld(0, ActualGameField.UpperLeftCorner.Y - 512, populate); break;
    case 1: expandWorld(1, ActualGameField.UpperLeftCorner.X - 512, populate); break;
    case 2: expandWorld(2, ActualGameField.LowerRightCorner.Y + 512, populate); break;
    case 3: expandWorld(3, ActualGameField.LowerRightCorner.X + 512, populate); break;
    }
    ActualGameField = TargetGameField;
    VisibleGameField = TargetGameField;
}

} // end namespace game
} // end namespace harvest
