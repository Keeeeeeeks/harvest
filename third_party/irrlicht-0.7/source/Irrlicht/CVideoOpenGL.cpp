#include "CVideoOpenGL.h"

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_OPENGL_

#include "COpenGLTexture.h"
#include "COpenGLMaterialRenderer.h"
#include "COpenGLShaderMaterialRenderer.h"
#include "os.h"
#include <stdlib.h>
#include <string.h>

namespace irr
{
namespace video
{

// -----------------------------------------------------------------------
// WINDOWS 32 CONSTRUCTOR
// -----------------------------------------------------------------------
#ifdef WIN32
//! Win32 constructor and init code
CVideoOpenGL::CVideoOpenGL(const core::dimension2d<s32>& screenSize, HWND window, bool fullscreen, bool stencilBuffer, io::IFileSystem* io)
: CVideoNull(io, screenSize), HDc(0), HRc(0), Window(window),
	CurrentRenderMode(ERM_NONE), ResetRenderStates(true), StencilBuffer(stencilBuffer),
	Transformation3DChanged(true), LastSetLight(-1), MultiTextureExtension(false),
	MaxTextureUnits(1), pGlActiveTextureARB(0), pGlClientActiveTextureARB(0),
	ARBVertexProgramExtension(false), ARBFragmentProgramExtension(false),
	pGlGenProgramsARB(0), pGlBindProgramARB(0), pGlProgramStringARB(0),
	pglDeleteProgramsARB(0), pglProgramLocalParameter4fvARB(0)
{
	#ifdef _DEBUG
	setDebugName("CVideoOpenGL");
	#endif

	os::Printer::log("OpenGL Renderer", ELL_INFORMATION);
}

//! inits the open gl driver
bool CVideoOpenGL::initDriver(const core::dimension2d<s32>& screenSize, HWND window, 
							  bool fullscreen, bool vsync)
{
	static	PIXELFORMATDESCRIPTOR pfd =	{
		sizeof(PIXELFORMATDESCRIPTOR),	// Size Of This Pixel Format Descriptor
		1,							// Version Number
		PFD_DRAW_TO_WINDOW |		// Format Must Support Window
		PFD_SUPPORT_OPENGL |		// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,			// Must Support Double Buffering
		PFD_TYPE_RGBA,				// Request An RGBA Format
		16,							// Select Our Color Depth
		0, 0, 0, 0, 0, 0,			// Color Bits Ignored
		0,							// No Alpha Buffer
		0,							// Shift Bit Ignored
		0,							// No Accumulation Buffer
		0, 0, 0, 0,					// Accumulation Bits Ignored
		16,							// 16Bit Z-Buffer (Depth Buffer)
		StencilBuffer ? 1 : 0,		// Stencil Buffer
		0,							// No Auxiliary Buffer
		PFD_MAIN_PLANE,				// Main Drawing Layer
		0,							// Reserved
		0, 0, 0						// Layer Masks Ignored
	};

	for (int i=0; i<3; ++i)
	{
		if (i == 1)
		{
			os::Printer::log("Cannot create a GL device with stencil buffer, disabling stencil shadows.", ELL_WARNING);
			StencilBuffer = false;
			pfd.cStencilBits = 0;
		}
		else
		if (i == 2)
		{
			os::Printer::log("Cannot create a GL device context.", ELL_ERROR);
			return false;
		}

		// get hdc
		if (!(HDc=GetDC(window)))
		{
			os::Printer::log("Cannot create a GL device context.", ELL_ERROR);
			continue;
		}

		GLuint PixelFormat;

		// choose pixelformat
		if (!(PixelFormat = ChoosePixelFormat(HDc, &pfd)))
		{
			os::Printer::log("Cannot find a suitable pixelformat.", ELL_ERROR);
			continue;
		}

		// set pixel format
		if(!SetPixelFormat(HDc, PixelFormat, &pfd))
		{
			os::Printer::log("Cannot set the pixel format.", ELL_ERROR);
			continue;
		}

		// create rendering context
		if (!(HRc=wglCreateContext(HDc)))
		{
			os::Printer::log("Cannot create a GL rendering context.", ELL_ERROR);
			continue;
		}

		// activate rendering context
		if(!wglMakeCurrent(HDc, HRc))
		{
			os::Printer::log("Cannot activate GL rendering context", ELL_ERROR);
			continue;
		}

		break;
	}

	// print renderer information
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* vendor = glGetString(GL_VENDOR);
	if (renderer && vendor)
		os::Printer::log((const c8*)renderer, (const c8*)vendor, ELL_INFORMATION);


	// load extensions
	loadExtensions();

	glViewport(0, 0, screenSize.Width, screenSize.Height); // Reset The Current Viewport
	glShadeModel(GL_SMOOTH);
	setAmbientLight(SColor(0,0,0,0));
	glClearDepth(1.0f);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glDepthFunc(GL_LEQUAL);

	// set vsync
	if (wglSwapIntervalEXT)
		wglSwapIntervalEXT(vsync ? 1 : 0);

	// create material renderers
	createMaterialRenderers();

	// set the renderstates
	ResetRenderStates = true;
	setRenderStates3DMode();

	// set fog mode
	setFog(FogColor, LinearFog, FogStart, FogEnd, FogDensity, PixelFog, RangeFog);

	// set exposed data
	ExposedData.OpenGLWin32.HDc = reinterpret_cast<s32>(HDc);
	ExposedData.OpenGLWin32.HRc = reinterpret_cast<s32>(HRc);
	ExposedData.OpenGLWin32.HWnd = reinterpret_cast<s32>(Window);

	return true;
}



//! win32 destructor
CVideoOpenGL::~CVideoOpenGL()
{
	deleteMaterialRenders();

	// I get a blue screen on my laptop, when I do not delete the
	// textures manually before releasing the dc. Oh how I love this.

	deleteAllTextures();

	if (HRc)
	{
		if (!wglMakeCurrent(0, 0))
			os::Printer::log("Release of dc and rc failed.", ELL_WARNING);

		if (!wglDeleteContext(HRc))
			os::Printer::log("Release of rendering context failed.", ELL_WARNING);

		HRc = 0;
	}

	if (HDc)
		ReleaseDC(Window, HDc);

	HDc = 0;
}
#endif


// -----------------------------------------------------------------------
// LINUX CONSTRUCTOR
// -----------------------------------------------------------------------
#ifdef LINUX
//! Linux constructor and init code
CVideoOpenGL::CVideoOpenGL(const core::dimension2d<s32>& screenSize, bool fullscreen, bool doublebuffer, bool stencilBuffer, Window window, Display* display, io::IFileSystem* io)
: CVideoNull(io, screenSize),
	CurrentRenderMode(ERM_NONE), ResetRenderStates(true), StencilBuffer(stencilBuffer),
	Transformation3DChanged(true), LastSetLight(-1), MultiTextureExtension(false),
  MaxTextureUnits(1), DoubleBuffered(doublebuffer), XWindow(window), XDisplay(display),
	ARBVertexProgramExtension(false), ARBFragmentProgramExtension(false)
{
	#ifdef _DEBUG
	setDebugName("CVideoOpenGL");
	#endif

	os::Printer::log("OpenGL Renderer.", ELL_INFORMATION);

	loadExtensions();

	// create material renderers
	createMaterialRenderers();

	// set fog mode
	setFog(FogColor, LinearFog, FogStart, FogEnd, FogDensity, PixelFog, RangeFog);
}

//! linux destructor
CVideoOpenGL::~CVideoOpenGL()
{
	deleteAllTextures();
}

// extensions
//PFNGLACTIVETEXTUREPROC pGlActiveTextureARB = 0;
//PFNGLCLIENTACTIVETEXTUREPROC pGlClientActiveTextureARB = 0;
//PFNGLGENPROGRAMSARBPROC pGlGenProgramsARB = 0;
//PFNGLBINDPROGRAMARBPROC pGlBindProgramARB = 0;

#endif // LINUX




// -----------------------------------------------------------------------
// METHODS
// -----------------------------------------------------------------------

void CVideoOpenGL::createMaterialRenderers()
{
	// create D3D8 material renderers

	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_SOLID( this));
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_SOLID_2_LAYER( this));
	
