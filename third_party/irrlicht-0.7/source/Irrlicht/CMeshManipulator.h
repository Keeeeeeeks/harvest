// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_MESH_MANIPULATOR_H_INCLUDED__
#define __C_MESH_MANIPULATOR_H_INCLUDED__

#include "IMeshManipulator.h"

namespace irr
{
namespace scene
{

//! An interface for easily manipulate meshes.
/** Scale, set alpha value, flip surfaces, and so on. This exists for fixing problems 
 with wrong imported or exported meshes quickly after loading. It is not intended for doing mesh
 modifications and/or animations during runtime.
*/
class CMeshManipulator : public IMeshManipulator
{
public:

	//! Constructor
	CMeshManipulator();

	//! destructor
	virtual ~CMeshManipulator();

	//! Flips the direction of surfaces. Changes backfacing triangles to frontfacing
	//! triangles and vice versa.
	//! \param mesh: Mesh on which the operation is performed.
	virtual void flipSurfaces(scene::IMesh* mesh) const;

	//! Sets the alpha vertex color value of the whole mesh to a new value
	//! \param mesh: Mesh on which the operation is performed.
	virtual void setVertexColorAlpha(scene::IMesh* mesh, s32 alpha) const;

	//! Recalculates all normals of the mesh.
	//! \param mesh: Mesh on which the operation is performed.
	virtual void recalculateNormals(scene::IMesh* mesh) const;

	//! Scales the whole mesh.
	//! \param mesh: Mesh on which the operation is performed.
	virtual void scaleMesh(scene::IMesh* mesh, const core::vector3df& scale) const;

	//! Clones a static IMesh into a modifyable SMesh.
	virtual SMesh* createMeshCopy(scene::IMesh* mesh) const;

	//! Creates a planar texture mapping on the mesh
	//! \param mesh: Mesh on which the operation is performed.
	//! \param resolution: resolution of the planar mapping. This is the value
	//! specifying which is the releation between world space and 
	//! texture coordinate space.
	virtual void makePlanarTextureMapping(scene::IMesh* mesh, f32 resolution) const;

	//! Recalculates the bounding box for a meshbuffer
	virtual void recalculateBoundingBox(scene::IMeshBuffer* buffer) const;

	//! Returns amount of polygons in mesh.
	virtual s32 getPolyCount(scene::IMesh* mesh) const;

	//! Returns amount of polygons in mesh.
	virtual s32 getPolyCount(scene::IAnimatedMesh* mesh) const;
};

} // end namespace scene
} // end namespace irr


#endif

