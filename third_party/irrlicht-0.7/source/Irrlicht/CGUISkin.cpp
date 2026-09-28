// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "CGUISkin.h"
#include "IGUIFont.h"

namespace irr
{
namespace gui
{

CGUISkin::CGUISkin(EGUI_SKIN_TYPE type)
: Font(0)
{
	#ifdef _DEBUG
	setDebugName("CGUISkin");
	#endif


	Colors[EGDC_3D_DARK_SHADOW] =	video::SColor(101,50,50,50);
	Colors[EGDC_3D_SHADOW] =		video::SColor(101,130,130,130);
	Colors[EGDC_3D_FACE] =			video::SColor(101,210,210,210);
	Colors[EGDC_3D_HIGH_LIGHT] =	video::SColor(101,255,255,255);
	Colors[EGDC_3D_LIGHT] =			video::SColor(101,210,210,210);
	Colors[EGDC_ACTIVE_BORDER] =	video::SColor(101,16,14,115);
	Colors[EGDC_ACTIVE_CAPTION] =	video::SColor(101,255,255,255);
	Colors[EGDC_APP_WORKSPACE] =	video::SColor(101,100,100,100);
	Colors[EGDC_BUTTON_TEXT] =		video::SColor(101,0,0,0);
	Colors[EGDC_GRAY_TEXT] =		video::SColor(101,130,130,130);
	Colors[EGDC_HIGH_LIGHT] =		video::SColor(101,8,36,107);
	Colors[EGDC_HIGH_LIGHT_TEXT] =	video::SColor(101,255,255,255);
	Colors[EGDC_INACTIVE_BORDER] =	video::SColor(101,165,165,165);
	Colors[EGDC_INACTIVE_CAPTION] = video::SColor(101,210,210,210);
	Colors[EGDC_TOOLTIP] =			video::SColor(101,255,255,230);
	Colors[EGDC_SCROLLBAR] =		video::SColor(101,230,230,230);
	Colors[EGDC_WINDOW] =			video::SColor(101,255,255,255);

	Sizes[EGDS_SCROLLBAR_SIZE] = 14;
	Sizes[EGDS_MENU_HEIGHT] = 18;
	Sizes[EGDS_WINDOW_BUTTON_WIDTH] = 15;
	Sizes[EGDS_CHECK_BOX_WIDTH] = 18;
	Sizes[EGDS_MESSAGE_BOX_WIDTH] = 500;
	Sizes[EGDS_MESSAGE_BOX_HEIGHT] = 200;
	Sizes[EGDS_BUTTON_WIDTH] = 80;
	Sizes[EGDS_BUTTON_HEIGHT] = 30;

	Texts[EGDT_MSG_BOX_OK] = L"OK";
	Texts[EGDT_MSG_BOX_CANCEL] = L"Cancel";
	Texts[EGDT_MSG_BOX_YES] = L"Yes";
	Texts[EGDT_MSG_BOX_NO] = L"No";
}


//! destructor
CGUISkin::~CGUISkin()
{
	if (Font)
		Font->drop();
}



//! returns default color
video::SColor CGUISkin::getColor(EGUI_DEFAULT_COLOR color)
{
	return Colors[color];
}


//! sets a default color
void CGUISkin::setColor(EGUI_DEFAULT_COLOR which, video::SColor newColor)
{
	if (which>=0 && which<= EGDC_COUNT)
		Colors[which] = newColor;
}


//! returns default color
s32 CGUISkin::getSize(EGUI_DEFAULT_SIZE size)
{
	return Sizes[size];
}



//! sets a default size
void CGUISkin::setSize(EGUI_DEFAULT_SIZE which, s32 size)
{
	if (which >= 0 && which <= EGDS_COUNT) 
		Sizes[which] = size;
}



//! returns the default font
IGUIFont* CGUISkin::getFont()
{
	return Font;
}


//! sets a default font
void CGUISkin::setFont(IGUIFont* font)
{
	if (Font)
		Font->drop();

	Font = font;

	if (Font)
		Font->grab();
}


//! Returns a default text. For example for Message box button captions:
//! "OK", "Cancel", "Yes", "No" and so on.
const wchar_t* CGUISkin::getDefaultText(EGUI_DEFAULT_TEXT text)
{
	return Texts[text].c_str();
}


//! Sets a default text. For example for Message box button captions:
//! "OK", "Cancel", "Yes", "No" and so on.
void CGUISkin::setDefaultText(EGUI_DEFAULT_TEXT which, const wchar_t* newText)
{
	Texts[which] = newText;
}


//! creates a color skin
IGUISkin* createSkin(EGUI_SKIN_TYPE type)
{
	return new CGUISkin(type);
}


} // end namespace gui
} // end namespace irr

