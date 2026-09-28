// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "CXMLReader.h"
#include <math.h>
#include "fast_atof.h"
#include <string.h>
#include <wchar.h>

namespace irr
{
namespace io
{


CXMLReader::CXMLReader(CTextReader* reader)
: Reader(reader), P(0), TextBegin(0), TextSize(0), CurrentNodeType(EXN_NONE)
{
	if (Reader)
	{
		Reader->grab();
		P = Reader->getText();
		TextBegin = P;
		TextSize = Reader->getTextSize();
	}
}


//! Destructor
CXMLReader::~CXMLReader()
{
	if (Reader)
		Reader->drop();
}


//! Reads forward to the next xml node. 
//! \return Returns false, if there was no further node. 
bool CXMLReader::read()
{
	if ((u32)(P - TextBegin) < TextSize - 1 && *P != 0)
	{
		parseCurrentNode();
		//++P;
		return true;
	}

	return false;
}


void CXMLReader::parseCurrentNode()
{
    wchar_t* start = P;

	while(*P != L'<' && *P)
		++P;

	if (!*P)
		return;

	if (P - start > 2)
	{
		// we found some text
		setText(start, P);
		--P;
		return;
	}

	++P;

	switch(*P)
	{
	case L'/':
		parseClosingXMLElement(); break;
	case L'?':
		ignoreDefinition();	break;
	case L'!':
		parseComment();	break;
	default:
		parseOpeningXMLElement();
		break;
	}
}


//! sets the state that text was found
void CXMLReader::setText(wchar_t* start, wchar_t* end)
{
    core::stringw s(start, (s32)(end - start));
	NodeName = replaceSpecialCharacters(s);
	CurrentNodeType = EXN_TEXT;
}



//! ignores an xml definition like <?xml something />
void CXMLReader::ignoreDefinition()
{
	CurrentNodeType = EXN_UNKNOWN;

	while(*P != L'>')
		++P;

	++P;
}


//! parses a comment
void CXMLReader::parseComment()
{
	CurrentNodeType = EXN_COMMENT;
	P += 1;

	wchar_t *pCommentBegin = P;

	s32 count = 1;

	while(count)
	{
		if (*P == L'>')
			--count;
		else
		if (*P == L'<')
			++count;

		++P;
	}

	P -= 3;
	NodeName = core::stringw(pCommentBegin+2, (s32)(P - pCommentBegin-2));
	P += 3;
}


//! parses an opening xml element and reads attributes
void CXMLReader::parseOpeningXMLElement()
{
	CurrentNodeType = EXN_ELEMENT;
	IsEmptyElement = false;
	Attributes.clear();

	// find name
	const wchar_t* startName = P;

	while(*P != L'>' && !isWhiteSpace(*P))
		++P;

	const wchar_t* endName = P;

	// find Attributes
	while(*P != L'>')
	{
		if (isWhiteSpace(*P))
			++P;
		else
		{
			if (*P != L'/')
			{
				// we've got an attribute

				// read the attribute names
				const wchar_t* attributeNameBegin = P;

				while(!isWhiteSpace(*P) && *P != L'=')
					++P;

				const wchar_t* attributeNameEnd = P;
				++P;

				// read the attribute value
				while(*P != L'\"')
					++P;

				++P;
				const wchar_t* attributeValueBegin = P;
				
				while(*P != L'\"')
					++P;

				const wchar_t* attributeValueEnd = P;
				++P;

				SAttribute attr;
				attr.Name = core::stringw(attributeNameBegin, attributeNameEnd - attributeNameBegin);
				core::stringw s(attributeValueBegin, attributeValueEnd - attributeValueBegin);
				attr.Value = replaceSpecialCharacters(s);
				Attributes.push_back(attr);
			}
			else
			{
				// tag is closed directly
				++P;
				IsEmptyElement = true;
				break;
			}
		}
	}

	NodeName = core::stringw(startName, (s32)(endName - startName));

	++P;
}


//! parses an closing xml tag
void CXMLReader::parseClosingXMLElement()
{
	CurrentNodeType = EXN_ELEMENT_END;
	IsEmptyElement = false;
	Attributes.clear();

	++P;
	const wchar_t* pBeginClose = P;

	while(*P != L'>')
		++P;

	NodeName = core::stringw(pBeginClose, (s32)(P - pBeginClose));
	++P;
}



//! Returns the type of the current XML node.
EXML_NODE CXMLReader::getNodeType()
{
	return CurrentNodeType;
}


//! Returns attribute count of the current XML node.
s32 CXMLReader::getAttributeCount()
{
	return Attributes.size();
}


//! Returns name of an attribute.
const wchar_t* CXMLReader::getAttributeName(s32 idx)
{
	if (idx < 0 || idx >= (s32)Attributes.size())
		return 0;

	return Attributes[idx].Name.c_str();
}


//! Returns the value of an attribute. 
const wchar_t* CXMLReader::getAttributeValue(s32 idx)
{
	if (idx < 0 || idx >= (s32)Attributes.size())
		return 0;

	return Attributes[idx].Value.c_str();
}


//! Returns the value of an attribute. 
const wchar_t* CXMLReader::getAttributeValue(const wchar_t* name)
{
	SAttribute* attr = getAttributeByName(name);
	if (!attr)
		return 0;

	return attr->Value.c_str();
}


//! Returns the value of an attribute
const wchar_t* CXMLReader::getAttributeValueSafe(const wchar_t* name)
{
	SAttribute* attr = getAttributeByName(name);
	if (!attr)
		return L"";

	return attr->Value.c_str();
}



//! Returns the value of an attribute as integer. 
s32 CXMLReader::getAttributeValueAsInt(const wchar_t* name)
{
	return (s32)getAttributeValueAsFloat(name);
}


//! Returns the value of an attribute as float. 
f32 CXMLReader::getAttributeValueAsFloat(const wchar_t* name)
{
	SAttribute* attr = getAttributeByName(name);
	if (!attr)
		return 0;

	core::stringc c = attr->Value.c_str();
	return core::fast_atof(c.c_str());
}


//! Returns the name of the current node.
const wchar_t* CXMLReader::getNodeName()
{
	return NodeName.c_str();
}


//! Returns data of the current node.
const wchar_t* CXMLReader::getNodeData()
{
	return NodeName.c_str();
}


//! Returns if an element is an empty element, like <foo />
bool CXMLReader::isEmptyElement()
{
	return IsEmptyElement;
}


CXMLReader::SAttribute* CXMLReader::getAttributeByName(const wchar_t* name)
{
	if (!name)
		return 0;

	core::stringw n = name;

	for (s32 i=0; i<(s32)Attributes.size(); ++i)
		if (Attributes[i].Name == n)
			return &Attributes[i];

	return 0;
}

const CXMLReader::XMLSpecialCharacters XMLRSChar[] = 
{
	// all start with &
	{ L'&', L"amp;" },
	{ L'<', L"lt;" },
	{ L'>', L"gt;" },
	{ L'"', L"quot;" },
	{ L' ', 0}
};

core::stringw CXMLReader::replaceSpecialCharacters(core::stringw& origstr)
{
	s32 pos = origstr.findFirst(L'&');
	s32 oldPos = 0;

	if (pos == -1)
		return origstr;

	core::stringw newstr;

	while(pos != -1 && pos < origstr.size()-2)
	{
		// check if it is one of the special characters

		s32 specialChar = -1;
		for (s32 i=0; XMLRSChar[i].Symbol != 0; ++i)
		{
			const wchar_t* p = &origstr.c_str()[pos]+1;
			if (!wcsncmp(p, XMLRSChar[i].Symbol, wcslen(XMLRSChar[i].Symbol)))
			{
				specialChar = i;
				break;
			}
		}

		if (specialChar != -1)
		{
			newstr.append(origstr.subString(oldPos, pos - oldPos));
			newstr.append(XMLRSChar[specialChar].Character);
			pos += wcslen(XMLRSChar[specialChar].Symbol) + 1;
		}
		else
		{
			newstr.append(origstr.subString(oldPos, pos - oldPos + 1));
			pos += 1;
		}

		// find next &
		oldPos = pos;
		pos = origstr.findNext(L'&', pos);		
	}

	if (oldPos < origstr.size()-1)
		newstr.append(origstr.subString(oldPos, origstr.size()-oldPos));

	return newstr;
}

} // end namespace irr
} // end namespace io

