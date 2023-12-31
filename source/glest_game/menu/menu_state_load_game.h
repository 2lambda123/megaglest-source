// ==============================================================
//	This file is part of Glest (www.glest.org)
//
//	Copyright (C) 2001-2008 Martiño Figueroa
//
//	You can redistribute this code and/or modify it under 
//	the terms of the GNU General Public License as published 
//	by the Free Software Foundation; either version 2 of the 
//	License, or (at your option) any later version
// ==============================================================

#ifndef _GLEST_GAME_MENUSTATELOADGAME_H_
#define _GLEST_GAME_MENUSTATELOADGAME_H_

#include "main_menu.h"
#include "leak_dumper.h"

namespace Glest{ namespace Game{

// ===============================
// 	class MenuStateLoadGame
// ===============================
//typedef vector<GraphicButton*> SaveSlotButtons;
class MenuStateLoadGame: public MenuState{
private:
	GraphicButton loadButton;
	GraphicButton deleteButton;
	GraphicButton abortButton;
	vector<GraphicButton*> slots;
	vector<GraphicComponent*> slotsGB;
	vector<string> filenames;
	GraphicScrollBar slotsScrollBar;
	GraphicButton* selectedButton;

	GraphicButton* buttonToDelete;

	Texture2D *previewTexture;

	GraphicLabel headerLabel;
	GraphicLabel noSavedGamesLabel;
	GraphicLabel savedGamesLabel;
	GraphicLabel infoHeaderLabel;
	GraphicLabel infoTextLabel;
	GraphicLabel versionWarningLabel;

	GraphicLine lines[2];

	GraphicMessageBox mainMessageBox;

	string saveGameDir;
	int slotLinesYBase;
	int slotsLineHeight;

public:
	MenuStateLoadGame(Program *program, MainMenu *mainMenu);
	~MenuStateLoadGame();

	void mouseClick(int x, int y, MouseButton mouseButton);
	void mouseDoubleClick(int x, int y, MouseButton mouseButton);
	void mouseUp(int x, int y, const MouseButton mouseButton);
	void mouseMove(int x, int y, const MouseState *mouseState);
	void update();
	void render();
	virtual void keyDown(SDL_KeyboardEvent key);

	void reloadUI();

private:

	void clearSlots();
	void listFiles();

	void showMessageBox(const string &text, const string &header, bool toggle);

	void cleanupTexture(Texture2D **texture);
};


}}//end namespace

#endif
