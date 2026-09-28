// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_OPEN_GL_TEXTURE_H_INCLUDED__
#define __C_OPEN_GL_TEXTURE_H_INCLUDED__

#include "ITexture.h"
#include "IImage.h"

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_OPENGL_

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#else
#include <GL/glx.h>
#include <GL/gl.h>
#endif

namespace irr
{
namespace video  
{

//! OpenGL texture.
class COpenGLTexture : public ITexture
{
public:

	//! constructor
	COpenGLTexture(IImage* surface, bool generateMipLevels);

	//! destructor
	virtual ~COpenGLTexture();

	//! lock function
	virtual void* lock();

	//! unlock function
	virtual void unlock();

	//! Returns original size of the texture.
	virtual const core::dimension2d<s32>& getOriginalSize();

	//! Returns (=size) of the texture.
	virtual const core::dimension2d<s32>& getSize();

	//! returns driver type of texture (=the driver, who created the texture)
	virtual E_DRIVER_TYPE getDriverType();

	//! returns color format of texture
	virtual ECOLOR_FORMAT getColorFormat();

	//! returns pitch of texture (in bytes)
	virtual s32 getPitch();

	//! return open gl texture name
	GLuint getOpenGLTextureName();

private:

	void getImageData(IImage* image);

	//! copies the the texture into an open gl texture.
	void copyTexture();
	
	//! returns the size of a texture which would be the optimize size for rendering it
	inline s32 getTextureSizeFromSurfaceSize(s32 size);

	//! test if an error occurred, prints the problem, and returns 
	//! true if an error happened
	inline bool testError();

	core::dimension2d<s32> ImageSize;
	core::dimension2d<s32> OriginalSize;
	s32 Pitch;
	bool SufaceHasSameSize; // true if Surface has the same dimension as texture.
	s32* ImageData;
	
	GLuint TextureName;
	bool hasMipMaps;
};


} // end namespace video
} // end namespace irr

#endif
#endif // _IRR_COMPILE_WITH_OPENGL_