	// add the same renderer for all lightmap types
	COpenGLMaterialRenderer_LIGHTMAP* lmr = new COpenGLMaterialRenderer_LIGHTMAP( this);
	addMaterialRenderer(lmr); // for EMT_LIGHTMAP:
	addMaterialRenderer(lmr); // for EMT_LIGHTMAP_ADD:
	addMaterialRenderer(lmr); // for EMT_LIGHTMAP_M2:
	addMaterialRenderer(lmr); // for EMT_LIGHTMAP_M4:
	addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING:
	addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING_M2:
	addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING_M4:
	lmr->drop();

	// add remaining material renderers
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_SPHERE_MAP( this));
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_REFLECTION_2_LAYER( this));
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_TRANSPARENT_ADD_COLOR( this));
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_TRANSPARENT_ALPHA_CHANNEL( this));
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_TRANSPARENT_VERTEX_ALPHA( this));
	addAndDropMaterialRenderer(new COpenGLMaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER( this));
}

void CVideoOpenGL::loadExtensions()
{
	if (atof((c8*)glGetString(GL_VERSION)) >= 1.2)
	{
		os::Printer::log("OpenGL driver version is 1.2 or better.", ELL_INFORMATION);
	}
	else
		os::Printer::log("OpenGL driver version is not 1.2 or better.", ELL_WARNING);

	const GLubyte* t = glGetString(GL_EXTENSIONS);
	s32 len = (s32)strlen((const char*)t);
	c8 *str = new c8[len+1];
	c8* p = str;

	for (s32 i=0; i<len; ++i)
	{
		str[i] = (char)t[i];

		if (str[i] == ' ')
		{
			str[i] = 0;
			if (strstr(p, "GL_ARB_multitexture"))
				MultiTextureExtension = true;
			else
			if (strstr(p, "GL_ARB_vertex_program"))
				ARBVertexProgramExtension = true;
			else
			if (strstr(p, "GL_ARB_fragment_program"))
				ARBFragmentProgramExtension = true;

			p = p + strlen(p) + 1;
		}
	}

	delete [] str;

	if (MultiTextureExtension)
	{
		#ifdef _WIN32

		// Windows
		// get multitextureing function pointers

		pGlActiveTextureARB   = (PFNGLACTIVETEXTUREARBPROC) wglGetProcAddress("glActiveTextureARB");
		pGlClientActiveTextureARB= (PFNGLCLIENTACTIVETEXTUREARBPROC) wglGetProcAddress("glClientActiveTextureARB");
		if (!pGlActiveTextureARB || !pGlClientActiveTextureARB)
		{
			MultiTextureExtension = false;
			os::Printer::log("Failed to load OpenGL's multitexture extension, proceeding without.", ELL_WARNING);
		}

		// get fragment and vertex program function pointers
		pGlGenProgramsARB = (PFNGLGENPROGRAMSARBPROC) wglGetProcAddress("glGenProgramsARB");
		pGlBindProgramARB = (PFNGLBINDPROGRAMARBPROC) wglGetProcAddress("glBindProgramARB");
		pGlProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC) wglGetProcAddress("glProgramStringARB");
		pglDeleteProgramsARB = (PFNGLDELETEPROGRAMSNVPROC) wglGetProcAddress("glDeleteProgramsARB");
		pglProgramLocalParameter4fvARB = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) wglGetProcAddress("glProgramLocalParameter4fvARB");

		#else

		// Linux

		//if (MultiTextureExtension)
		//	os::Printer::log("Multittexturing active.");

		// This works. 
		// But I am not able to test it out on my machine, because it doesnot have
		// any 3d acceleration in Linux, even with new drivers. Uncomment this and set MultiTextureExtension to true if
		// you want to use it.
		//pGlActiveTextureARB = (PFNGLACTIVETEXTUREPROC)glXGetProcAddressARB(reinterpret_cast<const GLubyte*>("glActiveTextureARB"));
		//pGlClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREPROC)glXGetProcAddressARB(reinterpret_cast<const GLubyte*>("glClientActiveTextureARB"));

		MultiTextureExtension = false;
		ARBVertexProgramExtension = false;
		ARBFragmentProgramExtension = false;
		os::Printer::log("Extentions disabled.", ELL_WARNING);

		#endif

		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &MaxTextureUnits);
	}

	if (MaxTextureUnits < 2)
	{
		MultiTextureExtension = false;
		os::Printer::log("Warning: OpenGL device only has one texture unit. Disabling multitexturing.", ELL_WARNING);
	}

	// get vsync extension
	#ifdef _WIN32
     wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress( "wglSwapIntervalEXT" );
	#endif
}



//! presents the rendered scene on the screen, returns false if failed
bool CVideoOpenGL::endScene()
{
	CVideoNull::endScene();

#ifdef WIN32
	return SwapBuffers(HDc) == TRUE;
#endif

#ifdef LINUX
	if (DoubleBuffered)
	  glXSwapBuffers(XDisplay, XWindow);
#endif
}



//! clears the zbuffer
bool CVideoOpenGL::beginScene(bool backBuffer, bool zBuffer, SColor color)
{
	CVideoNull::beginScene(backBuffer, zBuffer, color);

	GLbitfield mask = 0;

	if (backBuffer)
	{
		f32 inv = 1.0f / 255.0f;
		glClearColor(color.getRed() * inv, color.getGreen() * inv,
				color.getBlue() * inv, color.getAlpha() * inv);

		mask |= GL_COLOR_BUFFER_BIT;
	}

	if (zBuffer)
	{
		glDepthMask(GL_TRUE);
		mask |= GL_DEPTH_BUFFER_BIT;
	}

	glClear(mask);
	return true;
}



//! Returns the transformation set by setTransform
core::matrix4 CVideoOpenGL::getTransform(E_TRANSFORMATION_STATE state)
{
	return Matrizes[state];
}



//! sets transformation
void CVideoOpenGL::setTransform(E_TRANSFORMATION_STATE state, const core::matrix4& mat)
{
	Transformation3DChanged = true;

	GLfloat glmat[16];
	Matrizes[state] = mat;

	switch(state)
	{
	case ETS_VIEW:
		// OpenGL only has a model matrix, view and world is not existent. so lets fake these two.
		createGLMatrix(glmat, Matrizes[ETS_VIEW] * Matrizes[ETS_WORLD]);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(glmat);
		break;
	case ETS_WORLD:
		// OpenGL only has a model matrix, view and world is not existent. so lets fake these two.
		createGLMatrix(glmat, Matrizes[ETS_VIEW] * Matrizes[ETS_WORLD]);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(glmat);
		break;
	case ETS_PROJECTION:
		{
			createGLMatrix(glmat, mat);
			
			// flip z to compensate OpenGLs right-hand coordinate system
			glmat[12]*= -1.0f;

			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(glmat);
		}
		break;
	}
}



