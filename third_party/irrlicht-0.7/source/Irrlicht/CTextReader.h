// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_TEXT_READER_H_INCLUDED__
#define __C_TEXT_READER_H_INCLUDED__

#include "IUnknown.h"
#include "IReadFile.h"

namespace irr
{
namespace io
{
	//! Text Reader providing easy access text files, no matter if they are
	//! in ASCII or UTF-16 format. UTF-8 is currently not supported yet,
	//! but may be added later.
	class CTextReader : public IUnknown
	{
	public:

		CTextReader(IReadFile* file);

		//! Destructor
		virtual ~CTextReader();

		//! Returns pointer to read text buffer
		wchar_t* getText();

		//! returns size of text buffer
		s32 getTextSize() const;

	private:

		//! reads the complete file into the buffer
		void readFile(IReadFile* file);

		//! converts the read text data to utf16 text.
		void convertToUTF16();

		//! converts ASCII to an UTF-16, output pointer should be deleted by caller
		void convertASCIIToUTF16(const c8* characters, u32 len, wchar_t** out);

		//! converts an big endian to little endian
		inline unsigned short convertBEtoLE (unsigned short be)
		{
			return (be >> 8) | (be << 8);
		}

		char* CData;					// read data from file
		wchar_t* TextData;			// pointer to textdata
		unsigned int TextDataSize;	// size of wchat_t buffer without header
		unsigned int CDataSize;		// site of char buffer
	};

} // end namespace irr
} // end namespace io

#endif

