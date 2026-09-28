#include "CVideoNull.h"
#include "CSoftwareTexture.h"
#include "os.h"
#include <stdio.h>
#include "CImage.h"
#include "IMeshBuffer.h"
#include <memory.h>

namespace irr
{
namespace video
{

//! creates a loader which is able to load windows bitmaps
IImageLoader* createImageLoaderBmp();
//! creates a loader which is able to load jpeg images
IImageLoader* createImageLoaderJPG();
//! creates a loader which is able to load targa images
IImageLoader* createImageLoaderTGA();
//! creates a loader which is able to load psd images
IImageLoader* createImageLoaderPSD();
//! creates a loader which is able to load pcx images
IImageLoader* createImageLoaderPCX();


//! constructor
CVideoNull::CVideoNull(io::IFileSystem* io, const core::dimension2d<s32>& screenSize)
: ScreenSize(screenSize), ViewPort(0,0,0,0), 
 FileSystem(io), PrimitivesDrawn(0), TextureCreationFlags(0)
{
	#ifdef _DEBUG
	setDebugName("CVideoNull");
	#endif

	setFog();

	setTextureCreationFlag(ETCF_OPTIMIZED_FOR_SPEED, true);
	setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, true);

	ViewPort = core::rect<s32>(core::position2d<s32>(0,0), screenSize);

	if (FileSystem)
		FileSystem->grab();

	// create surface loader

	SurfaceLoader.push_back(video::createImageLoaderBmp());
	SurfaceLoader.push_back(video::createImageLoaderJPG());
	SurfaceLoader.push_back(video::createImageLoaderTGA());
	SurfaceLoader.push_back(video::createImageLoaderPSD());
	SurfaceLoader.push_back(video::createImageLoaderPCX());

