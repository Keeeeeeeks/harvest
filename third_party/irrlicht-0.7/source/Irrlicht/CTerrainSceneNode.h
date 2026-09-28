// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_TERRAIN_SCENE_NODE_H_INCLUDED__
#define __C_TERRAIN_SCENE_NODE_H_INCLUDED__

#include "ITerrainSceneNode.h"
#include "array.h"
#include "S3DVertex.h"

namespace irr
{
namespace scene
{

//! A terrain scene node using simple geomipmapping.
class CTerrainSceneNode : public ITerrainSceneNode
{
public:

	//! constructor
	CTerrainSceneNode(ISceneNode* parent, ISceneManager* mgr, s32 id, 
		const core::vector3df& position = core::vector3df(0,0,0),
		const core::vector3df& rotation = core::vector3df(0,0,0),
		const core::vector3df& scale = core::vector3df(1.0f, 1.0f, 1.0f));

	//! destructor
	~CTerrainSceneNode();

	//! loads the terrain
	bool load(video::IImage* texture, video::IImage* heightmap,
		video::ITexture* detailmap, 
		const core::dimension2d<f32>& stretchSize,
		const core::dimension2d<s32>& defaultVertexBlockSize,
		f32 maxHeight);

	//! renders the node.
	virtual void render();

	virtual void OnPreRender();

	virtual video::SMaterial& getMaterial(s32 i);

	virtual s32 getMaterialCount();

	//! Returns the axis aligned, not transformed bounding box of this node.
	virtual const core::aabbox3d<f32>& getBoundingBox() const;

	//! Calculates height of a point in the terrain
	virtual bool getHeight(core::vector2d<f32> pos, f32& outHeight);

	//! Sets the Level of detail to a fixed level for all tiles.
	//! Only use this for debug purposes.
	//! \param i: Level to add to the current debug level of detail,
	//! may be negative. To disable debugging LOD, set this to 0.
	virtual void setDebugLOD(s32 i);

private:

	struct SIndexBuffer
	{
		s32 WidthOfUpperBlock;
		s32 HeightOfLeftBlock;
		core::array<u16> Indices;
		core::dimension2d<s32> OrigVertexBlockSize;
		core::dimension2d<s32> IndexedVertexBlockSize; // size of this lod

		bool operator ==(const SIndexBuffer& other) const
		{
			return WidthOfUpperBlock == other.WidthOfUpperBlock &&
				HeightOfLeftBlock == other.HeightOfLeftBlock;
		}

		bool operator <(const SIndexBuffer& other) const
		{
			return WidthOfUpperBlock < other.WidthOfUpperBlock ||
				(WidthOfUpperBlock == other.WidthOfUpperBlock && 
				HeightOfLeftBlock < other.HeightOfLeftBlock);
		}
	};

	//! Structure for holding index buffers for the same level of detail
	//! and the same vertex block size, but differen border indices
	struct SLoD
	{
		core::array<SIndexBuffer> Buffer;
	};

	//! Index for a specific vertex block size
	struct SIndexArray
	{
		core::dimension2d<s32> VertexBlockSize;
		core::array<SLoD> LoDs; // index buffers for every level of detail
	};

	// Vertex block 
    struct SVertexBlock
	{
		SVertexBlock()
			: IndexArray(0)
		{
		}

		core::dimension2d<s32> Size; // in pixels
		SIndexArray* IndexArray; // array with indizes
		video::SMaterial Material;
		core::aabbox3d<f32> Box;

		core::array<video::S3DVertex2TCoords> Vertices;
	};

	const core::dimension2d<s32> getBestBlockSize(core::dimension2d<s32> input);
	video::IImage* getIdealHeightMap(video::IImage* heightmap, const core::dimension2d<s32> maxVtxSize);


	//! creates index buffers for all level of details for a vertex block
	void createIndexBuffers(SVertexBlock& block);

	//! calculates the bounding box of the vertex block
	void calculateBoundingBox(SVertexBlock& block);

	//! calculates all normals of the vertex block
	void calculateNormals(SVertexBlock& block);

	//! calculates the bounding box for all vertex blocks
	void calculateBoundingBox();

	//! returns corresponding index buffer for vertex block
	SIndexBuffer* getIndexBuffer(SVertexBlock& block);	

	core::array<SVertexBlock> VertexBlocks;
	core::array<SIndexArray*> IndexArrayList;

	core::aabbox3d<f32> Box;
	s32 debugLOD;
	bool debuggingLOD;
};

} // end namespace scene
} // end namespace irr


#endif

