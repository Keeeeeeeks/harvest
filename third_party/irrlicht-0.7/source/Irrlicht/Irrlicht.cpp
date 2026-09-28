// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#if defined(WIN32) || defined(_XBOX)
 
#ifdef WIN32 
	#include <windows.h>
#endif

#ifdef _XBOX
	#include <xtl.h>
#endif

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#include <Irrlicht.h>

#pragma comment(exestr, "Irrlicht Engine (c) 2002-2004 Nikolaus Gebhardt")

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			#ifdef _DEBUG
				_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
			#endif
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

#endif // ifdef _WIN32