	// set ExposedData to 0
	memset(&ExposedData, 0, sizeof(ExposedData));
}



//! destructor
CVideoNull::~CVideoNull()
{
	// delete file system 

	if (FileSystem)
		FileSystem->drop();

	// delete textures

	deleteAllTextures();

	// delete surface loader

	int i=0;
	for (i=0; i<(int)SurfaceLoader.size(); ++i)
		SurfaceLoader[i]->drop();


	deleteMaterialRenders();
}


//! Adds an external surface loader to the engine. 
void CVideoNull::addExternalImageLoader(IImageLoader* loader)
{
	if (!loader)
		return;

	loader->grab();
	SurfaceLoader.push_back(loader);
}


//! deletes all textures
void CVideoNull::deleteAllTextures()
{
	for (u32 i=0; i<Textures.size(); ++i)
		Textures[i].Surface->drop();

	Textures.clear();
}



//! applications must call this method before performing any rendering. returns false if failed.
bool CVideoNull::beginScene(bool backBuffer, bool zBuffer, SColor color)
{
	PrimitivesDrawn = 0;
	return true;
}



//! applications must call this method after performing any rendering. returns false if failed.
bool CVideoNull::endScene()
{
	FPSCounter.registerFrame(os::Timer::getTime());
	return true;
}



//! queries the features of the driver, returns true if feature is available
bool CVideoNull::queryFeature(E_VIDEO_DRIVER_FEATURE feature)
{
	return false;
}



//! sets transformation
void CVideoNull::setTransform(E_TRANSFORMATION_STATE state, const core::matrix4& mat)
{
}


//! Returns the transformation set by setTransform
core::matrix4 CVideoNull::getTransform(E_TRANSFORMATION_STATE state)
{
	return core::matrix4();
}



//! sets a material
void CVideoNull::setMaterial(const SMaterial& material)
{
}



//! Removes a texture from the texture cache and deletes it, freeing lot of
//! memory. 
void CVideoNull::removeTexture(ITexture* texture)
{
	for (u32 i=0; i<Textures.size(); ++i)
		if (Textures[i].Surface == texture)
		{
			texture->drop();
			Textures.erase(i);
		}
}


//! Removes all texture from the texture cache and deletes them, freeing lot of
//! memory. 
void CVideoNull::removeAllTextures()
{
	deleteAllTextures();
}



//! loads a Texture
ITexture* CVideoNull::getTexture(const c8* filename)
{
	ITexture* texture = findTexture(filename);

	if (texture)
		return texture;

	io::IReadFile* file = FileSystem->createAndOpenFile(filename);

	if (file)
	{
		texture = loadTextureFromFile(file);
		file->drop();

		if (texture)
		{
			addTexture(texture, filename);
			texture->drop(); // drop it becaus we created it, one grab to much
		}
	}
	else
		os::Printer::log("Could not open file of texture", filename, ELL_ERROR);

	if (!texture)
		os::Printer::log("Could not load texture", filename, ELL_ERROR);

	return texture;
}



//! loads a Texture
ITexture* CVideoNull::getTexture(io::IReadFile* file)
{
	ITexture* texture = 0;

	if (file)
	{
		texture = findTexture(file->getFileName());

		if (texture)
			return texture;

		texture = loadTextureFromFile(file);

		if (texture)
		{
			addTexture(texture, file->getFileName());
			texture->drop(); // drop it because we created it, one grab to much
		}
	}

	if (!texture)
		os::Printer::log("Could not load texture", file->getFileName(), ELL_ERROR);

	return texture;
}



//! opens the file and loads it into the surface
video::ITexture* CVideoNull::loadTextureFromFile(io::IReadFile* file)
{
	ITexture* texture = 0;
	IImage* image = createImageFromFile(file);
	
	if (image)
	{
		// create texture from surface
		texture = createDeviceDependentTexture(image);
		os::Printer::log("Loaded texture", file->getFileName());
		image->drop();
	}

	return texture;
}



//! adds a surface, not loaded or created by the Irrlicht Engine
void CVideoNull::addTexture(video::ITexture* texture, const c8* filename)
{
	if (texture)
	{
		if (!filename)
			filename = "";

		SSurface s;
		s.Filename = filename;
		s.Filename.make_lower();
		s.Surface = texture;
		texture->grab();

		Textures.push_back(s);
	}
}



//! looks if the image is already loaded
video::ITexture* CVideoNull::findTexture(const c8* filename)
{
	SSurface s;
	if (!filename)
		filename = "";

	s.Filename = filename;
	s.Filename.make_lower();

	s32 index = Textures.binary_search(s);
	if (index != -1)
		return Textures[index].Surface;

	return 0;
}



//! Creates a texture from a loaded IImage.
ITexture* CVideoNull::addTexture(const c8* name, IImage* image)
{
	if (!name || !image)
		return 0;

	ITexture* t = createDeviceDependentTexture(image);
	addTexture(t, name);
	t->drop();
	return t;
}



//! creates a Texture
ITexture* CVideoNull::addTexture(const core::dimension2d<s32>& size,
								 const c8* name,
								 ECOLOR_FORMAT format)
{
	if (!name)
		return 0;

	IImage* image = new CImage(format, size);
	ITexture* t = createDeviceDependentTexture(image);
	image->drop();
	addTexture(t, name);

	if (t)
		t->drop();

	return t;
}



//! returns a device dependent texture from a software surface (IImage)
//! THIS METHOD HAS TO BE OVERRIDDEN BY DERIVED DRIVERS WITH OWN TEXTURES
ITexture* CVideoNull::createDeviceDependentTexture(IImage* surface)
{
	return new CSoftwareTexture(surface);
}



//! sets a render target
void CVideoNull::setRenderTarget(video::ITexture* texture)
{
}



//! sets a viewport
void CVideoNull::setViewPort(const core::rect<s32>& area)
{
}



//! gets the area of the current viewport
const core::rect<s32>& CVideoNull::getViewPort() const
{
	return ViewPort;
}



//! draws an indexed triangle list
void CVideoNull::drawIndexedTriangleList(const S3DVertex* vertices, s32 vertexCount, const u16* indexList, s32 triangleCount)
{
	PrimitivesDrawn += triangleCount;
}



//! draws an indexed triangle list
void CVideoNull::drawIndexedTriangleList(const S3DVertex2TCoords* vertices, s32 vertexCount, const u16* indexList, s32 triangleCount)
{
	PrimitivesDrawn += triangleCount;
}

//! Draws an indexed triangle list.
void CVideoNull::drawIndexedTriangleFan(const S3DVertex2TCoords* vertices,
	s32 vertexCount, const u16* indexList, s32 triangleCount)
{
	PrimitivesDrawn += triangleCount;
}


//! Draws an indexed triangle fan.
void CVideoNull::drawIndexedTriangleFan(const S3DVertex* vertices,
	s32 vertexCount, const u16* indexList, s32 triangleCount)
{
	PrimitivesDrawn += triangleCount;
}


//! Draws a 3d line.
void CVideoNull::draw3DLine(const core::vector3df& start, 
							const core::vector3df& end, SColor color)
{
	core::vector3df vect = start.crossProduct(end);
	vect.normalize();
	vect *= 1.0f;

	S3DVertex vtx[4];

	vtx[0].Color = color;
	vtx[1].Color = color;
	vtx[2].Color = color;
	vtx[3].Color = color;

	vtx[0].Pos = start;
	vtx[1].Pos = end;

	vtx[2].Pos = start + vect;
	vtx[3].Pos = end + vect;
	
	u16 idx[12] = {0,1,2, 0,2,1,   0,1,3, 0,3,1};

	drawIndexedTriangleList(vtx, 4, idx, 4);
}



//! Draws a 3d triangle.
void CVideoNull::draw3DTriangle(const core::triangle3df& triangle, SColor color)
{
	draw3DLine(triangle.pointA, triangle.pointB, color);
	draw3DLine(triangle.pointB, triangle.pointC, color);
	draw3DLine(triangle.pointC, triangle.pointA, color);
}



//! Draws a 3d axis aligned box.
void CVideoNull::draw3DBox(const core::aabbox3d<f32> box, SColor color)
{
	core::vector3df edges[8];
	box.getEdges(edges);
	
	// TODO: optimize into one big drawIndexePrimitive call.

	draw3DLine(edges[5], edges[1], color);
	draw3DLine(edges[1], edges[3], color);
	draw3DLine(edges[3], edges[7], color);
	draw3DLine(edges[7], edges[5], color);
	draw3DLine(edges[0], edges[2], color);
	draw3DLine(edges[2], edges[6], color);
	draw3DLine(edges[6], edges[4], color);
	draw3DLine(edges[4], edges[0], color);
	draw3DLine(edges[1], edges[0], color);
	draw3DLine(edges[3], edges[2], color);
	draw3DLine(edges[7], edges[6], color);
	draw3DLine(edges[5], edges[4], color);
}



//! draws an 2d image
void CVideoNull::draw2DImage(video::ITexture* texture, const core::position2d<s32>& destPos)
{
}


//! Draws a part of the texture into the rectangle.
void CVideoNull::draw2DImage(video::ITexture* texture, const core::rect<s32>& destRect,
	const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect,
	video::SColor* colors, bool useAlphaChannelOfTexture)
{

}


//! draws an 2d image, using a color (if color is other then Color(255,255,255,255)) and the alpha channel of the texture if wanted.
void CVideoNull::draw2DImage(video::ITexture* texture, const core::position2d<s32>& destPos,
							 const core::rect<s32>& sourceRect,
							 const core::rect<s32>* clipRect, SColor color,
							 bool useAlphaChannelOfTexture)
{
}



//! draw an 2d rectangle
void CVideoNull::draw2DRectangle(SColor color, const core::rect<s32>& pos, const core::rect<s32>* clip)
{
}


//! Draws a 2d line. 
void CVideoNull::draw2DLine(const core::position2d<s32>& start,
								const core::position2d<s32>& end, 
								SColor color)
{
}




//! returns screen size
core::dimension2d<s32> CVideoNull::getScreenSize()
{
	return ScreenSize;
}



// returns current frames per second value
s32 CVideoNull::getFPS()
{
	return FPSCounter.getFPS();
}



//! returns amount of primitives (mostly triangles) were drawn in the last frame.
//! very useful method for statistics.
u32 CVideoNull::getPrimitiveCountDrawn()
{
	return PrimitivesDrawn;
}



//! Sets the dynamic ambient light color. The default color is
//! (0,0,0,0) which means it is dark.
//! \param color: New color of the ambient light.
void CVideoNull::setAmbientLight(const SColorf& color)
{
}



//! \return Returns the name of the video driver. Example: In case of the DirectX8
//! driver, it would return "Direct3D8".
const wchar_t* CVideoNull::getName()
{
	return L"NullDevice";
}



//! Draws a shadow volume into the stencil buffer. To draw a stencil shadow, do
//! this: Frist, draw all geometry. Then use this method, to draw the shadow
//! volume. Then, use IVideoDriver::drawStencilShadow() to visualize the shadow.
void CVideoNull::drawStencilShadowVolume(const core::vector3df* triangles, s32 count, bool zfail)
{
}


//! Fills the stencil shadow with color. After the shadow volume has been drawn
//! into the stencil buffer using IVideoDriver::drawStencilShadowVolume(), use this
//! to draw the color of the shadow. 
void CVideoNull::drawStencilShadow(bool clearStencilBuffer, video::SColor leftUpEdge, 
	video::SColor rightUpEdge,	video::SColor leftDownEdge,	video::SColor rightDownEdge)
{
}


//! deletes all dynamic lights there are
void CVideoNull::deleteAllDynamicLights()
{
	Lights.set_used(0);
}



//! adds a dynamic light
void CVideoNull::addDynamicLight(const SLight& light)
{
	Lights.push_back(light);
}



//! returns the maximal amount of dynamic lights the device can handle
s32 CVideoNull::getMaximalDynamicLightAmount()
{
	return 0;
}

//! Returns current amount of dynamic lights set
//! \return Current amount of dynamic lights set
s32 CVideoNull::getDynamicLightCount()
{
	return Lights.size();
}

//! Returns light data which was previously set with IVideDriver::addDynamicLight().
//! \param idx: Zero based index of the light. Must be greater than 0 and smaller
//! than IVideoDriver()::getDynamicLightCount.
//! \return Light data.
const SLight& CVideoNull::getDynamicLight(s32 idx)
{
	if (idx<0 || idx>=(s32)Lights.size())
		return *((SLight*)0);

	return Lights[idx];
}


//! Creates an 1bit alpha channel of the texture based of an color key.
void CVideoNull::makeColorKeyTexture(video::ITexture* texture, video::SColor color)
{
	if (!texture)
		return;

	if (texture->getColorFormat() != ECF_A1R5G5B5 &&
		texture->getColorFormat() != ECF_A8R8G8B8 )
	{
		os::Printer::log("Error: Unsupported texture color format for making color key channel.", ELL_ERROR);
		return;
	}

	if (texture->getColorFormat() == ECF_A1R5G5B5)
	{
		s16 *p = (s16*)texture->lock();

		if (!p)
		{
			os::Printer::log("Could not lock texture for making color key channel.", ELL_ERROR);
			return;
		}

		core::dimension2d<s32> dim = texture->getSize();
		s32 pitch = texture->getPitch() / 2;

		s16 ref = (0x0<<15) | (~(0x1<<15) & color.toA1R5G5B5());
		s16 blackalpha = (0x0<<15) | (~(0x1<<15) & 0);

		for (s32 x=0; x<pitch; ++x)
			for (s32 y=0; y<dim.Height; ++y)
			{
				s16 c = (0x0<<15) | (~(0x1<<15) & p[y*pitch + x]);
				p[y*pitch + x] = (c == ref) ? blackalpha : ((0x1<<15) | (~(0x1<<15) & c));
			}

		texture->unlock();
	}
	else
	{
		s32 *p = (s32*)texture->lock();

		if (!p)
		{
			os::Printer::log("Could not lock texture for making color key channel.", ELL_ERROR);
			return;
		}

		core::dimension2d<s32> dim = texture->getSize();
		s32 pitch = texture->getPitch() / 4;

		s32 ref = (0x0<<24) | (~(0xFF<<24) & color.color);
		s32 blackalpha = (0x0<<24) | (~(0xFF<<24) & 0);

		for (s32 x=0; x<pitch; ++x)
			for (s32 y=0; y<dim.Height; ++y)
			{
				s32 c = (0x0<<24) | (~(0xFF<<24) & p[y*pitch + x]);
				p[y*pitch + x] = (c == ref) ? blackalpha : ((0xFF<<24) | (~(0xFF<<24) & c));
			}

		texture->unlock();
	}
}



//! Creates an 1bit alpha channel of the texture based of an color key position.
void CVideoNull::makeColorKeyTexture(video::ITexture* texture, core::position2d<s32> colorKeyPixelPos)
{
	if (!texture)
		return;

	if (texture->getColorFormat() != ECF_A1R5G5B5 &&
		texture->getColorFormat() != ECF_A8R8G8B8 )
	{
		os::Printer::log("Error: Unsupported texture color format for making color key channel.", ELL_ERROR);
		return;
	}

	if (texture->getColorFormat() == ECF_A1R5G5B5)
	{
		s16 *p = (s16*)texture->lock();

		if (!p)
		{
			os::Printer::log("Could not lock texture for making color key channel.", ELL_ERROR);
			return;
		}

		core::dimension2d<s32> dim = texture->getSize();
		s32 pitch = texture->getPitch() / 2;

		s16 ref = (0x0<<15) | (~(0x1<<15) & p[colorKeyPixelPos.Y*dim.Width + colorKeyPixelPos.X]);
		s16 blackalpha = (0x0<<15) | (~(0x1<<15) & 0);

		for (s32 x=0; x<pitch; ++x)
			for (s32 y=0; y<dim.Height; ++y)
			{
				s16 c = (0x0<<15) | (~(0x1<<15) & p[y*pitch + x]);
				p[y*pitch + x] = (c == ref) ? blackalpha : ((0x1<<15) | (~(0x1<<15) & c));
			}

		texture->unlock();
	}
	else
	{
		s32 *p = (s32*)texture->lock();

		if (!p)
		{
			os::Printer::log("Could not lock texture for making color key channel.", ELL_ERROR);
			return;
		}

		core::dimension2d<s32> dim = texture->getSize();
		s32 pitch = texture->getPitch() / 4;

		s32 ref = (0x0<<24) | (~(0xFF<<24) & p[colorKeyPixelPos.Y*dim.Width + colorKeyPixelPos.X]);
		s32 blackalpha = (0x0<<24) | (~(0xFF<<24) & 0);

		for (s32 x=0; x<pitch; ++x)
			for (s32 y=0; y<dim.Height; ++y)
			{
				s32 c = (0x0<<24) | (~(0xFF<<24) & p[y*pitch + x]);
				p[y*pitch + x] = (c == ref) ? blackalpha : ((0xFF<<24) | (~(0xFF<<24) & c));
			}

		texture->unlock();
	}
}


//! Returns the maximum amount of primitives (mostly vertices) which
//! the device is able to render with one drawIndexedTriangleList
//! call.
s32 CVideoNull::getMaximalPrimitiveCount()
{
	return 65535;
}


//! checks triangle count and print warning if wrong
bool CVideoNull::checkPrimitiveCount(s32 vtxCount)
{
	s32 m = getMaximalPrimitiveCount();
	if ((vtxCount-1) > m)
	{
		char tmp[1024];
		sprintf(tmp,"Could not draw triangles, too many vertices(%d), maxium is %d.", vtxCount, m);
		os::Printer::log(tmp, ELL_ERROR);
		return false;
	}

	return true;
}

//! Enables or disables a texture creation flag.
void CVideoNull::setTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag, bool enabled)
{
	if (enabled && ((flag == ETCF_ALWAYS_16_BIT) || (flag == ETCF_ALWAYS_32_BIT)
		|| (flag == ETCF_OPTIMIZED_FOR_QUALITY) || (flag == ETCF_OPTIMIZED_FOR_SPEED)))
	{
		// disable other formats
		setTextureCreationFlag(ETCF_ALWAYS_16_BIT, false);
		setTextureCreationFlag(ETCF_ALWAYS_32_BIT, false);
		setTextureCreationFlag(ETCF_OPTIMIZED_FOR_QUALITY, false);
		setTextureCreationFlag(ETCF_OPTIMIZED_FOR_SPEED, false);
	}

	// set flag
	TextureCreationFlags = (TextureCreationFlags & (~flag)) | 
		((((u32)!enabled)-1) & flag);
}