//! draws an indexed triangle list
void CVideoOpenGL::drawIndexedTriangleList(const S3DVertex* vertices, s32 vertexCount, const u16* indexList, s32 triangleCount)
{
	if (!checkPrimitiveCount(vertexCount))
		return;

	CVideoNull::drawIndexedTriangleList(vertices, vertexCount, indexList, triangleCount);

	setRenderStates3DMode();

	extGlClientActiveTextureARB(GL_TEXTURE0_ARB);

	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY );
	glEnableClientState(GL_NORMAL_ARRAY );

	// convert colors to gl color format.

	const S3DVertex* p = vertices;
	ColorBuffer.set_used(vertexCount);
	for (s32 i=0; i<vertexCount; ++i)
	{
		ColorBuffer[i] = p->Color.toOpenGLColor();
		++p;
	}

	// draw everything

	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(video::SColor), &ColorBuffer[0]);
	glNormalPointer(GL_FLOAT, sizeof(S3DVertex), &vertices[0].Normal);
	glTexCoordPointer(2, GL_FLOAT, sizeof(S3DVertex), &vertices[0].TCoords);
	glVertexPointer(3, GL_FLOAT, sizeof(S3DVertex),  &vertices[0].Pos);

	glDrawElements(GL_TRIANGLES, triangleCount * 3, GL_UNSIGNED_SHORT, indexList);

	glFlush();

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY );
	glDisableClientState(GL_NORMAL_ARRAY );
}



//! draws an indexed triangle list
void CVideoOpenGL::drawIndexedTriangleList(const S3DVertex2TCoords* vertices, s32 vertexCount, const u16* indexList, s32 triangleCount)
{
	if (!checkPrimitiveCount(vertexCount))
		return;

	CVideoNull::drawIndexedTriangleList(vertices, vertexCount, indexList, triangleCount);

	setRenderStates3DMode();

	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY );
	glEnableClientState(GL_NORMAL_ARRAY );

	// convert colors to gl color format.

	const S3DVertex2TCoords* p = vertices;
	ColorBuffer.set_used(vertexCount);
	for (s32 i=0; i<vertexCount; ++i)
	{
		ColorBuffer[i] = p->Color.toOpenGLColor();
		++p;
	}

	// draw everything

	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(video::SColor), &ColorBuffer[0]);
	glNormalPointer(GL_FLOAT, sizeof(S3DVertex2TCoords), &vertices[0].Normal);
	glVertexPointer(3, GL_FLOAT, sizeof(S3DVertex2TCoords),  &vertices[0].Pos);

	// texture coordiantes
	if (MultiTextureExtension)
	{
		extGlClientActiveTextureARB(GL_TEXTURE0_ARB);
		glEnableClientState ( GL_TEXTURE_COORD_ARRAY );
		glTexCoordPointer(2, GL_FLOAT, sizeof(S3DVertex2TCoords), &vertices[0].TCoords);
		
		extGlClientActiveTextureARB(GL_TEXTURE1_ARB);
		glEnableClientState ( GL_TEXTURE_COORD_ARRAY );
		glTexCoordPointer(2, GL_FLOAT, sizeof(S3DVertex2TCoords), &vertices[0].TCoords2);
	}
	else
		glTexCoordPointer(2, GL_FLOAT, sizeof(S3DVertex2TCoords), &vertices[0].TCoords);

	glDrawElements(GL_TRIANGLES, triangleCount * 3, GL_UNSIGNED_SHORT, indexList);

	glFlush();

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	if (MultiTextureExtension)
	{
		extGlClientActiveTextureARB(GL_TEXTURE0_ARB);
		glDisableClientState ( GL_TEXTURE_COORD_ARRAY );

		extGlClientActiveTextureARB(GL_TEXTURE1_ARB);
		glDisableClientState ( GL_TEXTURE_COORD_ARRAY );
	}
	else
		glDisableClientState(GL_TEXTURE_COORD_ARRAY );

	glDisableClientState(GL_NORMAL_ARRAY );
}



//! draws an indexed triangle fan
void CVideoOpenGL::drawIndexedTriangleFan(const S3DVertex* vertices, s32 vertexCount, const u16* indexList, s32 triangleCount)
{
	if (!checkPrimitiveCount(vertexCount))
		return;

	CVideoNull::drawIndexedTriangleFan(vertices, vertexCount, indexList, triangleCount);

	setRenderStates3DMode();

	extGlClientActiveTextureARB(GL_TEXTURE0_ARB);

	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY );
	glEnableClientState(GL_NORMAL_ARRAY );

	// convert colors to gl color format.

	const S3DVertex* p = vertices;
	ColorBuffer.set_used(vertexCount);
	for (s32 i=0; i<vertexCount; ++i)
	{
		ColorBuffer[i] = p->Color.toOpenGLColor();
		++p;
	}

	// draw everything

	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(video::SColor), &ColorBuffer[0]);
	glNormalPointer(GL_FLOAT, sizeof(S3DVertex), &vertices[0].Normal);
	glTexCoordPointer(2, GL_FLOAT, sizeof(S3DVertex), &vertices[0].TCoords);
	glVertexPointer(3, GL_FLOAT, sizeof(S3DVertex),  &vertices[0].Pos);

	glDrawElements(GL_TRIANGLE_FAN, triangleCount+2, GL_UNSIGNED_SHORT, indexList);

	glFlush();

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY );
	glDisableClientState(GL_NORMAL_ARRAY );
}



//! draws an indexed triangle fan
void CVideoOpenGL::drawIndexedTriangleFan(const S3DVertex2TCoords* vertices, s32 vertexCount, const u16* indexList, s32 triangleCount)
{
	if (!checkPrimitiveCount(vertexCount))
		return;

	CVideoNull::drawIndexedTriangleFan(vertices, vertexCount, indexList, triangleCount);

	setRenderStates3DMode();

	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY );
	glEnableClientState(GL_NORMAL_ARRAY );

	// convert colors to gl color format.

	const S3DVertex2TCoords* p = vertices;
	ColorBuffer.set_used(vertexCount);
	for (s32 i=0; i<vertexCount; ++i)
	{
		ColorBuffer[i] = p->Color.toOpenGLColor();
		++p;
	}

	// draw everything

	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(video::SColor), &ColorBuffer[0]);
	glNormalPointer(GL_FLOAT, sizeof(S3DVertex2TCoords), &vertices[0].Normal);
	glVertexPointer(3, GL_FLOAT, sizeof(S3DVertex2TCoords),  &vertices[0].Pos);

	// texture coordiantes
	if (MultiTextureExtension)
	{
		extGlClientActiveTextureARB(GL_TEXTURE0_ARB);
		glEnableClientState ( GL_TEXTURE_COORD_ARRAY );
		glTexCoordPointer(2, GL_FLOAT, sizeof(S3DVertex2TCoords), &vertices[0].TCoords);
		
		extGlClientActiveTextureARB(GL_TEXTURE1_ARB);
		glEnableClientState ( GL_TEXTURE_COORD_ARRAY );
		glTexCoordPointer(2, GL_FLOAT, sizeof(S3DVertex2TCoords), &vertices[0].TCoords2);
	}
	else
		glTexCoordPointer(2, GL_FLOAT, sizeof(S3DVertex2TCoords), &vertices[0].TCoords);

	glDrawElements(GL_TRIANGLE_FAN, triangleCount+2, GL_UNSIGNED_SHORT, indexList);

	glFlush();

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	if (MultiTextureExtension)
	{
		extGlClientActiveTextureARB(GL_TEXTURE0_ARB);
		glDisableClientState ( GL_TEXTURE_COORD_ARRAY );

		extGlClientActiveTextureARB(GL_TEXTURE1_ARB);
		glDisableClientState ( GL_TEXTURE_COORD_ARRAY );
	}
	else
		glDisableClientState(GL_TEXTURE_COORD_ARRAY );

	glDisableClientState(GL_NORMAL_ARRAY );
}



