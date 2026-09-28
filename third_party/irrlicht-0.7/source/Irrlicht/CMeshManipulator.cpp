// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "CMeshManipulator.h"
#include "IMesh.h"
#include "SMesh.h"
#include "SMeshBuffer.h"
#include "SMeshBufferLightMap.h"
#include "IAnimatedMesh.h"

namespace irr
{
namespace scene 
{

//! Constructor
CMeshManipulator::CMeshManipulator()
{
}


//! destructor
CMeshManipulator::~CMeshManipulator()
{
}

//! Flips the direction of surfaces. Changes backfacing triangles to frontfacing
//! triangles and vice versa.
//! \param mesh: Mesh on which the operation is performed.
void CMeshManipulator::flipSurfaces(scene::IMesh* mesh) const
{
	if (!mesh)
		return;

	s32 bcount = mesh->getMeshBufferCount();
	for (s32 b=0; b<bcount; ++b)
	{
		IMeshBuffer* buffer = mesh->getMeshBuffer(b);
		s32 idxcnt = buffer->getIndexCount();
		u16* idx = buffer->getIndices();
		s32 tmp;

		for (s32 i=0; i<idxcnt; i+=3)
		{
			tmp = idx[i+1];
			idx[i+1] = idx[i+2];
			idx[i+2] = tmp;
		}
	}
}



//! Sets the alpha vertex color value of the whole mesh to a new value
//! \param mesh: Mesh on which the operation is performed.
void CMeshManipulator::setVertexColorAlpha(scene::IMesh* mesh, s32 alpha) const
{
	if (!mesh)
		return;

	s32 i = 0;

	s32 bcount = mesh->getMeshBufferCount();
	for (s32 b=0; b<bcount; ++b)
	{
		IMeshBuffer* buffer = mesh->getMeshBuffer(b);
		void* v = buffer->getVertices();
		s32 vtxcnt = buffer->getVertexCount();

		switch(buffer->getVertexType())
		{
		case video::EVT_STANDARD:
			for (; i<vtxcnt; ++i)
				((video::S3DVertex*)v)[i].Color.setAlpha(alpha);
			break;
		case video::EVT_2TCOORDS:
			for (; i<vtxcnt; ++i)
				((video::S3DVertex2TCoords*)v)[i].Color.setAlpha(alpha);
			break;
		}
	}
}



//! Recalculates all normals of the mesh.
//! \param mesh: Mesh on which the operation is performed.
void CMeshManipulator::recalculateNormals(scene::IMesh* mesh) const
{
	if (!mesh)
		return;

	s32 bcount = mesh->getMeshBufferCount();
	for (s32 b=0; b<bcount; ++b)
	{
		IMeshBuffer* buffer = mesh->getMeshBuffer(b);
		s32 vtxcnt = buffer->getVertexCount();
		s32 idxcnt = buffer->getIndexCount();
		u16* idx = buffer->getIndices();

		switch(buffer->getVertexType())
		{
		case video::EVT_STANDARD:
			{
				video::S3DVertex* v = 
					(video::S3DVertex*)buffer->getVertices();

				for (s32 i=0; i<idxcnt; i+=3)
				{
					core::plane3d<f32> p(v[idx[i+0]].Pos, v[idx[i+1]].Pos, v[idx[i+2]].Pos);
					p.Normal.normalize();
					v[idx[i+0]].Normal = p.Normal;
					v[idx[i+1]].Normal = p.Normal;
					v[idx[i+2]].Normal = p.Normal;
				}
			}
			break;
		case video::EVT_2TCOORDS:
			{
				video::S3DVertex2TCoords* v = 
					(video::S3DVertex2TCoords*)buffer->getVertices();

				for (s32 i=0; i<idxcnt; i+=3)
				{
					core::plane3d<f32> p(v[idx[i+0]].Pos, v[idx[i+1]].Pos, v[idx[i+2]].Pos);
					p.Normal.normalize();
					v[idx[i+0]].Normal = p.Normal;
					v[idx[i+1]].Normal = p.Normal;
					v[idx[i+2]].Normal = p.Normal;
				}
			}
			break;
		}
	}
}


//! Scales the whole mesh.
//! \param mesh: Mesh on which the operation is performed.
void CMeshManipulator::scaleMesh(scene::IMesh* mesh, const core::vector3df& scale) const
{
	if (!mesh)
		return;

	core::aabbox3df meshbox;

	s32 bcount = mesh->getMeshBufferCount();
	for (s32 b=0; b<bcount; ++b)
	{
		s32 i = 0;

		IMeshBuffer* buffer = mesh->getMeshBuffer(b);
		void* v = buffer->getVertices();
		s32 vtxcnt = buffer->getVertexCount();
		core::aabbox3df bufferbox;

		switch(buffer->getVertexType())
		{
		case video::EVT_STANDARD:
			if (vtxcnt != 0)
				bufferbox.reset(((video::S3DVertex*)v)[0].Pos * scale);
			for (; i<vtxcnt; ++i)
			{
				((video::S3DVertex*)v)[i].Pos *= scale;
				bufferbox.addInternalPoint(((video::S3DVertex*)v)[i].Pos);
			}
			break;
		case video::EVT_2TCOORDS:
			if (vtxcnt != 0)
				bufferbox.reset(((video::S3DVertex2TCoords*)v)[0].Pos * scale);
			for (; i<vtxcnt; ++i)
			{
				((video::S3DVertex2TCoords*)v)[i].Pos *= scale;
				bufferbox.addInternalPoint(((video::S3DVertex2TCoords*)v)[i].Pos);
			}
			break;
		}

		buffer->getBoundingBox() = bufferbox;

		if (b == 0)
			meshbox.reset(buffer->getBoundingBox());
		else
			meshbox.addInternalBox(buffer->getBoundingBox());
	}

	mesh->getBoundingBox() = meshbox;
}


//! Recalculates the bounding box for a meshbuffer
void CMeshManipulator::recalculateBoundingBox(scene::IMeshBuffer* buffer) const
{
	void* v = buffer->getVertices();
	s32 vtxcnt = buffer->getVertexCount();
	core::aabbox3df box;
	
	s32 i = 1;

	switch(buffer->getVertexType())
	{
	case video::EVT_STANDARD:
		if (vtxcnt != 0)
			box.reset(((video::S3DVertex*)v)[0].Pos);
		for (; i<vtxcnt; ++i)
			box.addInternalPoint(((video::S3DVertex*)v)[i].Pos);
		break;
	case video::EVT_2TCOORDS:
		if (vtxcnt != 0)
			box.reset(((video::S3DVertex2TCoords*)v)[0].Pos);
		for (; i<vtxcnt; ++i)
			box.addInternalPoint(((video::S3DVertex2TCoords*)v)[i].Pos);
		break;
	}

	buffer->getBoundingBox() = box;
}



//! Clones a static IMesh into a modifyable SMesh.
SMesh* CMeshManipulator::createMeshCopy(scene::IMesh* mesh) const
{
	if (!mesh)
		return 0;

	SMesh* clone = new SMesh();

	s32 meshBufferCount = mesh->getMeshBufferCount();
	
    for (s32 b=0; b<meshBufferCount; ++b)
	{
		s32 vtxCnt = mesh->getMeshBuffer(b)->getVertexCount();
		s32 idxCnt = mesh->getMeshBuffer(b)->getIndexCount();
		const u16* idx = mesh->getMeshBuffer(b)->getIndices();

		switch(mesh->getMeshBuffer(b)->getVertexType())
		{
		case video::EVT_STANDARD:
			{
				SMeshBuffer* buffer = new SMeshBuffer();
				buffer->Material = mesh->getMeshBuffer(b)->getMaterial();

				video::S3DVertex* v = 
					(video::S3DVertex*)mesh->getMeshBuffer(b)->getVertices();

				s32 i;

				for (i=0; i<vtxCnt; ++i)
					buffer->Vertices.push_back(v[i]);

				for (i=0; i<idxCnt; ++i)
					buffer->Indices.push_back(idx[i]);

				clone->addMeshBuffer(buffer);
				buffer->drop();
			}
			break;
		case video::EVT_2TCOORDS:
			{
				SMeshBufferLightMap* buffer = new SMeshBufferLightMap();
				buffer->Material = mesh->getMeshBuffer(b)->getMaterial();

				video::S3DVertex2TCoords* v = 
					(video::S3DVertex2TCoords*)mesh->getMeshBuffer(b)->getVertices();

				s32 i;

				for (i=0; i<vtxCnt; ++i)
					buffer->Vertices.push_back(v[i]);

				for (i=0; i<idxCnt; ++i)
					buffer->Indices.push_back(idx[i]);

				clone->addMeshBuffer(buffer);
				buffer->drop();
			}
			break;
		}// end switch

	}// end for all mesh buffers

	clone->BoundingBox = mesh->getBoundingBox();
	return clone;
}



//! Creates a planar texture mapping on the mesh
//! \param mesh: Mesh on which the operation is performed.
//! \param resolution: resolution of the planar mapping. This is the value
//! specifying which is the releation between world space and 
//! texture coordinate space.
void CMeshManipulator::makePlanarTextureMapping(scene::IMesh* mesh, f32 resolution=0.01f) const
{
	if (!mesh)
		return;

	s32 bcount = mesh->getMeshBufferCount();
	for (s32 b=0; b<bcount; ++b)
	{
		IMeshBuffer* buffer = mesh->getMeshBuffer(b);
		s32 vtxcnt = buffer->getVertexCount();
		s32 idxcnt = buffer->getIndexCount();
		u16* idx = buffer->getIndices();

		switch(buffer->getVertexType())
		{
		case video::EVT_STANDARD:
			{
				video::S3DVertex* v = 
					(video::S3DVertex*)buffer->getVertices();

				for (s32 i=0; i<idxcnt; i+=3)
				{
					core::plane3d<f32> p(v[idx[i+0]].Pos, v[idx[i+1]].Pos, v[idx[i+2]].Pos);
					p.Normal.normalize();

					p.Normal.X = (f32)(fabs(p.Normal.X));
					p.Normal.Y = (f32)(fabs(p.Normal.Y));
					p.Normal.Z = (f32)(fabs(p.Normal.Z));

					// calculate planar mapping worldspace coordinates

					if (p.Normal.X > p.Normal.Y && p.Normal.X > p.Normal.Z)
					{
						for (s32 o=0; o<3; ++o)
						{
							v[idx[i+o]].TCoords.X = v[idx[i+o]].Pos.Y * resolution;
							v[idx[i+o]].TCoords.Y = v[idx[i+o]].Pos.Z * resolution;
						}
					}
					else
					if (p.Normal.Y > p.Normal.X && p.Normal.Y > p.Normal.Z)
					{
						for (s32 o=0; o<3; ++o)
						{
							v[idx[i+o]].TCoords.X = v[idx[i+o]].Pos.X * resolution;
							v[idx[i+o]].TCoords.Y = v[idx[i+o]].Pos.Z * resolution;
						}
					}
					else
					{
						for (s32 o=0; o<3; ++o)
						{
							v[idx[i+o]].TCoords.X = v[idx[i+o]].Pos.X * resolution;
							v[idx[i+o]].TCoords.Y = v[idx[i+o]].Pos.Y * resolution;
						}
					}
				}
			}
			break;
		case video::EVT_2TCOORDS:
			{
				video::S3DVertex2TCoords* v = 
					(video::S3DVertex2TCoords*)buffer->getVertices();

				for (s32 i=0; i<idxcnt; i+=3)
				{
					core::plane3d<f32> p(v[idx[i+0]].Pos, v[idx[i+1]].Pos, v[idx[i+2]].Pos);
					p.Normal.normalize();

					p.Normal.X = (f32)(fabs(p.Normal.X));
					p.Normal.Y = (f32)(fabs(p.Normal.Y));
					p.Normal.Z = (f32)(fabs(p.Normal.Z));

					// calculate planar mapping worldspace coordinates

					if (p.Normal.X > p.Normal.Y && p.Normal.X > p.Normal.Z)
					{
						for (s32 o=0; o<3; ++o)
						{
							v[idx[i+o]].TCoords.X = v[idx[i+o]].Pos.Y * resolution;
							v[idx[i+o]].TCoords.Y = v[idx[i+o]].Pos.Z * resolution;
						}
					}
					else
					if (p.Normal.Y > p.Normal.X && p.Normal.Y > p.Normal.Z)
					{
						for (s32 o=0; o<3; ++o)
						{
							v[idx[i+o]].TCoords.X = v[idx[i+o]].Pos.X * resolution;
							v[idx[i+o]].TCoords.Y = v[idx[i+o]].Pos.Z * resolution;
						}
					}
					else
					{
						for (s32 o=0; o<3; ++o)
						{
							v[idx[i+o]].TCoords.X = v[idx[i+o]].Pos.X * resolution;
							v[idx[i+o]].TCoords.Y = v[idx[i+o]].Pos.Y * resolution;
						}
					}
				}

			}
			break;
		}
	}
}


//! Returns amount of polygons in mesh.
s32 CMeshManipulator::getPolyCount(scene::IMesh* mesh) const
{
	if (!mesh)
		return 0;

	s32 trianglecount = 0;

	for (int g=0; g<mesh->getMeshBufferCount(); ++g)
		trianglecount += mesh->getMeshBuffer(g)->getIndexCount() / 3;

	return trianglecount;
}


//! Returns amount of polygons in mesh.
s32 CMeshManipulator::getPolyCount(scene::IAnimatedMesh* mesh) const
{
	if (mesh && mesh->getFrameCount() != 0)
		return getPolyCount(mesh->getMesh(0));

	return 0;
}

} // end namespace scene
} // end namespace irr

