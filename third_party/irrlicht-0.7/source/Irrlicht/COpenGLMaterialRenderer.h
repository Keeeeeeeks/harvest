// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_OPENGL_MATERIAL_RENDERER_H_INCLUDED__
#define __C_OPENGL_MATERIAL_RENDERER_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_OPENGL_

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#endif

#ifdef LINUX
#define GL_GLEXT_LEGACY 1
#include <GL/glx.h>
#include <GL/gl.h>
#include "glext.h"
#endif

#include "CVideoOpenGL.h"
#include "IMaterialRenderer.h"

namespace irr
{
namespace video  
{

//! Base class for all internal D3D9 material renderers
class COpenGLMaterialRenderer : public IMaterialRenderer
{
public:

	//! Constructor
	COpenGLMaterialRenderer(video::CVideoOpenGL* driver)
		: Driver(driver)
	{
	}

protected:

	video::CVideoOpenGL* Driver;
};


//! Solid material renderer
class COpenGLMaterialRenderer_SOLID : public COpenGLMaterialRenderer
{
public:

	COpenGLMaterialRenderer_SOLID(video::CVideoOpenGL* d)
		: COpenGLMaterialRenderer(d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			if (Driver->hasMultiTextureExtension())
			{
				Driver->extGlActiveTextureARB(GL_TEXTURE1_ARB);
				glDisable(GL_TEXTURE_2D);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

				Driver->extGlActiveTextureARB(GL_TEXTURE0_ARB);
			}
			
			glDisable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}
};


//! Solid 2 layer material renderer
class COpenGLMaterialRenderer_SOLID_2_LAYER : public COpenGLMaterialRenderer
{
public:

	COpenGLMaterialRenderer_SOLID_2_LAYER(video::CVideoOpenGL* d)
		: COpenGLMaterialRenderer(d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			if (Driver->hasMultiTextureExtension())
			{
				Driver->extGlActiveTextureARB(GL_TEXTURE1_ARB);
				glDisable(GL_TEXTURE_2D);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

				Driver->extGlActiveTextureARB(GL_TEXTURE0_ARB);
			}
			
			glDisable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}
};


//! Transparent add color material renderer
class COpenGLMaterialRenderer_TRANSPARENT_ADD_COLOR : public COpenGLMaterialRenderer
{
public:

	COpenGLMaterialRenderer_TRANSPARENT_ADD_COLOR(video::CVideoOpenGL* d)
		: COpenGLMaterialRenderer(d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			glDisable(GL_ALPHA_TEST); 

			Driver->setTexture(1,0);

			if (Driver->hasMultiTextureExtension())
				Driver->extGlActiveTextureARB(GL_TEXTURE0_ARB);

			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
		}

		material.ZWriteEnable = false;
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	//! Returns if the material is transparent. 
	virtual bool isTransparent() 
	{
		return true; 
	}
};


//! Transparent vertex alpha material renderer
class COpenGLMaterialRenderer_TRANSPARENT_VERTEX_ALPHA : public COpenGLMaterialRenderer
{
public:

	COpenGLMaterialRenderer_TRANSPARENT_VERTEX_ALPHA(video::CVideoOpenGL* d)
		: COpenGLMaterialRenderer(d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			glDisable(GL_ALPHA_TEST); 

			Driver->setTexture(1,0);

			if (Driver->hasMultiTextureExtension())
				Driver->extGlActiveTextureARB(GL_TEXTURE0_ARB);

			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
		}

		material.ZWriteEnable = false;
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	//! Returns if the material is transparent. 
	virtual bool isTransparent() 
	{
		return true; 
	}
};


//! Transparent alpha channel material renderer
class COpenGLMaterialRenderer_TRANSPARENT_ALPHA_CHANNEL : public COpenGLMaterialRenderer
{
public:

	COpenGLMaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(video::CVideoOpenGL* d)
		: COpenGLMaterialRenderer(d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			glEnable(GL_ALPHA_TEST); 
			glAlphaFunc(GL_GREATER, 0); 

			Driver->setTexture(1,0);

			if (Driver->hasMultiTextureExtension())
				Driver->extGlActiveTextureARB(GL_TEXTURE0_ARB);

			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
		}

