// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_GUI_SKIN_H_INCLUDED__
#define __C_GUI_SKIN_H_INCLUDED__

#include "IGUISkin.h"
#include "irrString.h"

namespace irr
{
namespace gui
{

	class CGUISkin : public IGUISkin
	{
	public:

		CGUISkin(EGUI_SKIN_TYPE type);

		//! destructor
		~CGUISkin();

		//! returns default color
		virtual video::SColor getColor(EGUI_DEFAULT_COLOR color);

		//! sets a default color
		virtual void setColor(EGUI_DEFAULT_COLOR which, video::SColor newColor);

		//! returns default color
		virtual s32 getSize(EGUI_DEFAULT_SIZE size);

		//! sets a default size
		virtual void setSize(EGUI_DEFAULT_SIZE which, s32 size);

		//! returns the default font
		virtual IGUIFont* getFont();

		//! sets a default font
		virtual void setFont(IGUIFont* font);

		//! Returns a default text. For example for Message box button captions:
		//! "OK", "Cancel", "Yes", "No" and so on.
		virtual const wchar_t* getDefaultText(EGUI_DEFAULT_TEXT text);

		//! Sets a default text. For example for Message box button captions:
		//! "OK", "Cancel", "Yes", "No" and so on.
		virtual void setDefaultText(EGUI_DEFAULT_TEXT which, const wchar_t* newText);

	private:

		video::SColor Colors[EGDC_COUNT];
		s32 Sizes[EGDS_COUNT];
		IGUIFont* Font;
		core::stringw Texts[EGDT_COUNT];
	};




} // end namespace gui
} // end namespace irr

#endif