//! Returns if a texture creation flag is enabled or disabled.
bool CVideoNull::getTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag)
{
	return (TextureCreationFlags & flag)!=0;
}


//! Creates a software image from a file. 
IImage* CVideoNull::createImageFromFile(const char* filename)
{
	IImage* image = 0;
	io::IReadFile* file = FileSystem->createAndOpenFile(filename);

	if (file)
	{
		image = createImageFromFile(file);
		file->drop();	
	}
	else
		os::Printer::log("Could not open file of image", filename, ELL_ERROR);

	return image;
}


//! Creates a software image from a file. 
IImage* CVideoNull::createImageFromFile(io::IReadFile* file)
{
	IImage* image = 0;
	// try to load file based on file extension
	
	u32 i;

	for (i=0; i<SurfaceLoader.size(); ++i)
		if (SurfaceLoader[i]->isALoadableFileExtension(file->getFileName()))
		{
			image = SurfaceLoader[i]->loadImage(file);
			if (image)
				break;
		}

	// try to load file based on what is in it
	if (!image)
		for (i=0; i<SurfaceLoader.size(); ++i)
		{
			if (i!=0)
				file->seek(0);

			if (SurfaceLoader[i]->isALoadableFileFormat(file))
			{
				file->seek(0);
				image = SurfaceLoader[i]->loadImage(file);
				if (image)
					break;
			}
		}

	return image;
}