//! draws an 2d image
void CVideoOpenGL::draw2DImage(video::ITexture* texture, 
							   const core::position2d<s32>& destPos)
{
	if (!texture)
		return;

	draw2DImage(texture, destPos, core::rect<s32>(core::position2d<s32>(0,0), texture->getOriginalSize()));
}



//! draws an 2d image, using a color (if color is other then Color(255,255,255,255)) and the alpha channel of the texture if wanted.
void CVideoOpenGL::draw2DImage(video::ITexture* texture, const core::position2d<s32>& pos, 
								 const core::rect<s32>& sourceRect, 
								 const core::rect<s32>* clipRect, SColor color, 
								 bool useAlphaChannelOfTexture)
{
	if (!texture)
		return;

	if (sourceRect.UpperLeftCorner.X >= sourceRect.LowerRightCorner.X ||
		sourceRect.UpperLeftCorner.Y >= sourceRect.LowerRightCorner.Y)
		return;

	core::position2d<s32> targetPos = pos;
	core::position2d<s32> sourcePos = sourceRect.UpperLeftCorner;
	core::dimension2d<s32> sourceSize(sourceRect.getWidth(), sourceRect.getHeight());
	const core::dimension2d<s32> targetSurfaceSize = ScreenSize;

	if (clipRect)
	{
		if (targetPos.X < clipRect->UpperLeftCorner.X)
		{
			sourceSize.Width += targetPos.X - clipRect->UpperLeftCorner.X;
			if (sourceSize.Width <= 0)
				return;

			sourcePos.X -= targetPos.X - clipRect->UpperLeftCorner.X;
			targetPos.X = clipRect->UpperLeftCorner.X;
		}
		
		if (targetPos.X + sourceSize.Width > clipRect->LowerRightCorner.X)
		{
			sourceSize.Width -= (targetPos.X + sourceSize.Width) - clipRect->LowerRightCorner.X;
			if (sourceSize.Width <= 0)
				return;
		}

		if (targetPos.Y < clipRect->UpperLeftCorner.Y)
		{
			sourceSize.Height += targetPos.Y - clipRect->UpperLeftCorner.Y;
			if (sourceSize.Height <= 0)
				return;

			sourcePos.Y -= targetPos.Y - clipRect->UpperLeftCorner.Y;
			targetPos.Y = clipRect->UpperLeftCorner.Y;
		}
		
		if (targetPos.Y + sourceSize.Height > clipRect->LowerRightCorner.Y)
		{
			sourceSize.Height -= (targetPos.Y + sourceSize.Height) - clipRect->LowerRightCorner.Y;
			if (sourceSize.Height <= 0)
				return;
		}
	}

	// clip these coordinates

	if (targetPos.X<0)
	{
		sourceSize.Width += targetPos.X;
		if (sourceSize.Width <= 0)
			return;

		sourcePos.X -= targetPos.X;
		targetPos.X = 0;
	}
	
	if (targetPos.X + sourceSize.Width > targetSurfaceSize.Width)
	{
		sourceSize.Width -= (targetPos.X + sourceSize.Width) - targetSurfaceSize.Width;
		if (sourceSize.Width <= 0)
			return;
	}

	if (targetPos.Y<0)
	{
		sourceSize.Height += targetPos.Y;
		if (sourceSize.Height <= 0)
			return;

		sourcePos.Y -= targetPos.Y;
		targetPos.Y = 0;
	}
	
	if (targetPos.Y + sourceSize.Height > targetSurfaceSize.Height)
	{
		sourceSize.Height -= (targetPos.Y + sourceSize.Height) - targetSurfaceSize.Height;
		if (sourceSize.Height <= 0)
			return;
	}

	// ok, we've clipped everything.
	// now draw it.

	setTexture(0, texture);	
	glColor4ub(color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha());

	if (useAlphaChannelOfTexture)
		setRenderStates2DMode(false, true, true);
	else
		setRenderStates2DMode(false, true, false);

	core::rect<s32> poss(targetPos, sourceSize);

	s32 xPlus = -(ScreenSize.Width>>1);
	f32 xFact = 1.0f / (ScreenSize.Width>>1);

	s32 yPlus = ScreenSize.Height-(ScreenSize.Height>>1);
	f32 yFact = 1.0f / (ScreenSize.Height>>1);

	const core::dimension2d<s32> sourceSurfaceSize = texture->getOriginalSize();
	core::rect<f32> tcoords;
	tcoords.UpperLeftCorner.X = (((f32)sourcePos.X)+0.5f) / texture->getOriginalSize().Width ;
	tcoords.UpperLeftCorner.Y = (((f32)sourcePos.Y)+0.5f) / texture->getOriginalSize().Height;
	tcoords.LowerRightCorner.X = (((f32)sourcePos.X +0.5f + (f32)sourceSize.Width)) / texture->getOriginalSize().Width;
	tcoords.LowerRightCorner.Y = (((f32)sourcePos.Y +0.5f + (f32)sourceSize.Height)) / texture->getOriginalSize().Height;

	core::rect<float> npos;
	npos.UpperLeftCorner.X = (f32)(poss.UpperLeftCorner.X+xPlus+0.5f) * xFact;
	npos.UpperLeftCorner.Y = (f32)(yPlus-poss.UpperLeftCorner.Y+0.5f) * yFact;
	npos.LowerRightCorner.X = (f32)(poss.LowerRightCorner.X+xPlus+0.5f) * xFact;
	npos.LowerRightCorner.Y = (f32)(yPlus-poss.LowerRightCorner.Y+0.5f) * yFact;

	glBegin(GL_QUADS);
	
	glTexCoord2f(tcoords.UpperLeftCorner.X, tcoords.UpperLeftCorner.Y);
	glVertex2f(npos.UpperLeftCorner.X, npos.UpperLeftCorner.Y);

	glTexCoord2f(tcoords.LowerRightCorner.X, tcoords.UpperLeftCorner.Y);
	glVertex2f(npos.LowerRightCorner.X, npos.UpperLeftCorner.Y);

	glTexCoord2f(tcoords.LowerRightCorner.X, tcoords.LowerRightCorner.Y);
	glVertex2f(npos.LowerRightCorner.X, npos.LowerRightCorner.Y); 

	glTexCoord2f(tcoords.UpperLeftCorner.X, tcoords.LowerRightCorner.Y);
	glVertex2f(npos.UpperLeftCorner.X, npos.LowerRightCorner.Y); 
	
	glEnd();
}


