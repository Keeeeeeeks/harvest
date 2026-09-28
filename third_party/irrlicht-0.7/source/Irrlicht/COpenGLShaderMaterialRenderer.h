// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_OPENGL_SHADER_MATERIAL_RENDERER_H_INCLUDED__
#define __C_OPENGL_SHADER_MATERIAL_RENDERER_H_INCLUDED__

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

#include "IMaterialRenderer.h"

namespace irr
{
namespace video  
{

class CVideoOpenGL;
class IShaderConstantSetCallBack;
class IMaterialRenderer;

//! Class for using vertex and pixel shaders with OpenGL
class COpenGLShaderMaterialRenderer : public IMaterialRenderer
{
public:

	//! Constructor
	COpenGLShaderMaterialRenderer(video::CVideoOpenGL* driver, 
		s32& outMaterialTypeNr, const c8* vertexShaderProgram, const c8* pixelShaderProgram,
		IShaderConstantSetCallBack* callback, IMaterialRenderer* baseMaterial);

	//! Destructor
	~COpenGLShaderMaterialRenderer();

	virtual void OnSetMaterial(video::SMaterial& material, const video::SMaterial& lastMaterial,
		bool resetAllRenderstates, video::IMaterialRendererServices* services);

	virtual void OnUnsetMaterial();

	//! Returns if the material is transparent.
	virtual bool isTransparent();

protected:

	bool createPixelShader(const c8* pxsh);
	bool createVertexShader(const char* vtxsh);

	video::CVideoOpenGL* Driver;
	IShaderConstantSetCallBack* CallBack;
	IMaterialRenderer* BaseMaterial;

	GLuint VertexShader;
	GLuint PixelShader;
};


} // end namespace video
} // end namespace irr

#endif
#endif