//! Creates a software image from a byte array.
IImage* CVideoNull::createImageFromData(ECOLOR_FORMAT format, 
										const core::dimension2d<s32>& size, void *data)
{
	return new CImage(format,size, data);
}


//! Sets the fog mode.
void CVideoNull::setFog(SColor color, bool linearFog, f32 start, f32 end, f32 density,
						bool pixelFog, bool rangeFog)
{
	FogColor = color;
	LinearFog = linearFog;
	FogStart = start;
	FogEnd = end;
	FogDensity = density;
	PixelFog = pixelFog;
	RangeFog = rangeFog;
}


//! Draws a mesh buffer
void CVideoNull::drawMeshBuffer(scene::IMeshBuffer* mb)
{
	if (!mb)
		return;

	switch(mb->getVertexType())
	{
	case video::EVT_STANDARD:
		drawIndexedTriangleList((video::S3DVertex*)mb->getVertices(), mb->getVertexCount(), mb->getIndices(), mb->getIndexCount()/ 3);
		break;
	case video::EVT_2TCOORDS:
		drawIndexedTriangleList((video::S3DVertex2TCoords*)mb->getVertices(), mb->getVertexCount(), mb->getIndices(), mb->getIndexCount()/ 3);
		break;				
	}
}



//! Only used by the internal engine. Used to notify the driver that
//! the window was resized.
void CVideoNull::OnResize(const core::dimension2d<s32>& size)
{
	ScreenSize = size;
}