void CVideoOpenGL::draw2DImage(video::ITexture* texture, const core::rect<s32>& destRect, 
                   const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect, 
                   video::SColor* colors, bool useAlphaChannelOfTexture) 
{ 
   if (!texture) 
      return; 

   core::rect<s32> trgRect=destRect; 
   core::rect<s32> srcRect=sourceRect; 

   const core::dimension2d<s32> targetSurfaceSize=ScreenSize; 

   const core::dimension2d<s32> ss = texture->getOriginalSize(); 
   float ssw=1.0f/ss.Width; 
   float ssh=1.0f/ss.Height; 

   core::rect<f32> tcoords; 
   tcoords.UpperLeftCorner.X = (((f32)srcRect.UpperLeftCorner.X)+0.5f) * ssw ;
   tcoords.UpperLeftCorner.Y = (((f32)srcRect.UpperLeftCorner.Y)+0.5f) * ssh; 
   tcoords.LowerRightCorner.X = (((f32)srcRect.UpperLeftCorner.X +0.5f + (f32)srcRect.getWidth())) * ssw; 
   tcoords.LowerRightCorner.Y = (((f32)srcRect.UpperLeftCorner.Y +0.5f + (f32)srcRect.getHeight())) * ssh; 

   s32 xPlus = -(ScreenSize.Width>>1); 
   f32 xFact = 1.0f / (ScreenSize.Width>>1); 

   s32 yPlus = ScreenSize.Height-(ScreenSize.Height>>1); 
   f32 yFact = 1.0f / (ScreenSize.Height>>1); 

   core::rect<float> npos; 
   npos.UpperLeftCorner.X = (f32)(trgRect.UpperLeftCorner.X+xPlus+0.5f) * xFact; 
   npos.UpperLeftCorner.Y = (f32)(yPlus-trgRect.UpperLeftCorner.Y+0.5f) * yFact; 
   npos.LowerRightCorner.X = (f32)(trgRect.LowerRightCorner.X+xPlus+0.5f) * xFact; 
   npos.LowerRightCorner.Y = (f32)(yPlus-trgRect.LowerRightCorner.Y+0.5f) * yFact; 

   setTexture(0, texture);    

   if (useAlphaChannelOfTexture) 
      setRenderStates2DMode(false, true, true); 
   else 
      setRenderStates2DMode(false, true, false); 

   bool bTempColors=false; 

   if(colors==NULL) 
   { 
      colors=new SColor[4];
      for(int i=0;i<4;i++)colors[i]=SColor(0,255,255,255); 
      bTempColors=true; 
   } 

   glBegin(GL_QUADS); 

   glColor4ub(colors[0].getRed(), colors[0].getGreen(), colors[0].getBlue(), colors[0].getAlpha()); 
   glTexCoord2f(tcoords.UpperLeftCorner.X, tcoords.UpperLeftCorner.Y); 
   glVertex2f(npos.UpperLeftCorner.X, npos.UpperLeftCorner.Y); 

   glColor4ub(colors[3].getRed(), colors[3].getGreen(), colors[3].getBlue(), colors[3].getAlpha()); 
   glTexCoord2f(tcoords.LowerRightCorner.X, tcoords.UpperLeftCorner.Y); 
   glVertex2f(npos.LowerRightCorner.X, npos.UpperLeftCorner.Y); 

   glColor4ub(colors[2].getRed(), colors[2].getGreen(), colors[2].getBlue(), colors[2].getAlpha()); 
   glTexCoord2f(tcoords.LowerRightCorner.X, tcoords.LowerRightCorner.Y); 
   glVertex2f(npos.LowerRightCorner.X, npos.LowerRightCorner.Y); 

   glColor4ub(colors[1].getRed(), colors[1].getGreen(), colors[1].getBlue(), colors[1].getAlpha());
   glTexCoord2f(tcoords.UpperLeftCorner.X, tcoords.LowerRightCorner.Y); 
   glVertex2f(npos.UpperLeftCorner.X, npos.LowerRightCorner.Y); 

   glEnd(); 

   if(bTempColors) 
	   delete [] colors; 
}


//! Draws a 2d line. 
void CVideoOpenGL::draw2DLine(const core::position2d<s32>& start,
						const core::position2d<s32>& end, 
						SColor color)
{
	// thanks to Vash TheStampede who sent in his implementation 

	setRenderStates2DMode(color.getAlpha() < 255, false, false);
	setTexture(0,0);
 
	const s32 xPlus = -(ScreenSize.Width>>1);
	const f32 xFact = 1.0f / (ScreenSize.Width>>1);
	 
	const s32 yPlus =
	ScreenSize.Height-(ScreenSize.Height>>1);
	const f32 yFact = 1.0f / (ScreenSize.Height>>1);
	 
	core::position2d<f32> npos_start;
	npos_start.X  = (f32)(start.X + xPlus) * xFact;
	npos_start.Y  = (f32)(yPlus - start.Y) * yFact;
	 
	core::position2d<f32> npos_end;
	npos_end.X  = (f32)(end.X + xPlus) * xFact;
	npos_end.Y  = (f32)(yPlus - end.Y) * yFact;
	 
	glBegin(GL_LINES);
	glColor4ub(color.getRed(), color.getGreen(),
	color.getBlue(), 
	color.getAlpha());
	glVertex2f(npos_start.X, npos_start.Y);
	glVertex2f(npos_end.X,   npos_end.Y);
	glEnd();
}

//! draw an 2d rectangle
void CVideoOpenGL::draw2DRectangle(SColor color, const core::rect<s32>& position, 
		const core::rect<s32>* clip)
{
	setRenderStates2DMode(color.getAlpha() < 255, false, false);
	setTexture(0,0);

	core::rect<s32> pos = position;

	if (clip)
		pos.clipAgainst(*clip);

	s32 xPlus = -(ScreenSize.Width>>1);
	f32 xFact = 1.0f / (ScreenSize.Width>>1);

	s32 yPlus = ScreenSize.Height-(ScreenSize.Height>>1);
	f32 yFact = 1.0f / (ScreenSize.Height>>1);

	core::rect<float> npos;
	npos.UpperLeftCorner.X = (f32)(pos.UpperLeftCorner.X+xPlus) * xFact;
	npos.UpperLeftCorner.Y = (f32)(yPlus-pos.UpperLeftCorner.Y) * yFact;
	npos.LowerRightCorner.X = (f32)(pos.LowerRightCorner.X+xPlus) * xFact;
	npos.LowerRightCorner.Y = (f32)(yPlus-pos.LowerRightCorner.Y) * yFact;

	glBegin(GL_QUADS);
	glColor4ub(color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha());
	glVertex2f(npos.UpperLeftCorner.X, npos.UpperLeftCorner.Y);
	glVertex2f(npos.LowerRightCorner.X, npos.UpperLeftCorner.Y);
	glVertex2f(npos.LowerRightCorner.X, npos.LowerRightCorner.Y); 
	glVertex2f(npos.UpperLeftCorner.X, npos.LowerRightCorner.Y); 
	glEnd();
}



//! queries the features of the driver, returns true if feature is available
bool CVideoOpenGL::queryFeature(E_VIDEO_DRIVER_FEATURE feature)
{
	switch (feature)
	{
	case EVDF_BILINEAR_FILER:
		return true;
	case EVDF_RENDER_TO_TARGET:
		return false;
	case EVDF_HARDWARE_TL:
		return false;
	case EVDF_MIP_MAP:
		return true;
	case EVDF_STENCIL_BUFFER:
		return StencilBuffer;
	case EVDF_ARB_VERTEX_PROGRAM_1:
		return ARBVertexProgramExtension;
	case EVDF_ARB_FRAGMENT_PROGRAM_1:
		return ARBFragmentProgramExtension;
	};

	return false;
}


