// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "CTerrainSceneNode.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"
#include "os.h"
#include "CImage.h"
#include <stdio.h>

namespace irr
{
namespace scene
{

//! constructor
CTerrainSceneNode::CTerrainSceneNode(ISceneNode* parent,
									 ISceneManager* mgr, s32 id,
									 const core::vector3df& position,
									 const core::vector3df& rotation,
									 const core::vector3df& scale)
: ITerrainSceneNode(parent, mgr, id, position, rotation, scale),
	debuggingLOD(false), debugLOD(0)
{
}


//! destructor
CTerrainSceneNode::~CTerrainSceneNode()
{
	for (s32 i=0; i<(s32)IndexArrayList.size(); ++i)
		delete IndexArrayList[i];
}


const core::dimension2d<s32> CTerrainSceneNode::getBestBlockSize(
	core::dimension2d<s32> input)
{
	if (input.Width < 16) input.Width = 16;
	if (input.Width > 128) input.Width = 128;
	if (input.Height < 16) input.Height = 16;
	if (input.Height > 128) input.Height = 128;

	core::dimension2d<s32> output;

	output.Width = 0x01;
	while(output.Width < input.Width)
		output.Width <<= 1;

	output.Height = 0x01;
	while(output.Height < input.Height)
		output.Height <<= 1;

	++output.Height;
	++output.Width;

	return output;
}


video::IImage* CTerrainSceneNode::getIdealHeightMap(video::IImage* heightmap, 
													const core::dimension2d<s32> maxVtxSize)
{
	video::IImage* ret = 0;

	core::dimension2d<s32> hmapsize = heightmap->getDimension();
	core::dimension2d<s32> idealsize(1,1);
	
	while(idealsize.Width < maxVtxSize.Width || idealsize.Width < hmapsize.Width)
		idealsize.Width <<= 1;

	while(idealsize.Height < maxVtxSize.Height || idealsize.Height < hmapsize.Height)
		idealsize.Height <<= 1;

	idealsize.Height+=1;
	idealsize.Width+=1;

	if (idealsize == hmapsize)
	{
		heightmap->grab();
		ret = heightmap;
	}
	else
	{
		char tmp[255];
		sprintf(tmp, "Resized heightmap to ideal size %dx%d (vtxblock size was: %dx%d)", 
			idealsize.Width, idealsize.Height,
			maxVtxSize.Width, maxVtxSize.Height);
		os::Printer::log(tmp);

		video::CImage* h = new video::CImage(heightmap->getColorFormat(), heightmap);
		h->resizeTo(idealsize);
		ret = h;
	}		

	return ret;
}


//! loads the terrain
bool CTerrainSceneNode::load(video::IImage* texture, video::IImage* inputheightmap,
	video::ITexture* detailmap, const core::dimension2d<f32>& stretchSize,
	const core::dimension2d<s32>& vtxBlockSize, f32 maxHeight)
{
	u32 tm = os::Timer::getTime()/1000;

	if (!texture || !inputheightmap)
		return false;

	// get ideal size of vertex blocks
	core::dimension2d<s32> maxVtxBlockSize = 
		getBestBlockSize(vtxBlockSize);

	// resize heightmap to ideal size if necessary
	video::IImage* heightmap = getIdealHeightMap(
		inputheightmap, maxVtxBlockSize);

	// store some variables
	
	c8 textureName[64];
	c8 tmp[255];
	
	video::S3DVertex2TCoords vtx;
	vtx.Color.set(255,255,255,255);

	core::dimension2d<s32> hMapSize= heightmap->getDimension();
	core::dimension2d<s32> tMapSize= texture->getDimension();
	core::position2d<f32> thRel((f32)tMapSize.Width / (s32)hMapSize.Width, (f32)tMapSize.Height / (s32)hMapSize.Height); 
	core::position2d<s32> processed(0,0);

	// create all vertex blocks

	while (processed.Y<hMapSize.Height)
	{
		while(processed.X<hMapSize.Width)
		{
			core::dimension2d<s32> blockSize = maxVtxBlockSize;
			if (processed.X + blockSize.Width > hMapSize.Width)
				blockSize.Width = hMapSize.Width - processed.X;
			if (processed.Y + blockSize.Height > hMapSize.Height)
				blockSize.Height = hMapSize.Height - processed.Y;

			VertexBlocks.push_back(SVertexBlock());
			SVertexBlock& cvb = VertexBlocks[VertexBlocks.size()-1];
			cvb.Size = blockSize;

			s32 x,y;

			// add vertices of vertex block
			for (y=0; y<blockSize.Height; ++y)
				for (x=0; x<blockSize.Width; ++x)
				{
					video::SColor clr = heightmap->getPixel(x+processed.X, y+processed.Y);
					f32 height = ((clr.getRed() + clr.getGreen() + clr.getBlue()) / 3.0f)/255.0f * maxHeight;

					vtx.Pos.set((f32)(x+processed.X) * stretchSize.Width,
						height, (f32)(y+processed.Y) * stretchSize.Height);

					vtx.TCoords.set((f32)(x+0.5f) / ((f32)blockSize.Width), 
						(f32)(y+0.5f) / ((f32)blockSize.Height));

					cvb.Vertices.push_back(vtx);
				}

			// create all indices
			createIndexBuffers(cvb);

			// calculate bounding box
			calculateBoundingBox(cvb);

			// calculate all normals
			calculateNormals(cvb);

			// create texture for the block
			if (cvb.Vertices.size())
			{
				// create texture for this block
				video::IImage* img = new video::CImage(texture,
					core::position2d<s32>((s32)(processed.X*thRel.X), (s32)(processed.Y*thRel.Y)),
					core::dimension2d<s32>((s32)(blockSize.Width*thRel.X), (s32)(blockSize.Height*thRel.Y)));

				sprintf(textureName, "terrain%d_%d", tm, VertexBlocks.size());

				cvb.Material.Lighting = false;
				cvb.Material.Texture1 = 
					SceneManager->getVideoDriver()->addTexture(textureName, img);

				if (cvb.Material.Texture1)
				{
					sprintf(tmp, "Generated terrain texture (%dx%d): %s", 
						cvb.Material.Texture1->getSize().Width, 
						cvb.Material.Texture1->getSize().Height,
						textureName);
					os::Printer::log(tmp);
				}
				else
					os::Printer::log("Could not create terrain texture.", textureName, ELL_ERROR);

				img->drop();
			}

            // keep on processing
			processed.X += maxVtxBlockSize.Width-1;
		}

		// keep on processing
		processed.X = 0;
		processed.Y += maxVtxBlockSize.Height-1;
	}

	calculateBoundingBox();
	heightmap->drop();

	return true;
}


//! creates index buffers for all level of details for a vertex block
void CTerrainSceneNode::createIndexBuffers(SVertexBlock& block)
{
	// search if we already have a fitting indexbuffer
	for (s32 i=0; i<(s32)IndexArrayList.size(); ++i)
		if (IndexArrayList[i]->VertexBlockSize == block.Size)
		{
			block.IndexArray = IndexArrayList[i];
			return;
		}

	// add new index array
	IndexArrayList.push_back(new SIndexArray());
	SIndexArray& idxarray = *IndexArrayList[IndexArrayList.size()-1];
    idxarray.VertexBlockSize = block.Size;
	block.IndexArray = IndexArrayList[IndexArrayList.size()-1];

	s32 x,y;
	
	// create levels of detail without borders

	core::array<s32> widths;
	core::array<s32> heights;

	for (s32 lodfactor=1; 
		(block.Size.Height/lodfactor)>2 &&
		(block.Size.Width/lodfactor)>2; lodfactor*=2)
	{
		idxarray.LoDs.push_back(SLoD());
		s32 lodnr = idxarray.LoDs.size()-1;
		idxarray.LoDs[lodnr].Buffer.push_back(SIndexBuffer());
		SIndexBuffer& idxbfr2 = idxarray.LoDs[lodnr].Buffer[0];
		
		idxbfr2.OrigVertexBlockSize = block.Size;
		idxbfr2.IndexedVertexBlockSize.Width = (block.Size.Width - lodfactor - 1) / lodfactor;
		idxbfr2.IndexedVertexBlockSize.Height = (block.Size.Height - lodfactor - 1) / lodfactor;

		if (widths.binary_search(idxbfr2.IndexedVertexBlockSize.Width)==-1)
			widths.push_back(idxbfr2.IndexedVertexBlockSize.Width);

		if (heights.binary_search(idxbfr2.IndexedVertexBlockSize.Height)==-1)
			heights.push_back(idxbfr2.IndexedVertexBlockSize.Height);

		// generate indices
		for (y=0; y<(s32)((block.Size.Height-lodfactor-1)/lodfactor); ++y)
			for (x=0; x<(s32)((block.Size.Width-lodfactor-1)/lodfactor); ++x)
		{
			s32 c = ((y*lodfactor)*block.Size.Width) + (x*lodfactor);

			idxbfr2.Indices.push_back(c);
			idxbfr2.Indices.push_back(c + (block.Size.Width*lodfactor));
			idxbfr2.Indices.push_back(c + lodfactor);

			idxbfr2.Indices.push_back(c + lodfactor);
			idxbfr2.Indices.push_back(c + (block.Size.Width*lodfactor));
			idxbfr2.Indices.push_back(c + lodfactor + (block.Size.Width*lodfactor));
		}
	}

	// now every LOD has got its default indices. The indices to fit
	// the different borders are still missing. We create them now.

	for (s32 l=0; l<(s32)idxarray.LoDs.size(); ++l)
	{
		// copy already created indices
		SLoD& lod = idxarray.LoDs[l];
		SIndexBuffer virginBuffer = lod.Buffer[0];
		lod.Buffer.clear();

		for (s32 w=0; w<(s32)widths.size(); ++w)
			for (s32 h=0; h<(s32)heights.size(); ++h)
			{
				lod.Buffer.push_back(virginBuffer);
				SIndexBuffer& buf = lod.Buffer[lod.Buffer.size()-1];

				s32 wVertexStep = buf.OrigVertexBlockSize.Width / buf.IndexedVertexBlockSize.Width;
				s32 hVertexStep = buf.OrigVertexBlockSize.Height / buf.IndexedVertexBlockSize.Height;

				// add %width% indices on top
				//if (widths[w] <= buf.IndexedVertexBlockSize.Width)
				{
					// this block has more details

					//buf.Indices.push_back(buf.IndexedVertexBlockSize.Height*hVertexStep);
					//buf.Indices.push_back((buf.IndexedVertexBlockSize.Height*hVertexStep)+(buf.OrigVertexBlockSize.Width*wVertexStep));
					//buf.Indices.push_back(buf.OrigVertexBlockSize.Width+(buf.OrigVertexBlockSize.Width*wVertexStep)-1);
	
					// add first basic triangle
					// |------
					// |    /
					// |  /
					// |/
					//
					buf.Indices.push_back(buf.IndexedVertexBlockSize.Height*wVertexStep);
					buf.Indices.push_back((buf.IndexedVertexBlockSize.Height*hVertexStep)+(buf.OrigVertexBlockSize.Height*hVertexStep));
					buf.Indices.push_back(buf.OrigVertexBlockSize.Height+(buf.OrigVertexBlockSize.Height*wVertexStep)-1);

				}
				/*else
				{
					// this block has smaller or similar details

					s32 vpb = widths[w] / buf.IndexedVertexBlockSize.Width;
					buf.Indices.clear();
				}*/
				
			}
	}
}


//! calculates all normals of the vertex block
void CTerrainSceneNode::calculateNormals(SVertexBlock& block)
{
	if (!block.IndexArray || block.IndexArray->LoDs.empty())
		return;

	s32 indexCnt = block.IndexArray->LoDs[0].Buffer[0].Indices.size();
	u16* idx = block.IndexArray->LoDs[0].Buffer[0].Indices.pointer();

	for (s32 i=0; i<indexCnt; i+=3)
	{
		core::plane3d<f32> p(
			block.Vertices[idx[i+0]].Pos,
			block.Vertices[idx[i+1]].Pos,
			block.Vertices[idx[i+2]].Pos);
		p.Normal.normalize();

		block.Vertices[idx[i+0]].Normal = p.Normal;
		block.Vertices[idx[i+1]].Normal = p.Normal;
		block.Vertices[idx[i+2]].Normal = p.Normal;
	}
}


//! calculates the bounding box of the vertex block
void CTerrainSceneNode::calculateBoundingBox(SVertexBlock& block)
{
	if (block.Vertices.empty())
		return;

	block.Box.reset(block.Vertices[0].Pos);

	for (s32 i=1; i<(s32)block.Vertices.size(); ++i)
		block.Box.addInternalPoint(block.Vertices[i].Pos);		
}



//! Calculates height of a point in the terrain
bool CTerrainSceneNode::getHeight(core::vector2d<f32> pos, f32& outHeight)
{
	return false;
}



//! renders the node.
void CTerrainSceneNode::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();
	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