// adds a material renderer and drops it afterwards. To be used for internal creation
s32 CVideoNull::addAndDropMaterialRenderer(IMaterialRenderer* m)
{
	s32 i = addMaterialRenderer(m);

	if (m)
		m->drop();

	return i;
}


//! Adds a new material renderer to the video device.
s32 CVideoNull::addMaterialRenderer(IMaterialRenderer* renderer)
{
	if (!renderer)
		return -1;

	MaterialRenderers.push_back(renderer);
	renderer->grab();

	return MaterialRenderers.size()-1;
}


//! Returns driver and operating system specific data about the IVideoDriver.
SExposedVideoData CVideoNull::getExposedVideoData()
{
	return ExposedData;
}


//! Returns type of video driver
E_DRIVER_TYPE CVideoNull::getDriverType()
{
	return EDT_NULL;
}

//! deletes all material renderers
void CVideoNull::deleteMaterialRenders()
{
	// delete material renderers
	for (int i=0; i<(int)MaterialRenderers.size(); ++i)
		if (MaterialRenderers[i])
			MaterialRenderers[i]->drop();

	MaterialRenderers.clear();
}

//! Returns pointer to material renderer or null
IMaterialRenderer* CVideoNull::getMaterialRenderer(s32 idx)
{
	if (idx < 0 || idx >= (s32)MaterialRenderers.size())
		return 0;

	return MaterialRenderers[idx];
}