//! sets the current Texture
void CVideoOpenGL::setTexture(s32 stage, video::ITexture* texture)
{
	if (stage > 1)
		return;

	if (MultiTextureExtension)
		extGlActiveTextureARB(stage == 0 ? GL_TEXTURE0_ARB : GL_TEXTURE1_ARB);
	else
		if (stage != 0)
			return;
	
	if (texture && texture->getDriverType() != EDT_OPENGL)
	{
		glDisable(GL_TEXTURE_2D);
		os::Printer::log("Fatal Error: Tried to set a texture not owned by this driver.", ELL_ERROR);
		return;
	}

	if (texture == 0)
		glDisable(GL_TEXTURE_2D);
	else
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,
			((COpenGLTexture*)texture)->getOpenGLTextureName());

		GLint p = ClampTexture ? GL_CLAMP : GL_REPEAT;

		// TODO: reenable this. Seems to cause problems on some gfx adapters.
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, p);
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, p);
	}
}


//! returns a device dependent texture from a software surface (IImage)
//! THIS METHOD HAS TO BE OVERRIDDEN BY DERIVED DRIVERS WITH OWN TEXTURES
video::ITexture* CVideoOpenGL::createDeviceDependentTexture(IImage* surface)
{
	bool generateMipLevels = getTextureCreationFlag(ETCF_CREATE_MIP_MAPS);

	return new COpenGLTexture(surface, generateMipLevels);
}



//! Sets a material. All 3d drawing functions draw geometry now
//! using this material.
//! \param material: Material to be used from now on.
void CVideoOpenGL::setMaterial(const SMaterial& material)
{
	Material = material;

	setTexture(0, Material.Texture1);
	setTexture(1, Material.Texture2);
}



//! prints error if an error happened.
inline void CVideoOpenGL::printGLError()
{
#ifdef _DEBUG
	GLenum g = glGetError();
	switch(g)
	{
	case GL_NO_ERROR:
		break;
	case GL_INVALID_ENUM:
		os::Printer::log("GL_INVALID_ENUM", ELL_ERROR); break;
	case GL_INVALID_VALUE:
		os::Printer::log("GL_INVALID_VALUE", ELL_ERROR); break;
	case GL_INVALID_OPERATION:
		os::Printer::log("GL_INVALID_OPERATION", ELL_ERROR); break;
	case GL_STACK_OVERFLOW:
		os::Printer::log("GL_STACK_OVERFLOW", ELL_ERROR); break;
	case GL_STACK_UNDERFLOW:
		os::Printer::log("GL_STACK_UNDERFLOW", ELL_ERROR); break;
	case GL_OUT_OF_MEMORY:
		os::Printer::log("GL_OUT_OF_MEMORY", ELL_ERROR); break;
	};
#endif
}



//! sets the needed renderstates
void CVideoOpenGL::setRenderStates3DMode()
{
	if (CurrentRenderMode != ERM_3D)
	{
		// switch back the matrices
		GLfloat glmat[16];

		createGLMatrix(glmat, Matrizes[ETS_VIEW] * Matrizes[ETS_WORLD]);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(glmat);

		createGLMatrix(glmat, Matrizes[ETS_PROJECTION]);
		glmat[12]*= -1.0f;
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(glmat);

		ResetRenderStates = true;
	}

	if (ResetRenderStates)
		glFrontFace(GL_CW);

	if (ResetRenderStates || LastMaterial != Material)
	{
		// unset old material

		if (LastMaterial.MaterialType != Material.MaterialType &&
			LastMaterial.MaterialType >= 0 && LastMaterial.MaterialType < (s32)MaterialRenderers.size())
			MaterialRenderers[LastMaterial.MaterialType]->OnUnsetMaterial();

		// set new material.

		if (Material.MaterialType >= 0 && Material.MaterialType < (s32)MaterialRenderers.size())
			MaterialRenderers[Material.MaterialType]->OnSetMaterial(
				Material, LastMaterial, ResetRenderStates, this);
	}

	LastMaterial = Material;

	ResetRenderStates = false;

	CurrentRenderMode = ERM_3D;
}


//! Can be called by an IMaterialRenderer to make its work easier.
void CVideoOpenGL::setBasicRenderStates(const SMaterial& material, const SMaterial& lastmaterial,
	bool resetAllRenderstates)
{
	if (resetAllRenderstates ||
		lastmaterial.AmbientColor != material.AmbientColor ||
		lastmaterial.DiffuseColor != material.DiffuseColor ||
		lastmaterial.SpecularColor != material.SpecularColor ||
		lastmaterial.EmissiveColor != material.EmissiveColor ||
		lastmaterial.Shininess != material.Shininess)
	{
		GLfloat color[4];

		float inv = 1.0f / 255.0f;

		color[0] = Material.AmbientColor.getRed() * inv;
		color[1] = Material.AmbientColor.getGreen() * inv;
		color[2] = Material.AmbientColor.getBlue() * inv;
		color[3] = Material.AmbientColor.getAlpha() * inv;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);

		color[0] = Material.DiffuseColor.getRed() * inv;
		color[1] = Material.DiffuseColor.getGreen() * inv;
		color[2] = Material.DiffuseColor.getBlue() * inv;
		color[3] = Material.DiffuseColor.getAlpha() * inv;
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);

		color[0] = Material.SpecularColor.getRed() * inv;
		color[1] = Material.SpecularColor.getGreen() * inv;
		color[2] = Material.SpecularColor.getBlue() * inv;
		color[3] = Material.SpecularColor.getAlpha() * inv;
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);

		color[0] = Material.EmissiveColor.getRed() * inv;
		color[1] = Material.EmissiveColor.getGreen() * inv;
		color[2] = Material.EmissiveColor.getBlue() * inv;
		color[3] = Material.EmissiveColor.getAlpha() * inv;
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, color);

		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, Material.Shininess);
	}

	// bilinear

	if (resetAllRenderstates || lastmaterial.BilinearFilter != material.BilinearFilter)
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
			Material.BilinearFilter ? GL_LINEAR : GL_NEAREST);

	// fillmode

	if (resetAllRenderstates || lastmaterial.Wireframe != material.Wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, material.Wireframe ? GL_LINE : GL_FILL);

	// lighting

	if (resetAllRenderstates || lastmaterial.Lighting != material.Lighting)
	{
		if (Material.Lighting)
			glEnable(GL_LIGHTING);
		else
			glDisable(GL_LIGHTING);
	}

	// zbuffer

	if (resetAllRenderstates || lastmaterial.ZBuffer != material.ZBuffer)
	{
		if (material.ZBuffer)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	// zwrite
	if (resetAllRenderstates || lastmaterial.ZWriteEnable != material.ZWriteEnable)
			glDepthMask(material.ZWriteEnable ? GL_TRUE : GL_FALSE);

	// back face culling

	if (resetAllRenderstates || lastmaterial.BackfaceCulling != material.BackfaceCulling)
	{
		if (material.BackfaceCulling)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}

	// fog
	if (resetAllRenderstates || lastmaterial.FogEnable != material.FogEnable)
	{
		if (material.FogEnable)
			glEnable(GL_FOG);
		else
			glDisable(GL_FOG);
	}
}


//! sets the needed renderstates
void CVideoOpenGL::setRenderStates2DMode(bool alpha, bool texture, bool alphaChannel)
{
	if (CurrentRenderMode != ERM_2D || Transformation3DChanged)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		Transformation3DChanged = false;

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_FOG);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_LIGHTING);

		if (MultiTextureExtension)
			extGlActiveTextureARB(GL_TEXTURE0_ARB);

		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);

		ClampTexture = false;

		// unset last 3d material
		if (CurrentRenderMode == ERM_3D &&
			LastMaterial.MaterialType >= 0 && LastMaterial.MaterialType < (s32)MaterialRenderers.size())
			MaterialRenderers[LastMaterial.MaterialType]->OnUnsetMaterial();
	}

	if (texture)
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if (alphaChannel)
		{
			/*
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
			glDisable(GL_ALPHA_TEST); */
		 
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glDisable(GL_BLEND);
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0);			
		}
		else
		{
			if (alpha)
			{
				glDisable(GL_ALPHA_TEST);
				glEnable(GL_BLEND);
			}
			else
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				glDisable(GL_ALPHA_TEST);
				glDisable(GL_BLEND);
			}
		}

	}
	else
	{
		if (alpha)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glDisable(GL_ALPHA_TEST);
		}
		else
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glDisable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
		}
	}

	CurrentRenderMode = ERM_2D;
}


