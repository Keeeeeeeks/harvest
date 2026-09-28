// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "CTextReader.h"

namespace irr
{
namespace io
{


CTextReader::CTextReader(IReadFile* file)
: TextData(0), TextDataSize(0), CData(0), CDataSize(0)
{
	readFile(file);
}


//! Destructor
CTextReader::~CTextReader()
{
	delete [] CData;
}


//! Returns pointer to read text buffer
wchar_t* CTextReader::getText()
{
	return TextData;
}


//! returns size of text buffer
s32 CTextReader::getTextSize() const
{
	return TextDataSize;
}


//! reads the complete file into the buffer
void CTextReader::readFile(IReadFile* file)
{
	if (!file)
		return;

	CDataSize = file->getSize();

	if (!CDataSize)
		return;

	CDataSize+=2; // We need two terminating 0's at the end.
	             // for ASCII we need 1 0's, for UTF-16 2.

	CData = new c8[CDataSize];

	if (!file->read(CData, CDataSize-2))
	{
		delete [] CData;
		CData = 0;
		CDataSize = 0;
	}
	else
	{

		CData[CDataSize-1] = 0;
		CData[CDataSize-2] = 0;
		convertToUTF16();
	}
}



//! converts the read text data to utf16 text.
void CTextReader::convertToUTF16()
{
	// only worked with 16 bit, doen't work on some unixes. corrected 
	// version thanks to G.o.D:

	wchar_t* header = (wchar_t*)CData;
	int firstoffset=3,secondoffset=2;

	if (CDataSize>=2 && 
		((header[0]&0xffff) == 0xFFFE || (header[0]&0xffff) == 0xFEFF))
	{
		// We've got UTF-16

		if ((header[0]&0xffff) == 0xFFFE)
		{
			// This is Big Endian, 
			// switch endianess
			firstoffset = 2;
			secondoffset = 3;
		}

		TextDataSize=(CDataSize-1)/2;
		TextData= new wchar_t[TextDataSize];

		for (u32 i=0; i<TextDataSize; i++) 
			TextData[i]=(wchar_t)(CData[i*2+firstoffset]<<8)|CData[i*2+secondoffset];

		delete [] CData;
		CData = (c8*)TextData;
	}
	else
	{
		// ASCII, convert to UTF-16

		TextDataSize = CDataSize;
		convertASCIIToUTF16(CData, CDataSize, &TextData);
		delete [] CData; 
		CData = (c8*)TextData;
		return;
	}
}


//! converts ASCII to an UTF-16, output pointer should be deleted by caller
void CTextReader::convertASCIIToUTF16(const c8* characters, u32 len, wchar_t** out)
{
	(*out) = new wchar_t[len];
	for (u32 i=0; i<len; ++i)
		(*out)[i] = characters[i];
}
	

} // end namespace irr
} // end namespace io

