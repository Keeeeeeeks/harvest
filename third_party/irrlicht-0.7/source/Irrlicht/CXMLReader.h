// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_XML_READER_H_INCLUDED__
#define __C_XML_READER_H_INCLUDED__

#include "IUnknown.h"
#include "IReadFile.h"
#include "CTextReader.h"
#include "IXMLReader.h"
#include "irrString.h"
#include "array.h"

namespace irr
{
namespace io
{
	//! XML Reader providing easy read access to an XML file.
	/** Read IXMLReader for more information.
	*/
	class CXMLReader : public IXMLReader
	{
	public:

		CXMLReader(CTextReader* reader);

		//! Destructor
		virtual ~CXMLReader();

		//! Reads forward to the next xml node. 
		//! \return Returns false, if there was no further node. 
		virtual bool read();

		//! Returns the type of the current XML node.
		virtual EXML_NODE getNodeType();

        //! Returns attribute count of the current XML node.
		virtual s32 getAttributeCount();

		//! Returns name of an attribute.
		virtual const wchar_t* getAttributeName(s32 idx);

		//! Returns the value of an attribute. 
		virtual const wchar_t* getAttributeValue(s32 idx);

		//! Returns the value of an attribute. 
		virtual const wchar_t* getAttributeValue(const wchar_t* name);

		//! Returns the value of an attribute
		virtual const wchar_t* getAttributeValueSafe(const wchar_t* name);

		//! Returns the value of an attribute as integer. 
		virtual s32 getAttributeValueAsInt(const wchar_t* name);

		//! Returns the value of an attribute as float. 
		virtual f32 getAttributeValueAsFloat(const wchar_t* name);

		//! Returns the name of the current node.
		virtual const wchar_t* getNodeName();

		//! Returns data of the current node.
		virtual const wchar_t* getNodeData();

		//! Returns if an element is an empty element, like <foo />
		virtual bool isEmptyElement();

		struct XMLSpecialCharacters
		{
			wchar_t Character;
			wchar_t* Symbol;
		};

	private:

		void parseCurrentNode();

		//! sets the state that text was found
		void setText(wchar_t* start, wchar_t* end);

		//! ignores an xml definition like <?xml something />
		void ignoreDefinition();

		//! parses a comment
		void parseComment();

		//! parses an opening xml element and reads attributes
		void parseOpeningXMLElement();

		//! parses an closing xml tag
		void parseClosingXMLElement();

		inline bool isWhiteSpace(wchar_t c)
		{
			return (c==L' ' || c==L'\t' || c==L'\n' || c==L'\r');
		}

		CTextReader* Reader;
		wchar_t* P;
		wchar_t* TextBegin;
		u32 TextSize;

		EXML_NODE CurrentNodeType;
		core::stringw NodeName;
		bool IsEmptyElement;

		struct SAttribute
		{
			core::stringw Name;
			core::stringw Value;
		};

		SAttribute* getAttributeByName(const wchar_t* name);
		core::stringw replaceSpecialCharacters(core::stringw& str);

		core::array<SAttribute> Attributes;
	};

} // end namespace irr
} // end namespace io

#endif