//! Returns pointer to the IGPUProgrammingServices interface.
IGPUProgrammingServices* CVideoNull::getGPUProgrammingServices()
{
	return 0;
}

//! Adds a new material renderer to the VideoDriver, using pixel and/or 
//! vertex shaders to render geometry.
s32 CVideoNull::addShaderMaterial(const c8* vertexShaderProgram,
	const c8* pixelShaderProgram,
	IShaderConstantSetCallBack* callback,
	E_MATERIAL_TYPE baseMaterial)
{
	os::Printer::log("Shader materials not implemented yet in this driver, sorry.");
	return -1;
}

//! Like IGPUProgrammingServices::addShaderMaterial(), but tries to load the 
//! programs from files.
s32 CVideoNull::addShaderMaterialFromFiles(io::IReadFile* vertexShaderProgram,
	io::IReadFile* pixelShaderProgram,
	IShaderConstantSetCallBack* callback,
	E_MATERIAL_TYPE baseMaterial)
{
	c8* vs = 0;
	c8* ps = 0;

	if (vertexShaderProgram)
	{
		s32 size = vertexShaderProgram->getSize();
		if (size)
		{
			vs = new c8[size+1];
			vertexShaderProgram->read(vs, size);
			vs[size] = 0;
		}
	}

	if (pixelShaderProgram)
	{
		s32 size = pixelShaderProgram->getSize();
		if (size)
		{
			ps = new c8[size+1];
			pixelShaderProgram->read(ps, size);
			ps[size] = 0;
		}
	}

	s32 result = addShaderMaterial(vs, ps, callback, baseMaterial);

	delete [] vs;
	delete [] ps;

	return result;
}



