// ==============================================================
//	This file is part of Glest (www.glest.org)
//
//	Copyright (C) 2001-2005 Martio Figueroa
//
//	You can redistribute this code and/or modify it under 
//	the terms of the GNU General Public License as published 
//	by the Free Software Foundation; either version 2 of the 
//	License, or (at your option) any later version
// ==============================================================

#ifndef _GLEST_GAME_MENUSTATEGRAPHICINFO_H_
#define _GLEST_GAME_MENUSTATEGRAPHICINFO_H_

#include "main_menu.h"
#include "leak_dumper.h"

using namespace Shared::Graphics::Gl;

namespace Glest{ namespace Game{

// ===============================
// 	class MenuStateGraphicInfo  
// ===============================

class MenuStateGraphicInfo: public MenuState {
private:
	GraphicButton buttonReturn;
	GraphicLabel labelInfo;
	GraphicLabel labelMoreInfo;

	GraphicLabel labelInternalInfo;

	//string glInfo;
	//string glMoreInfo;

public:
	MenuStateGraphicInfo(Program *program, MainMenu *mainMenu);

	void mouseClick(int x, int y, MouseButton mouseButton);
	void mouseDoubleClick(int x, int y, MouseButton mouseButton){};
	void mouseMove(int x, int y, const MouseState *mouseState);
	void render();
	virtual void keyDown(SDL_KeyboardEvent key);

	virtual void reloadUI();
};

}}//end namespace

#endif
