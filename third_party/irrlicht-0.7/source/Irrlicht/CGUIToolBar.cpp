// Copyright (C) 2002-2004 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "CGUIToolBar.h"
#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IVideoDriver.h"
#include "IGUIButton.h"
#include "IGUIFont.h"
#include "CGUIButton.h"

namespace irr
{
namespace gui
{

//! constructor
CGUIToolBar::CGUIToolBar(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
:IGUIToolBar(environment, parent, id, rectangle), ButtonX(5)
{
	// calculate position and find other menubars
	s32 y = 0;
	s32 parentwidth = 100;

	if (parent)
	{
		parentwidth = Parent->getAbsolutePosition().getWidth();

		const core::list<IGUIElement*>& children = parent->getChildren();
        core::list<IGUIElement*>::Iterator it = children.begin();

		for (; it != children.end(); ++it)
		{
			core::rect<s32> r = (*it)->getAbsolutePosition();
			if (r.UpperLeftCorner.X == 0 && r.UpperLeftCorner.Y <= y &&
				r.LowerRightCorner.X == parentwidth)
				y = r.LowerRightCorner.Y;
		}
	}

	RelativeRect.UpperLeftCorner.X = 0;
	RelativeRect.UpperLeftCorner.Y = y;
	s32 height = 30;

	/*IGUISkin* skin = Environment->getSkin();
	IGUIFont* font = skin->getFont();
	if (font)
	{
		s32 t = font->getDimension(L"A").Height + 5;
		if (t > height)
			height = t;
	}*/

	RelativeRect.LowerRightCorner.X = parentwidth;
	RelativeRect.LowerRightCorner.Y = RelativeRect.UpperLeftCorner.Y + height;
	updateAbsolutePosition();
}



//! destructor
CGUIToolBar::~CGUIToolBar()
{
}



//! draws the element and its children
void CGUIToolBar::draw()
{
	if (!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	IGUIFont* font = skin->getFont();
	IGUIFont* defaultFont = Environment->getBuiltInFont();
	video::IVideoDriver* driver = Environment->getVideoDriver();

	core::rect<s32> rect = AbsoluteRect;
	core::rect<s32>* clip = 0;

	// draw frame

	/*rect.UpperLeftCorner.X = AbsoluteRect.UpperLeftCorner.X;
	rect.UpperLeftCorner.Y = AbsoluteRect.LowerRightCorner.Y - 1;
	rect.LowerRightCorner.Y = AbsoluteRect.LowerRightCorner.Y;
	rect.LowerRightCorner.X = AbsoluteRect.LowerRightCorner.X;
	driver->draw2DRectangle(skin->getColor(EGDC_3D_DARK_SHADOW), rect, clip);

	rect.UpperLeftCorner.X += 1;
	rect.UpperLeftCorner.Y -= 1;
	rect.LowerRightCorner.Y -= 1;
	driver->draw2DRectangle(skin->getColor(EGDC_3D_SHADOW), rect, clip);

	rect = AbsoluteRect;
	rect.LowerRightCorner.Y -= 2;
	driver->draw2DRectangle(skin->getColor(EGDC_3D_FACE), rect, clip);*/

	rect.UpperLeftCorner.X = AbsoluteRect.UpperLeftCorner.X;
	rect.UpperLeftCorner.Y = AbsoluteRect.LowerRightCorner.Y - 1;
	rect.LowerRightCorner.Y = AbsoluteRect.LowerRightCorner.Y;
	rect.LowerRightCorner.X = AbsoluteRect.LowerRightCorner.X;
	driver->draw2DRectangle(skin->getColor(EGDC_3D_SHADOW), rect, clip);

	rect = AbsoluteRect;
	rect.LowerRightCorner.Y -= 1;
	driver->draw2DRectangle(skin->getColor(EGDC_3D_FACE), rect, clip);

	IGUIElement::draw();
}


//! Updates the absolute position.
void CGUIToolBar::updateAbsolutePosition()
{
	if (Parent)
		RelativeRect.LowerRightCorner.X = Parent->getAbsolutePosition().getWidth();

	IGUIElement::updateAbsolutePosition();
}


//! Adds a button to the tool bar
IGUIButton* CGUIToolBar::addButton(s32 id, const wchar_t* text,
	video::ITexture* img, video::ITexture* pressed, bool isPushButton)
{
	ButtonX += 3;

	core::rect<s32> rectangle(ButtonX,2,0,0);
	rectangle.LowerRightCorner.X = rectangle.UpperLeftCorner.X + 23;
	rectangle.LowerRightCorner.Y = rectangle.UpperLeftCorner.Y + 22;

	ButtonX += rectangle.getWidth();

	IGUIButton* button = new CGUIButton(Environment, this, id, rectangle);
	button->drop();

	if (text)
		button->setText(text);

	if (img)
		button->setImage(img);

	if (pressed)
		button->setPressedImage(pressed);

	if (isPushButton)
		button->setIsPushButton(isPushButton);
	
	return button;
}
	
} // end namespace gui
} // end namespace irr
