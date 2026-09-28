// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __IRR_COMPILE_CONFIG_H_INCLUDED__
#define __IRR_COMPILE_CONFIG_H_INCLUDED__

//! The defines for different operating system are:
//! _XBOX for XBox
//! WIN32 for Windows
//! OS2 for OS/2
//! MACOS for MacOs
//! LINUX for linux and unix (it is defined here if no other os is defined)

#if !defined(WIN32) && !defined(_XBOX) && !defined(OS2) && !defined(MACOS)
#define LINUX
#endif


//! Define _IRR_COMPILE_WITH_DIRECTX_8_ and _IRR_COMPILE_WITH_DIRECTX_9_compile 
//! the irrlicht engine with DirectX8 and/or DirectX9.
/** If you only want to use the software device or opengl this can be useful.
This switch is mostly disabled because people do not get the g++ compiler compile
 directX header files, and directX is only available on windows platforms.**/
#if (defined(_WIN32) || defined(_XBOX)) && !defined(__GNUC__)

#define _IRR_COMPILE_WITH_DIRECTX_8_
#define _IRR_COMPILE_WITH_DIRECTX_9_

#endif

//! Define _IRR_COMPILE_WITH_OPENGL_ to compile the irrlicht engine with OpenGL.
/** If you do not wish to be the engine to compiled with OpengGL, comment this
define. */
#if !defined(_XBOX)

#define _IRR_COMPILE_WITH_OPENGL_

#endif // ! _XBOX

//! Define _IRR_COMPILE_WITH_ZLIB_ to enable compiling the engine using zlib.
/** This enables the engine to read from compressed .zip archives. If you
disable this feature, the engine can still read archives, but only uncompressed
ones. */
#define _IRR_COMPILE_WITH_ZLIB_

//! Define _IRR_COMPILE_WITH_JPEGLIB_ to enable compiling the engine using libjpeg.
/** This enables the engine to read jpeg images. If you comment this out,
the engine will no longer read .jpeg images. */
#define _IRR_COMPILE_WITH_LIBJPEG_

#endif