	// simply draw all
	for (s32 i=0; i<(s32)VertexBlocks.size(); ++i)
	{
		SVertexBlock& block = VertexBlocks[i];
		SIndexBuffer* idx = getIndexBuffer(block);

		if (idx)
		{
			driver->setMaterial(block.Material);

			const u16* indices = idx->Indices.pointer();
			const video::S3DVertex2TCoords* vertices = block.Vertices.pointer();
			s32 vertexCnt = block.Vertices.size();
			s32 triangleCount = idx->Indices.size() / 3;

			driver->drawIndexedTriangleList(
				vertices, vertexCnt, indices, triangleCount);
		}
	}
}



//! Returns the axis aligned, not transformed bounding box of this node.
const core::aabbox3d<f32>& CTerrainSceneNode::getBoundingBox() const
{
	return Box;
}


//! calculates the bounding box for all vertex blocks
void CTerrainSceneNode::calculateBoundingBox()
{
	if (VertexBlocks.empty())
		return;

	Box = VertexBlocks[0].Box;

	for (s32 i=1; i<(s32)VertexBlocks.size(); ++i)
		Box.addInternalBox(VertexBlocks[i].Box);
}


//! returns corresponding index buffer for vertex block
CTerrainSceneNode::SIndexBuffer* CTerrainSceneNode::getIndexBuffer(SVertexBlock& block)
{
	s32 cnt = block.IndexArray->LoDs.size();
	if (!cnt)
		return 0;

	s32 t= debugLOD;
	if (t > cnt-1)
		t = cnt-1;
	return &block.IndexArray->LoDs[t].Buffer[0];
}


void CTerrainSceneNode::OnPreRender()
{
	if (IsVisible)
		SceneManager->registerNodeForRendering(this);

	ISceneNode::OnPreRender();
}


video::SMaterial& CTerrainSceneNode::getMaterial(s32 i)
{
	if (i<0 || i>=(s32)VertexBlocks.size())
        return *((video::SMaterial*)0);

	return VertexBlocks[i].Material;
}


s32 CTerrainSceneNode::getMaterialCount()
{
	return VertexBlocks.size();
}


void CTerrainSceneNode::setDebugLOD(s32 i)
{
	debuggingLOD = (i != 0);

	debugLOD+=i;
	if (debugLOD <0)
		debugLOD = 0;
	if (debugLOD >= (s32)IndexArrayList[0]->LoDs.size())
		debugLOD = IndexArrayList[0]->LoDs.size()-1;
}




} // end namespace scene
} // end namespace irr

