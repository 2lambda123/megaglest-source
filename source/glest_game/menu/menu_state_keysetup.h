// ==============================================================
//	This file is part of Glest (www.glest.org)
//
//	Copyright (C) 2011-  by Titus Tscharntke
//
//	You can redistribute this code and/or modify it under
//	the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the
//	License, or (at your option) any later version
// ==============================================================

#ifndef _GLEST_GAME_MENUSTATEKEYSETUP_H_
#define _GLEST_GAME_MENUSTATEKEYSETUP_H_

#include "main_menu.h"
#include "server_line.h"
#include "leak_dumper.h"

namespace Glest{ namespace Game{

// ===============================
// 	class
// ===============================
typedef vector<GraphicButton*> UserButtons;
typedef vector<GraphicLabel*> GraphicLabels;

class MenuStateKeysetup: public MenuState {

private:

	GraphicButton buttonOk;
	GraphicButton buttonDefaults;
	GraphicButton buttonReturn;

	GraphicButton buttonKeyboardSetup; // configure the keyboard
	GraphicButton buttonVideoSection;
	GraphicButton buttonAudioSection;
	GraphicButton buttonMiscSection;
	GraphicButton buttonNetworkSettings;

	GraphicLabel labelTitle;

	GraphicScrollBar keyScrollBar;
	UserButtons keyButtons;
	GraphicLabels labels;
	int keyButtonsToRender;
	int keyButtonsYBase;
	int keyButtonsXBase;
	int keyButtonsLineHeight;
	int	keyButtonsHeight;
	int keyButtonsWidth;

	GraphicMessageBox mainMessageBox;
	int mainMessageBoxState;
	vector<pair<string,string> > mergedProperties;
	vector<pair<string,string> > masterProperties;
	vector<pair<string,string> > userProperties;

	int hotkeyIndex;
	//char hotkeyChar;
	SDL_Keycode hotkeyChar;

	GraphicLabel labelTestTitle;
	GraphicLabel labelTestValue;

	ProgramState **parentUI;

public:
	MenuStateKeysetup(Program *program, MainMenu *mainMenu, ProgramState **parentUI=NULL);
	virtual ~MenuStateKeysetup();

	void mouseClick(int x, int y, MouseButton mouseButton);
	void mouseDoubleClick(int x, int y, MouseButton mouseButton){};
	void mouseUp(int x, int y, const MouseButton mouseButton);
	void mouseMove(int x, int y, const MouseState *mouseState);
	void update();
	void render();

	virtual void keyDown(SDL_KeyboardEvent key);
    virtual void keyPress(SDL_KeyboardEvent c);
    virtual void keyUp(SDL_KeyboardEvent key);

	virtual bool isInSpecialKeyCaptureEvent() { return true; }

	//static void setDisplayMessageFunction(DisplayMessageFunction pDisplayMessage) { pCB_DisplayMessage = pDisplayMessage; }

	void reloadUI();

private:
	void showMessageBox(const string &text, const string &header, bool toggle);
	void clearUserButtons();
	void cleanup();
};


}}//end namespace

#endif
