// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "CGUIStaticText.h"
#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IGUIFont.h"
#include "IVideoDriver.h"
#include "rect.h"

namespace irr
{
namespace gui
{



//! constructor
CGUIStaticText::CGUIStaticText(const wchar_t* text, bool border, IGUIEnvironment* environment, IGUIElement* parent, s32 id, const core::rect<s32>& rectangle)
: IGUIStaticText(environment, parent, id, rectangle), Border(border),
	OverrideFont(0), OverrideColorEnabled(false), WordWarp(false),
	LastBreakFont(0)
{
	#ifdef _DEBUG
	setDebugName("CGUIStaticText");
	#endif

	OverrideColor = video::SColor(101,255,255,255);
	Text = text;
}


//! destructor
CGUIStaticText::~CGUIStaticText()
{
	if (OverrideFont)
		OverrideFont->drop();
}


//! draws the element and its children
void CGUIStaticText::draw()
{
	if (!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	irr::video::IVideoDriver* driver = Environment->getVideoDriver();

	core::rect<s32> frameRect(AbsoluteRect);

	// draw the border

	if (Border)
	{
		frameRect.LowerRightCorner.Y = frameRect.UpperLeftCorner.Y + 1;
		driver->draw2DRectangle(skin->getColor(EGDC_3D_SHADOW), frameRect, &AbsoluteClippingRect);

		frameRect.LowerRightCorner.Y = AbsoluteRect.LowerRightCorner.Y;
		frameRect.LowerRightCorner.X = frameRect.UpperLeftCorner.X + 1;
		driver->draw2DRectangle(skin->getColor(EGDC_3D_SHADOW), frameRect, &AbsoluteClippingRect);

		frameRect = AbsoluteRect;
		frameRect.UpperLeftCorner.X = frameRect.LowerRightCorner.X - 1;
		driver->draw2DRectangle(skin->getColor(EGDC_3D_HIGH_LIGHT), frameRect, &AbsoluteClippingRect);

		frameRect = AbsoluteRect;
		frameRect.UpperLeftCorner.Y = AbsoluteRect.LowerRightCorner.Y - 1;
		frameRect.LowerRightCorner.Y = AbsoluteRect.LowerRightCorner.Y;
		driver->draw2DRectangle(skin->getColor(EGDC_3D_HIGH_LIGHT), frameRect, &AbsoluteClippingRect);

		frameRect = AbsoluteRect;
		frameRect.UpperLeftCorner.X += 3;
	}


	// draw the text
	if (Text.size())
	{
		IGUIFont* font = OverrideFont;
		if (!OverrideFont)
			font = skin->getFont();

		if (font)
		{
			if (!WordWarp)
				font->draw(Text.c_str(), frameRect, 
					OverrideColorEnabled ? OverrideColor : skin->getColor(EGDC_BUTTON_TEXT),
					false, true, &AbsoluteClippingRect);
			else
			{
				if (font != LastBreakFont)
					breakText();

				core::rect<s32> r = frameRect;
				s32 height = font->getDimension(L"A").Height;

				for (u32 i=0; i<BrokenText.size(); ++i)
				{
					font->draw(BrokenText[i].c_str(), r,
						OverrideColorEnabled ? OverrideColor : skin->getColor(EGDC_BUTTON_TEXT),
						false, false, &AbsoluteClippingRect);

					r.LowerRightCorner.Y += height;
					r.UpperLeftCorner.Y += height;
				}
			}
		}
	}

	IGUIElement::draw();
}



//! Sets another skin independent font.
void CGUIStaticText::setOverrideFont(IGUIFont* font)
{
	if (OverrideFont)
		OverrideFont->drop();

	OverrideFont = font;

	if (OverrideFont)
		OverrideFont->grab();

	breakText();
}


//! Sets another color for the text.
void CGUIStaticText::setOverrideColor(video::SColor color)
{
	OverrideColor = color;
	OverrideColorEnabled = true;
}



//! Sets if the static text should use the overide color or the
//! color in the gui skin.
void CGUIStaticText::enableOverrideColor(bool enable)
{
	OverrideColorEnabled = enable;
}


//! Enables or disables word wrap for using the static text as
//! multiline text control.
void CGUIStaticText::setWordWrap(bool enable)
{
	WordWarp = enable;
	breakText();
}


//! Breaks the single text line.
void CGUIStaticText::breakText()
{
	IGUISkin* skin = Environment->getSkin();

	if (!WordWarp || !skin)
		return;

	BrokenText.clear();

	IGUIFont* font = OverrideFont;
	if (!OverrideFont)
		font = skin->getFont();

	if (!font)
		return;

	LastBreakFont = font;

	core::stringw line;
	core::stringw word;
	core::stringw whitespace;
	s32 size = Text.size();
	s32 length = 0;
	s32 elWidth = RelativeRect.getWidth() - 6;
	wchar_t c;

	for (s32 i=0; i<size; ++i)
	{
		c = Text[i];
		bool lineBreak = false;

		if (c == L'\n')
		{
			lineBreak = true;
			c = ' ';
		}
		
		if (c == L' ' || c == 0)
		{
			
			if (word.size())
			{
				// here comes the next whitespace, look if 
				// we can break the last word to the next line.
				s32 whitelgth = font->getDimension(whitespace.c_str()).Width;
				s32 worldlgth = font->getDimension(word.c_str()).Width;

				if (length + worldlgth + whitelgth > elWidth)
				{
					// break to next line
					length = worldlgth;
					BrokenText.push_back(line);
					line = word;
				}
				else
				{
					// add word to line
					line += whitespace;
					line += word;
					length += whitelgth + worldlgth;
				}

				word = L"";
				whitespace = L"";
			}

			whitespace += c;

			// compute line break
			if (lineBreak)
			{
				line += whitespace;
				line += word;
				BrokenText.push_back(line);
				line = L"";
				word = L"";
				whitespace = L"";
				length = 0;
			}
		}
		else
		{
			// yippee this is a word..
			word += c;
		}
	}

	line += whitespace; 
	line += word; 
	BrokenText.push_back(line);
}


//! Sets the new caption of this element.
void CGUIStaticText::setText(const wchar_t* text)
{
	IGUIElement::setText(text);
	breakText();
}


//! Returns the height of the text in pixels when it is drawn.
s32 CGUIStaticText::getTextHeight()
{
	IGUISkin* skin = Environment->getSkin();

	if (!skin)
		return 0;

	IGUIFont* font = OverrideFont;
	if (!OverrideFont)
		font = skin->getFont();

	if (!font)
		return 0;

	s32 height = font->getDimension(L"A").Height;

	if (WordWarp)
		height *= BrokenText.size();

	return height;
}



} // end namespace gui
} // end namespace irr