//! \return Returns the name of the video driver. Example: In case of the DirectX8
//! driver, it would return "Direct3D8.1".
const wchar_t* CVideoOpenGL::getName()
{
	return L"OpenGL1.2";
}


//! deletes all dynamic lights there are
void CVideoOpenGL::deleteAllDynamicLights()
{
	for (s32 i=0; i<LastSetLight+1; ++i)
		glDisable(GL_LIGHT0 + i);
	
	LastSetLight = -1;

	CVideoNull::deleteAllDynamicLights();
}


//! adds a dynamic light
void CVideoOpenGL::addDynamicLight(const SLight& light)
{
	++LastSetLight;
	if (!(LastSetLight < GL_MAX_LIGHTS))
		return;

	setTransform(ETS_WORLD, core::matrix4());

	CVideoNull::addDynamicLight(light);

	s32 lidx = GL_LIGHT0 + LastSetLight;
	GLfloat data[4];

	// set position
	data[0] = light.Position.X;
    data[1] = light.Position.Y;
    data[2] = light.Position.Z;
	data[3] = 1.0f;
    glLightfv(lidx, GL_POSITION, data);

	// set diffuse color
	data[0] = light.DiffuseColor.r;
	data[1] = light.DiffuseColor.g;
	data[2] = light.DiffuseColor.b;
	data[3] = light.DiffuseColor.a;
	glLightfv(lidx, GL_DIFFUSE, data);

	// set specular color
	data[0] = light.SpecularColor.r;
	data[1] = light.SpecularColor.g;
	data[2] = light.SpecularColor.b;
	data[3] = light.SpecularColor.a;
	glLightfv(lidx, GL_SPECULAR, data);

	// set ambient color
	data[0] = light.AmbientColor.r;
	data[1] = light.AmbientColor.g;
	data[2] = light.AmbientColor.b;
	data[3] = light.AmbientColor.a;
	glLightfv(lidx, GL_AMBIENT, data);

	// 1.0f / (constant + linar * d + quadratic*(d*d);

	// set attenuation
	glLightf(lidx, GL_CONSTANT_ATTENUATION, 0.0f);
	glLightf(lidx, GL_LINEAR_ATTENUATION, 1.0f / light.Radius);
	glLightf(lidx, GL_QUADRATIC_ATTENUATION, 0.0f);

	glEnable(lidx);
}


//! returns the maximal amount of dynamic lights the device can handle
s32 CVideoOpenGL::getMaximalDynamicLightAmount()
{
	return GL_MAX_LIGHTS;
}


//! Sets the dynamic ambient light color. The default color is
//! (0,0,0,0) which means it is dark.
//! \param color: New color of the ambient light.
void CVideoOpenGL::setAmbientLight(const SColorf& color)
{
	GLfloat data[4] = {color.r, color.g, color.b, color.a};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, data);
}


// this code was sent in by Oliver Klems, thank you! (I modified the glVieport
// method just a bit.
void CVideoOpenGL::setViewPort(const core::rect<s32>& area)
{
	core::rect<s32> vp = area;
	core::rect<s32> rendert(0,0, ScreenSize.Width, ScreenSize.Height);
	vp.clipAgainst(rendert);


	if (vp.getHeight()>0 && vp.getWidth()>0)
		glViewport(vp.UpperLeftCorner.X,
		           ScreenSize.Height - vp.UpperLeftCorner.Y - vp.getHeight(),
				   vp.getWidth(),
				   vp.getHeight());

	ViewPort = vp;
}


//! Draws a shadow volume into the stencil buffer. To draw a stencil shadow, do
//! this: Frist, draw all geometry. Then use this method, to draw the shadow
//! volume. Then, use IVideoDriver::drawStencilShadow() to visualize the shadow.
void CVideoOpenGL::drawStencilShadowVolume(const core::vector3df* triangles, s32 count, bool zfail)
{
	if (!StencilBuffer || !count)
		return;
	
	// store current OpenGL	state
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT |
		GL_POLYGON_BIT	| GL_STENCIL_BUFFER_BIT	);

	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_STENCIL_TEST);
	glColorMask(GL_FALSE, GL_FALSE,	GL_FALSE, GL_FALSE ); // no color buffer drawing
	glStencilFunc(GL_ALWAYS, 1,	0xFFFFFFFFL	);
	glColorMask(0, 0, 0, 0);
	glEnable(GL_CULL_FACE);

	// unset last 3d material
	if (CurrentRenderMode == ERM_3D &&
		LastMaterial.MaterialType >= 0 && LastMaterial.MaterialType < (s32)MaterialRenderers.size())
	{
		MaterialRenderers[LastMaterial.MaterialType]->OnUnsetMaterial();
		ResetRenderStates = true;
	}

	if (!zfail)
	{
		// ZPASS Method

		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		glCullFace(GL_BACK);
		glBegin(GL_TRIANGLES);

		s32 i;
		for(i =	0; i < count; ++i)
			glVertex3f(triangles[i].X, triangles[i].Y, triangles[i].Z);

		glEnd();

		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
		glCullFace(GL_FRONT);
		
		glBegin(GL_TRIANGLES);
		for(i =	0; i < count; ++i)
			glVertex3f(triangles[i].X, triangles[i].Y, triangles[i].Z);
		
		glEnd();

	}
	else
	{
		// ZFAIL Method

		glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
		glCullFace(GL_FRONT);
		
		glBegin(GL_TRIANGLES);

		s32 i;
		for(i =	0; i < count; i++)
			glVertex3f(triangles[i].X, triangles[i].Y, triangles[i].Z);
		
		glEnd();

		glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
		glCullFace(GL_BACK);
		
		glBegin(GL_TRIANGLES);

		for(i =	0; i < count; i++)
			glVertex3f(triangles[i].X, triangles[i].Y, triangles[i].Z);
		
		glEnd();

	}

	glPopAttrib();
}



//! Fills the stencil shadow with color. After the shadow volume has been drawn
//! into the stencil buffer using IVideoDriver::drawStencilShadowVolume(), use this
//! to draw the color of the shadow. 
void CVideoOpenGL::drawStencilShadow(bool clearStencilBuffer, video::SColor leftUpEdge,
			video::SColor rightUpEdge, video::SColor leftDownEdge, video::SColor rightDownEdge)
{
	if (!StencilBuffer)
		return;

	glPushAttrib( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT | GL_STENCIL_BUFFER_BIT );

	glFrontFace( GL_CCW );

	// Enable Rendering To Colour Buffer For All Components
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	glDepthMask(GL_FALSE); 
	glDisable(GL_DEPTH_TEST); 

	// Draw A Shadowing Rectangle Covering The Entire Screen

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glStencilFunc(GL_NOTEQUAL, 0, 0xFFFFFFFFL);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glEnable(GL_STENCIL_TEST); 
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_FOG);
	
	glPushMatrix();
	glLoadIdentity();
	
	glBegin(GL_TRIANGLE_STRIP);
	glColor4ub (leftUpEdge.getRed(), leftUpEdge.getGreen(), leftUpEdge.getBlue(), leftUpEdge.getAlpha() );
	glVertex3f(-10.1f, 10.1f,0.90f);

	glColor4ub (leftDownEdge.getRed(), leftDownEdge.getGreen(), leftDownEdge.getBlue(), leftDownEdge.getAlpha() );
	glVertex3f(-10.1f,-10.1f,0.90f);

	glColor4ub (rightUpEdge.getRed(), rightUpEdge.getGreen(), rightUpEdge.getBlue(), rightUpEdge.getAlpha() );
	glVertex3f( 10.1f, 10.1f,0.90f);

	glColor4ub (rightDownEdge.getRed(), rightDownEdge.getGreen(), rightDownEdge.getBlue(), rightDownEdge.getAlpha() );
	glVertex3f( 10.1f,-10.1f,0.90f);
	glEnd();

	glPopMatrix();
	glPopAttrib();

	if (clearStencilBuffer)
    	glClear(GL_STENCIL_BUFFER_BIT);

	glDepthMask(GL_TRUE); 
	glEnable(GL_DEPTH_TEST); 

}


