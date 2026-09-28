// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "CFileList.h"
#include "IrrCompileConfig.h"

namespace irr
{
namespace io
{

#ifdef LINUX
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#ifdef WIN32
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <direct.h>
#endif


CFileList::CFileList()
{
	// --------------------------------------------
	// Windows version
	#ifdef WIN32

	char tmp[_MAX_PATH];
	_getcwd(tmp, _MAX_PATH);
	Path = tmp;

	struct _finddata_t c_file;
	s32 hFile;
	FileEntry entry;

	if( (hFile = _findfirst( "*", &c_file )) != -1L )
	{
		do
		{
			entry.Name = c_file.name;
			entry.Size = c_file.size;
			entry.isDirectory = (_A_SUBDIR & c_file.attrib) != 0;
			Files.push_back(entry);
		}
		while( _findnext( hFile, &c_file ) == 0 );

		_findclose( hFile );
	}

	//TODO add drives
	//entry.Name = "E:\\";
	//entry.isDirectory = true;
	//Files.push_back(entry);
	#endif
	
	// --------------------------------------------
	// Linux version
	#ifdef LINUX
	struct dirent **namelist;
	FileEntry entry;

	s32 n = scandir(".", &namelist, 0, alphasort);
	if (n >= 0)
	{
		while(n--)
		{
			entry.Name = namelist[n]->d_name;
			entry.Size = 0;
			entry.isDirectory = namelist[n]->d_type == DT_DIR;
			Files.push_back(entry);
			free(namelist[n]);
		}
		free(namelist);
	}
	#endif
}


CFileList::~CFileList()
{
}


s32 CFileList::getFileCount()
{
	return Files.size();
}


const c8* CFileList::getFileName(s32 index)
{
	if (index < 0 || index > (s32)Files.size())
		return 0;

	return Files[index].Name.c_str();
}


//! Gets the full name of a file in the list, path included, based on an index.
const c8* CFileList::getFullFileName(s32 index)
{
	if (index < 0 || index > (s32)Files.size())
		return 0;

	if (Files[index].FullName.size() < Files[index].Name.size())
	{
		// create full name
		Files[index].FullName = Path;

		if (Path.size() > 3)
			Files[index].FullName.append("/");

		Files[index].FullName.append(Files[index].Name);
	}

	return Files[index].FullName.c_str();
}


bool CFileList::isDirectory(s32 index)
{
	if (index < 0 || index > (s32)Files.size())
		return false;

	return Files[index].isDirectory;
}

} // end namespace irr
} // end namespace io