//! Like IGPUProgrammingServices::addShaderMaterial(), but tries to load the 
//! programs from files.
s32 CVideoNull::addShaderMaterialFromFiles(const c8* vertexShaderProgramFileName,
	const c8* pixelShaderProgramFileName,
	IShaderConstantSetCallBack* callback,
	E_MATERIAL_TYPE baseMaterial)
{
	io::IReadFile* vsfile = 0;
	io::IReadFile* psfile = 0;

	if (vertexShaderProgramFileName)
	{
		vsfile = FileSystem->createAndOpenFile(vertexShaderProgramFileName);
		if (!vsfile)
		{
			os::Printer::log("Could not open vertex shader program file", 
				vertexShaderProgramFileName, ELL_WARNING);
			return -1;
		}
	}

	if (pixelShaderProgramFileName)
	{
		psfile = FileSystem->createAndOpenFile(pixelShaderProgramFileName);
		if (!psfile)
		{
			os::Printer::log("Could not open pixel shader program file", 
				pixelShaderProgramFileName, ELL_WARNING);
			if (vsfile)
				vsfile->drop();
			return -1;
		}
	}

	s32 result = addShaderMaterialFromFiles(vsfile, psfile, callback, 
		baseMaterial);

	if (psfile)
		psfile->drop();

	if (vsfile)
		vsfile->drop();

	return result;
}



//! creates a video driver
IVideoDriver* createNullDriver(io::IFileSystem* io, const core::dimension2d<s32>& screenSize)
{
	return new CVideoNull(io, screenSize);
}


} // end namespace
} // end namespace

