// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "CBillboardSceneNode.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"
#include "ICameraSceneNode.h"
#include "os.h"

namespace irr
{
namespace scene
{

//! constructor
CBillboardSceneNode::CBillboardSceneNode(ISceneNode* parent, ISceneManager* mgr, s32 id,	
const core::vector3df& position, const core::dimension2d<f32>& size)
: IBillboardSceneNode(parent, mgr, id, position)
{
	setSize(size);

	AutomaticCullingEnabled = false;

	indices[0] = 0;
	indices[1] = 2;
	indices[2] = 1;
	indices[3] = 0;
	indices[4] = 3;
	indices[5] = 2;

	vertices[0].TCoords.set(0.0f, 0.0f);
	vertices[0].Color = 0xffffffff;

	vertices[1].TCoords.set(0.0f, 1.0f);
	vertices[1].Color = 0xffffffff;

	vertices[2].TCoords.set(1.0f, 1.0f);
	vertices[2].Color = 0xffffffff;

	vertices[3].TCoords.set(1.0f, 0.0f);
	vertices[3].Color = 0xffffffff;
}



CBillboardSceneNode::~CBillboardSceneNode()
{
}



//! pre render event
void CBillboardSceneNode::OnPreRender()
{
	if (IsVisible)
	{
		SceneManager->registerNodeForRendering(this, scene::SNRT_DEFAULT);
		ISceneNode::OnPreRender();
	}
}


//! render
void CBillboardSceneNode::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();
	ICameraSceneNode* camera = SceneManager->getActiveCamera();

	if (!camera || !driver)
		return;

	// make billboard look to Camera

	core::vector3df pos = getAbsolutePosition();

	core::vector3df campos = camera->getAbsolutePosition();
	core::vector3df target = camera->getTarget();
	core::vector3df up = camera->getUpVector();
	core::vector3df view = target - campos;
	view.normalize();

	core::vector3df horizontal = up.crossProduct(view);
	horizontal.normalize();

	core::vector3df vertical = horizontal.crossProduct(view);
	vertical.normalize();

	horizontal *= 0.5f * Size.Width;
	vertical *= 0.5f * Size.Height;	

	vertices[0].Pos = pos + horizontal + vertical;
	vertices[1].Pos = pos + horizontal - vertical;
	vertices[2].Pos = pos - horizontal - vertical;
	vertices[3].Pos = pos - horizontal + vertical;

	view *= -1.0f;

	for (s32 i=0; i<4; ++i)
		vertices[i].Normal = view;

	// draw

	core::matrix4 mat;
    driver->setTransform(video::ETS_WORLD, mat);
	driver->setMaterial(Material);

	driver->drawIndexedTriangleList(vertices, 4, indices, 2);
}


//! returns the axis aligned bounding box of this node
const core::aabbox3d<f32>& CBillboardSceneNode::getBoundingBox() const
{
	return BBox;
}


//! sets the size of the billboard
void CBillboardSceneNode::setSize(const core::dimension2d<f32>& size)
{
	Size = size;
}


video::SMaterial& CBillboardSceneNode::getMaterial(s32 i)
{
	return Material;
}


//! returns amount of materials used by this scene node.
s32 CBillboardSceneNode::getMaterialCount()
{
	return 1;
}


//! gets the size of the billboard
const core::dimension2d<f32>& CBillboardSceneNode::getSize()
{
	return Size;
}

} // end namespace scene
} // end namespace irr

