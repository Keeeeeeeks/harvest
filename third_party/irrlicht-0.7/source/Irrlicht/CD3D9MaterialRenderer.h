// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_D3D9_MATERIAL_RENDERER_H_INCLUDED__
#define __C_D3D9_MATERIAL_RENDERER_H_INCLUDED__

#ifdef WIN32

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECTX_9_
#include <d3d9.h>

#include "IMaterialRenderer.h"

namespace irr
{
namespace video  
{

D3DMATRIX UnitMatrixD3D9;
D3DMATRIX SphereMapMatrixD3D9;

//! Base class for all internal D3D9 material renderers
class CD3D9MaterialRenderer : public IMaterialRenderer
{
public:

	//! Constructor
	CD3D9MaterialRenderer(IDirect3DDevice9* d3ddev, video::IVideoDriver* driver)
		: pID3DDevice(d3ddev), Driver(driver)
	{
	}

protected:

	IDirect3DDevice9* pID3DDevice;
	video::IVideoDriver* Driver;
};


//! Solid material renderer
class CD3D9MaterialRenderer_SOLID : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_SOLID(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);
		}

		pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}
};


//! Solid 2 layer material renderer
class CD3D9MaterialRenderer_SOLID_2_LAYER : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_SOLID_2_LAYER(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			pID3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);

			pID3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0);
			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDDIFFUSEALPHA);

			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);		
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}
};


//! Transparent add color material renderer
class CD3D9MaterialRenderer_TRANSPARENT_ADD_COLOR : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_TRANSPARENT_ADD_COLOR(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pID3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			pID3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
		}

		material.ZWriteEnable = false;
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	
	//! Returns if the material is transparent. The scene managment needs to know this
	//! for being able to sort the materials by opaque and transparent.
	virtual bool isTransparent() 
	{ 
		return true; 
	}
};


//! Transparent vertex alpha material renderer
class CD3D9MaterialRenderer_TRANSPARENT_VERTEX_ALPHA : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_ALPHAOP,  D3DTOP_SELECTARG1);
			pID3DDevice->SetTextureStageState (0, D3DTSS_ALPHAARG1,  D3DTA_DIFFUSE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pID3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			pID3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		}

		material.ZWriteEnable = false;
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}
	
	//! Returns if the material is transparent. The scene managment needs to know this
	//! for being able to sort the materials by opaque and transparent.
	virtual bool isTransparent() 
	{ 
		return true; 
	}
};


//! Transparent alpha channel material renderer
class CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE ); 
			pID3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ); 
			pID3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT ); 
			pID3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE ); 
			pID3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 ); 
			pID3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ); 

			pID3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE ); 
			pID3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE ); 

			//pID3DDevice->SetRenderState(D3DRS_ALPHAREF, 1); 
			//pID3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE); 
			//pID3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL); 

			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE); 
			pID3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA); 
			pID3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA ); 
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	//! Returns if the material is transparent. The scene managment needs to know this
	//! for being able to sort the materials by opaque and transparent.
	virtual bool isTransparent() 
	{ 
		return true; 
	}
};


//! material renderer for all kinds of linghtmaps 
class CD3D9MaterialRenderer_LIGHTMAP : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_LIGHTMAP(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			if (material.MaterialType >= EMT_LIGHTMAP_LIGHTING)
			{
				// with lighting
				pID3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				pID3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				pID3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			}
			else
			{
				pID3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
				pID3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			}

			pID3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);
			pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);

			if (material.MaterialType == EMT_LIGHTMAP_ADD)
				pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);
			else
			if (material.MaterialType == EMT_LIGHTMAP_M4)
				pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
			else
			if (material.MaterialType == EMT_LIGHTMAP_M2)
				pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
			else
				pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
			pID3DDevice->SetTextureStageState (1, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);

			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);		
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}
};


//! sphere map material renderer
class CD3D9MaterialRenderer_SPHERE_MAP : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_SPHERE_MAP(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

			pID3DDevice->SetTransform( D3DTS_TEXTURE0, &SphereMapMatrixD3D9 );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	virtual void OnUnsetMaterial()
	{
		pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0);
		pID3DDevice->SetTransform( D3DTS_TEXTURE0, &UnitMatrixD3D9 );
	}
};


//! reflection 2 layer material renderer
class CD3D9MaterialRenderer_REFLECTION_2_LAYER : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_REFLECTION_2_LAYER(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_ALPHAOP,  D3DTOP_DISABLE);

			pID3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0);
			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);

			pID3DDevice->SetTransform( D3DTS_TEXTURE0, &SphereMapMatrixD3D9 );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	virtual void OnUnsetMaterial()
	{
		pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0);
		pID3DDevice->SetTransform( D3DTS_TEXTURE0, &UnitMatrixD3D9 );
	}
};


//! reflection 2 layer material renderer
class CD3D9MaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER : public CD3D9MaterialRenderer
{
public:

	CD3D9MaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER(IDirect3DDevice9* p, video::IVideoDriver* d)
		: CD3D9MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			pID3DDevice->SetTextureStageState (0, D3DTSS_ALPHAOP,  D3DTOP_SELECTARG1);
			pID3DDevice->SetTextureStageState (0, D3DTSS_ALPHAARG1,  D3DTA_DIFFUSE);

			pID3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0);
			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
			pID3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			pID3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);

			pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pID3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			pID3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

			pID3DDevice->SetTransform( D3DTS_TEXTURE0, &SphereMapMatrixD3D9 );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
		}

		material.ZWriteEnable = false;
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	virtual void OnUnsetMaterial()
	{
		pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0);
		pID3DDevice->SetTransform( D3DTS_TEXTURE0, &UnitMatrixD3D9 );
	}

	//! Returns if the material is transparent. The scene managment needs to know this
	//! for being able to sort the materials by opaque and transparent.
	virtual bool isTransparent() 
	{ 
		return true; 
	}
};

} // end namespace video
} // end namespace irr

#endif
#endif
#endif