//! Sets the fog mode.
void CVideoOpenGL::setFog(SColor c, bool linearFog, f32 start, 
	f32 end, f32 density, bool pixelFog, bool rangeFog)
{
	CVideoNull::setFog(c, linearFog, start, end, density, pixelFog, rangeFog);

	glFogi(GL_FOG_MODE, linearFog ? GL_LINEAR : GL_EXP); 

	if(linearFog)
 	{ 
		glFogf(GL_FOG_START, start);
		glFogf(GL_FOG_END, end); 
	}       
	else 
		glFogf(GL_FOG_DENSITY, density); 

	SColorf color(c);
	GLfloat data[4] = {color.r, color.g, color.b, color.a};
	glFogfv(GL_FOG_COLOR, data); 
}


//! Draws a 3d line.
void CVideoOpenGL::draw3DLine(const core::vector3df& start,
	const core::vector3df& end, SColor color)
{
	setRenderStates3DMode();

	glBegin(GL_LINES);
	glColor4ub(color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha());
	glVertex3f(start.X, start.Y, start.Z);

	glColor4ub(color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha());
	glVertex3f(end.X, end.Y, end.Z);
	glEnd();
}

//! Only used by the internal engine. Used to notify the driver that
//! the window was resized.
void CVideoOpenGL::OnResize(const core::dimension2d<s32>& size)
{
	CVideoNull::OnResize(size);
	glViewport(0, 0, size.Width, size.Height); 
}

//! Returns type of video driver
E_DRIVER_TYPE CVideoOpenGL::getDriverType()
{
	return EDT_OPENGL;
}

void CVideoOpenGL::extGlActiveTextureARB(GLenum texture)
{
#ifdef WIN32
	if (MultiTextureExtension && pGlActiveTextureARB)
		pGlActiveTextureARB(texture);
#endif
}

void CVideoOpenGL::extGlClientActiveTextureARB(GLenum texture)
{
#ifdef WIN32
	if (MultiTextureExtension && pGlClientActiveTextureARB)
		pGlClientActiveTextureARB(texture);
#endif
}

void CVideoOpenGL::extGlGenProgramsARB(GLsizei n, GLuint *programs)
{
#ifdef WIN32
	if (pGlGenProgramsARB)
		pGlGenProgramsARB(n, programs);
#endif
	// TODO: linux
}

void CVideoOpenGL::extGlBindProgramARB(GLenum target, GLuint program)
{
#ifdef WIN32
	if (pGlBindProgramARB)
		pGlBindProgramARB(target, program);
#endif
	// TODO: linux
}

void CVideoOpenGL::extGlProgramStringARB(GLenum target, GLenum format, GLsizei len, const GLvoid *string)
{
#ifdef WIN32
	if (pGlProgramStringARB)
		pGlProgramStringARB(target, format, len, string);
#endif
	// TODO: linux
}

void CVideoOpenGL::extGlDeleteProgramsARB(GLsizei n, const GLuint *programs)
{
#ifdef WIN32
	if (pglDeleteProgramsARB)
		pglDeleteProgramsARB(n, programs);
#endif
	// TODO: linux
}

void CVideoOpenGL::extGlProgramLocalParameter4fvARB(GLenum n, GLuint i, const GLfloat * f)
{
#ifdef WIN32
	if (pglProgramLocalParameter4fvARB)
		pglProgramLocalParameter4fvARB(n,i,f);
#endif
	// TODO: linux
}

bool CVideoOpenGL::hasMultiTextureExtension()
{
    return MultiTextureExtension;
}

//! Sets a vertex shader constant.
void CVideoOpenGL::setVertexShaderConstant(f32* data, s32 startRegister, s32 constantAmount)
{
	for (int i=0; i<constantAmount; ++i)
		extGlProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, startRegister+i, &data[i*4]);
}

//! Sets a pixel shader constant.
void CVideoOpenGL::setPixelShaderConstant(f32* data, s32 startRegister, s32 constantAmount)
{
	for (int i=0; i<constantAmount; ++i)
		extGlProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, startRegister+i, &data[i*4]);
}


//! Adds a new material renderer to the VideoDriver, using pixel and/or 
//! vertex shaders to render geometry.
s32 CVideoOpenGL::addShaderMaterial(const c8* vertexShaderProgram,
	const c8* pixelShaderProgram,
	IShaderConstantSetCallBack* callback,
	E_MATERIAL_TYPE baseMaterial)
{
	s32 nr = -1;
	COpenGLShaderMaterialRenderer* r = new COpenGLShaderMaterialRenderer(
		this, nr, vertexShaderProgram, pixelShaderProgram,
		callback, getMaterialRenderer(baseMaterial));

	r->drop();
	return nr;
}

//! Returns pointer to the IGPUProgrammingServices interface.
IGPUProgrammingServices* CVideoOpenGL::getGPUProgrammingServices()
{
	return this;
}




} // end namespace
} // end namespace

#endif // _IRR_COMPILE_WITH_OPENGL_


namespace irr
{
namespace video
{


// -----------------------------------
// WINDOWS VERSION
// -----------------------------------
#ifdef WIN32
IVideoDriver* createOpenGLDriver(const core::dimension2d<s32>& screenSize,
        HWND window, bool fullscreen, bool stencilBuffer, bool vsync, io::IFileSystem* io)
{
#ifdef _IRR_COMPILE_WITH_OPENGL_
	CVideoOpenGL* ogl =  new CVideoOpenGL(screenSize, window, fullscreen, stencilBuffer, io);
	if (!ogl->initDriver(screenSize, window, fullscreen, vsync))
	{
		ogl->drop();
		ogl = 0;
	}
	return ogl;	
#else
	return 0;
#endif // _IRR_COMPILE_WITH_OPENGL_
}
#endif // WIN32


// -----------------------------------
// LINUX VERSION
// -----------------------------------
#ifdef LINUX
IVideoDriver* createOpenGLDriver(const core::dimension2d<s32>& screenSize,
        bool fullscreen, bool doublebuffer, bool stencilBuffer, Window window,
		Display* display, io::IFileSystem* io)
{
#ifdef _IRR_COMPILE_WITH_OPENGL_
	return new CVideoOpenGL(screenSize, fullscreen, doublebuffer, stencilBuffer,
		window, display, io);
#else
	return 0;
#endif //  _IRR_COMPILE_WITH_OPENGL_
}
#endif // LINUX

} // end namespace
} // end namespace