		material.ZWriteEnable = false;
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	//! Returns if the material is transparent. 
	virtual bool isTransparent() 
	{
		return true; 
	}
};


//! material renderer for all kinds of linghtmaps 
class COpenGLMaterialRenderer_LIGHTMAP : public COpenGLMaterialRenderer
{
public:

	COpenGLMaterialRenderer_LIGHTMAP(video::CVideoOpenGL* d)
		: COpenGLMaterialRenderer(d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			if (Driver->hasMultiTextureExtension())
			{
				glDisable(GL_BLEND);
				glDisable(GL_ALPHA_TEST);

				// diffuse map

				Driver->extGlActiveTextureARB(GL_TEXTURE0_ARB);

				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);

				// lightmap
												
				Driver->extGlActiveTextureARB(GL_TEXTURE1_ARB);

				glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_COMBINE_EXT);

				if (material.MaterialType == EMT_LIGHTMAP_ADD)
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
				else
					glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_RGB_EXT, GL_MODULATE);

				glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_RGB_EXT,GL_PREVIOUS_EXT);
				glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND0_RGB_EXT,GL_SRC_COLOR);//GL_TEXTURE);

				glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE1_RGB_EXT, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND1_RGB_EXT,GL_SRC_COLOR);

				glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE2_RGB_EXT,GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV,GL_OPERAND2_RGB_EXT,GL_SRC_COLOR);

				if (material.MaterialType == EMT_LIGHTMAP_M4)
					glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 4);
				else
				if (material.MaterialType == EMT_LIGHTMAP_M2)
					glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 2);
				else
					glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 1);
			}
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}
};


//! sphere map material renderer
class COpenGLMaterialRenderer_SPHERE_MAP : public COpenGLMaterialRenderer
{
public:

	COpenGLMaterialRenderer_SPHERE_MAP(video::CVideoOpenGL* d)
		: COpenGLMaterialRenderer(d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			if (Driver->hasMultiTextureExtension())
			{
				Driver->extGlActiveTextureARB(GL_TEXTURE1_ARB);
				glDisable(GL_TEXTURE_2D);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

				Driver->extGlActiveTextureARB(GL_TEXTURE0_ARB);
			}

			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			glDisable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);

			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);				
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	virtual void OnUnsetMaterial()
	{
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);	
	}
};


//! reflection 2 layer material renderer
class COpenGLMaterialRenderer_REFLECTION_2_LAYER : public COpenGLMaterialRenderer
{
public:

	COpenGLMaterialRenderer_REFLECTION_2_LAYER(video::CVideoOpenGL* d)
		: COpenGLMaterialRenderer(d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			if (Driver->hasMultiTextureExtension())
			{
				Driver->extGlActiveTextureARB(GL_TEXTURE1_ARB);
				glDisable(GL_TEXTURE_2D);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

				Driver->extGlActiveTextureARB(GL_TEXTURE0_ARB);
			}

			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			glDisable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);

			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);				
		}

		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	virtual void OnUnsetMaterial()
	{
		if (Driver->hasMultiTextureExtension())
		{
				Driver->extGlActiveTextureARB(GL_TEXTURE1_ARB);

				glDisable(GL_TEXTURE_2D);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

				Driver->extGlActiveTextureARB(GL_TEXTURE0_ARB);
		}

		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
	}
};


//! reflection 2 layer material renderer
class COpenGLMaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER : public COpenGLMaterialRenderer
{
public:

	COpenGLMaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER(video::CVideoOpenGL* d)
		: COpenGLMaterialRenderer(d) {}

	virtual void OnSetMaterial(SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) 
	{
		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			if (Driver->hasMultiTextureExtension())
			{
				Driver->extGlActiveTextureARB(GL_TEXTURE1_ARB);
				glDisable(GL_TEXTURE_2D);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

				Driver->extGlActiveTextureARB(GL_TEXTURE0_ARB);
			}

			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);

			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);			
		}

		material.ZWriteEnable = false;
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	}

	virtual void OnUnsetMaterial()
	{
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);		
	}

	//! Returns if the material is transparent. 
	virtual bool isTransparent() 
	{
		return true; 
	}
};

} // end namespace video
} // end namespace irr

#endif
#endif

