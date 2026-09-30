// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#ifndef OX_ENTITY_COXENTITY_H
#define OX_ENTITY_COXENTITY_H

#include "../core/CPosition2d.h"
#include "../core/CRect.h"
#include "../core/CVector3d.h"

namespace ox {
namespace entity {

//! Base of the entities an entity manager holds: an id, a position and a killed flag.
class COxEntity
{
public:
    COxEntity(int id);
    COxEntity(int id, float x, float y, float z);
    COxEntity(int id, const core::CPosition2d<int>& position);
    COxEntity(int id, const core::CPosition2d<float>& position);
    COxEntity(int id, const core::CVector3d<float>& position);
    virtual ~COxEntity();

    virtual int getEntityType() = 0;
    virtual int update(float frameDelta) = 0;
    //! Returns true when the entity is inside the visible area and should be rendered.
    virtual bool addToRenderList(const core::CRect<float>& visibleArea) = 0;

    //! Marks the entity for removal by its manager.
    virtual void killEntity();
    virtual bool isKilled();
    //! Called when the manager has removed the entity.
    virtual void notifyRemoved();

    virtual void setPosition(float x, float y, float z);
    virtual void setPosition(const core::CPosition2d<int>& position);
    virtual void setPosition(const core::CPosition2d<float>& position);
    virtual void setPosition(const core::CVector3d<float>& position);

    virtual int getRenderLayer() const;

    //! Render ordering: layer, then vertical position, then horizontal position.
    bool operator<(const COxEntity& other) const
    {
        if (getRenderLayer() == other.getRenderLayer())
        {
            if (Position.Y == other.Position.Y) return Position.X < other.Position.X;
            return Position.Y < other.Position.Y;
        }
        return getRenderLayer() < other.getRenderLayer();
    }

    int getId() const;
    void setId(int id);

    const core::CVector3d<float>& getPosition() const;

protected:
    int Id;
    bool Killed;
    core::CVector3d<float> Position;
};

} // end namespace entity
} // end namespace ox

#endif
