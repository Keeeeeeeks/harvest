// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#include "COxEntity.h"
// The object has an iostream static initializer; the header that pulled it in is not identified yet.
#include <iostream>

namespace ox {
namespace entity {

COxEntity::COxEntity(int id)
    : Id(id), Killed(false)
{
}

COxEntity::COxEntity(int id, float x, float y, float z)
    : Id(id), Killed(false), Position(x, y, z)
{
}

// Y is left uninitialized: both 2d constructors take it from the member being constructed
// (Position.Y where position.Y was meant), as the Mac and Linux builds do.
COxEntity::COxEntity(int id, const core::CPosition2d<int>& position)
    : Id(id), Killed(false), Position((float)position.X, Position.Y, 0)
{
}

COxEntity::COxEntity(int id, const core::CPosition2d<float>& position)
    : Id(id), Killed(false), Position(position.X, Position.Y, 0)
{
}

COxEntity::COxEntity(int id, const core::CVector3d<float>& position)
    : Id(id), Killed(false), Position(position)
{
}

COxEntity::~COxEntity()
{
}

int COxEntity::getId() const
{
    return Id;
}

void COxEntity::setId(int id)
{
    Id = id;
}

void COxEntity::killEntity()
{
    Killed = true;
}

bool COxEntity::isKilled()
{
    return Killed;
}

void COxEntity::notifyRemoved()
{
}

const core::CVector3d<float>& COxEntity::getPosition() const
{
    return Position;
}

void COxEntity::setPosition(float x, float y, float z)
{
    Position.X = x;
    Position.Y = y;
    Position.Z = z;
}

void COxEntity::setPosition(const core::CPosition2d<int>& position)
{
    Position.X = (float)position.X;
    Position.Y = (float)position.Y;
    Position.Z = 0;
}

void COxEntity::setPosition(const core::CPosition2d<float>& position)
{
    Position.X = position.X;
    Position.Y = position.Y;
    Position.Z = 0;
}

void COxEntity::setPosition(const core::CVector3d<float>& position)
{
    Position = position;
}

int COxEntity::getRenderLayer() const
{
    return 0;
}

} // end namespace entity
} // end namespace ox
