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

#include "main_menu.h"

#include "renderer.h"
#include "sound.h"
#include "config.h"
#include "program.h"
#include "game_util.h"
#include "game.h"
#include "platform_util.h"
#include "sound_renderer.h"
#include "core_data.h"
#include "faction.h"
#include "metrics.h"
#include "network_manager.h"
#include "network_message.h"
#include "socket.h"
#include "menu_state_root.h"
#include "video_player.h"
#include "string_utils.h"

#include "leak_dumper.h"

using namespace Shared::Sound;
using namespace Shared::Platform;
using namespace Shared::Util;
using namespace Shared::Graphics;
using namespace	Shared::Xml;

namespace Glest{ namespace Game{

// =====================================================
// 	class MainMenu
// =====================================================

// ===================== PUBLIC ========================
MenuState * MainMenu::oldstate=NULL;

MainMenu::MainMenu(Program *program) : ProgramState(program), menuBackgroundVideo(NULL) {
	if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s %d]\n",__FILE__,__FUNCTION__,__LINE__);

	//printf("In MainMenu::MainMenu()\n");

	mouseX=100;
	mouseY=100;

	state= NULL;
	this->program= program;

	if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s %d]\n",__FILE__,__FUNCTION__,__LINE__);

	Config &config = Config::getInstance();
	if(config.getString("CustomMenuTextColor","") != "") {
		string customMenuTextColor = config.getString("CustomMenuTextColor");
		Vec3f customTextColor = Vec3f::strToVec3(customMenuTextColor);
		GraphicComponent::setCustomTextColor(customTextColor);
	}

	setState(new MenuStateRoot(program, this));

	if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s %d]\n",__FILE__,__FUNCTION__,__LINE__);
}

void MainMenu::reloadUI() {
	if(state) {
		state->reloadUI();
	}
}

MainMenu::~MainMenu() {
	if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s %d]\n",__FILE__,__FUNCTION__,__LINE__);

	//printf("In MainMenu::~MainMenu()\n");

	if(menuBackgroundVideo != NULL) {
		menuBackgroundVideo->closePlayer();
		delete menuBackgroundVideo;
		menuBackgroundVideo = NULL;
	}
	delete state;
	state = NULL;
	delete oldstate;
	oldstate = NULL;

	if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s %d]\n",__FILE__,__FUNCTION__,__LINE__);

	Renderer::getInstance().endMenu();

	if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s %d]\n",__FILE__,__FUNCTION__,__LINE__);

	//SoundRenderer &soundRenderer= SoundRenderer::getInstance();
	//soundRenderer.stopAllSounds();
	GraphicComponent::setCustomTextColor(Vec3f(1.0f,1.0f,1.0f));

	if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s %d]\n",__FILE__,__FUNCTION__,__LINE__);
}

void MainMenu::init() {
	Renderer::getInstance().initMenu(this);

	initBackgroundVideo();
}

void MainMenu::initBackgroundVideo() {
	if(GlobalStaticFlags::getIsNonGraphicalModeEnabled() == false &&
		::Shared::Graphics::VideoPlayer::hasBackEndVideoPlayer() == true &&
		CoreData::getInstance().hasMainMenuVideoFilename() == true) {
		string introVideoFile = CoreData::getInstance().getMainMenuVideoFilename();
		string introVideoFileFallback = CoreData::getInstance().getMainMenuVideoFilenameFallback();

		Context *c= GraphicsInterface::getInstance().getCurrentContext();
		PlatformContextGl *glCtx = static_cast<ContextGl*>(c)->getPlatformContextGlPtr();
		SDL_Window *window = glCtx->getScreenWindow();
		SDL_Surface *screen = glCtx->getScreenSurface();

		string vlcPluginsPath = Config::getInstance().getString("VideoPlayerPluginsPath","");
		//printf("screen->w = %d screen->h = %d screen->format->BitsPerPixel = %d\n",screen->w,screen->h,screen->format->BitsPerPixel);
		menuBackgroundVideo = new VideoPlayer(
				&Renderer::getInstance(),
				introVideoFile,
				introVideoFileFallback,
				window,
				0,0,
				screen->w,
				screen->h,
				screen->format->BitsPerPixel,
				true,
				vlcPluginsPath,
				SystemFlags::VERBOSE_MODE_ENABLED);
		menuBackgroundVideo->initPlayer();
	}
}

//asynchronus render update
void MainMenu::render() {
	Renderer &renderer= Renderer::getInstance();

	canRender();
	incrementFps();

	if(state->isMasterserverMode() == false) {
		if(GlobalStaticFlags::getIsNonGraphicalModeEnabled() == false) {

			if(state->isVideoPlaying() == true) {
				if(menuBackgroundVideo != NULL) {
					if(menuBackgroundVideo->isPlaying() == true) {
						menuBackgroundVideo->closePlayer();
						delete menuBackgroundVideo;
						menuBackgroundVideo = NULL;
					}
				}
			}
			else if(menuBackgroundVideo == NULL) {
				initBackgroundVideo();
			}
			renderer.clearBuffers();

			//3d
			renderer.reset3dMenu();
			renderer.clearZBuffer();

			//printf("In [%s::%s Line: %d] menuBackgroundVideo [%p]\n",__FILE__,__FUNCTION__,__LINE__,menuBackgroundVideo);

			if(menuBackgroundVideo == NULL) {
				renderer.loadCameraMatrix(menuBackground.getCamera());
				renderer.renderMenuBackground(&menuBackground);
				renderer.renderParticleManager(rsMenu);
			}

			//2d
			renderer.reset2d();

			if(menuBackgroundVideo != NULL) {
				if(menuBackgroundVideo->isPlaying() == true) {
					menuBackgroundVideo->playFrame(false);
				}
				else {
					menuBackgroundVideo->RestartVideo();
				}
			}
		}
		state->render();

		if(GlobalStaticFlags::getIsNonGraphicalModeEnabled() == false) {
			renderer.renderMouse2d(mouseX, mouseY, mouse2dAnim);
			renderer.renderFPSWhenEnabled(lastFps);
			renderer.swapBuffers();
		}
	}
}

//syncronus update
void MainMenu::update(){
	if(menuBackgroundVideo == NULL) {
		Renderer::getInstance().updateParticleManager(rsMenu);
	}
	mouse2dAnim= (mouse2dAnim +1) % Renderer::maxMouse2dAnim;
	if(menuBackgroundVideo == NULL) {
		menuBackground.update();
	}
	state->update();
}

//event magangement: mouse click
void MainMenu::mouseMove(int x, int y, const MouseState *ms){
    mouseX= x; mouseY= y;
	state->mouseMove(x, y, ms);
}

void MainMenu::eventMouseWheel(int x, int y,int zDelta){
	state->eventMouseWheel(x, y, zDelta);
}

//returns if exiting
void MainMenu::mouseDownLeft(int x, int y){
	if(GraphicComponent::getFade()<0.2f) return;
	state->mouseClick(x, y, mbLeft);
}

void MainMenu::mouseDoubleClickLeft(int x, int y){
	if(GraphicComponent::getFade()<0.2f) return;
	state->mouseDoubleClick(x, y, mbLeft);
}

void MainMenu::mouseDoubleClickRight(int x, int y){
	if(GraphicComponent::getFade()<0.2f) return;
	state->mouseDoubleClick(x, y, mbRight);
}

void MainMenu::mouseDownRight(int x, int y){
	if(GraphicComponent::getFade()<0.2f) return;
	state->mouseClick(x, y, mbRight);
}

void MainMenu::mouseUpLeft(int x, int y){
	if(GraphicComponent::getFade()<0.2f) return;
	state->mouseUp(x, y, mbLeft);
}

bool MainMenu::textInput(std::string text) {
	return state->textInput(text);
}
void MainMenu::keyDown(SDL_KeyboardEvent key) {
	state->keyDown(key);
}

void MainMenu::keyUp(SDL_KeyboardEvent key) {
	state->keyUp(key);
}

void MainMenu::keyPress(SDL_KeyboardEvent c) {
	state->keyPress(c);
}

void MainMenu::setState(MenuState *newstate) {
    //printf("In [%s::%s Line: %d] oldstate [%p] newstate [%p] this->state [%p]\n",__FILE__,__FUNCTION__,__LINE__,oldstate,newstate,this->state);

	//printf("In MainMenu::setState() #1\n");

	if(oldstate != NULL && oldstate != newstate) {
		MenuState *oldstatePtr = oldstate;
		delete oldstate;

		//printf("In MainMenu::setState() #2\n");

        //printf("In [%s::%s Line: %d]\n",__FILE__,__FUNCTION__,__LINE__);
		if(oldstatePtr != this->state) {
			oldstate=this->state;
			//printf("In [%s::%s Line: %d]\n",__FILE__,__FUNCTION__,__LINE__);
			//printf("In MainMenu::setState() #3\n");
		}
		else {
			oldstate = NULL;
			//printf("In [%s::%s Line: %d]\n",__FILE__,__FUNCTION__,__LINE__);
			//printf("In MainMenu::setState() #4\n");
		}
	}
	else {
		oldstate=this->state;
		//printf("In [%s::%s Line: %d]\n",__FILE__,__FUNCTION__,__LINE__);
		//printf("In MainMenu::setState() #5\n");
	}

	if(this->state != NULL) {

	}

	//printf("In [%s::%s Line: %d]\n",__FILE__,__FUNCTION__,__LINE__);
	this->state= newstate;

	GraphicComponent::resetFade();
	if(newstate) {
		menuBackground.setTargetCamera(newstate->getCamera());
	}
}

bool MainMenu::isInSpecialKeyCaptureEvent() {
	return state->isInSpecialKeyCaptureEvent();
}

void MainMenu::consoleAddLine(string line) {
	if(state != NULL) {
		state->consoleAddLine(line);
	}
}

// =====================================================
// 	class MenuState
// =====================================================

MenuState::MenuState(Program *program, MainMenu *mainMenu, const string &stateName) {
	if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s %d]\n",__FILE__,__FUNCTION__,__LINE__);
	this->containerName = "MenuState";
	this->program= program;
	this->mainMenu= mainMenu;
	console.registerGraphicComponent(containerName, "menuStateConsole");

	if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s %d]\n",__FILE__,__FUNCTION__,__LINE__);
	//switch on menu music again, it might be muted
	Config &config = Config::getInstance();
	float configVolume = (config.getInt("SoundVolumeMusic") / 100.f);
	CoreData::getInstance().getMenuMusic()->setVolume(configVolume);

	string data_path = getGameReadWritePath(GameConstants::path_data_CacheLookupKey);

	//camera
	XmlTree xmlTree;
	xmlTree.load(getGameCustomCoreDataPath(data_path, "data/core/menu/menu.xml"),Properties::getTagReplacementValues());
	const XmlNode *menuNode= xmlTree.getRootNode();
	const XmlNode *cameraNode= menuNode->getChild("camera");

	if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s %d]\n",__FILE__,__FUNCTION__,__LINE__);

	//position
	const XmlNode *positionNode= cameraNode->getChild(stateName + "-position");
	Vec3f startPosition;
    startPosition.x= positionNode->getAttribute("x")->getFloatValue();
	startPosition.y= positionNode->getAttribute("y")->getFloatValue();
	startPosition.z= positionNode->getAttribute("z")->getFloatValue();
	camera.setPosition(startPosition);

	if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s %d]\n",__FILE__,__FUNCTION__,__LINE__);

	//rotation
	const XmlNode *rotationNode= cameraNode->getChild(stateName + "-rotation");
	Vec3f startRotation;
    startRotation.x= rotationNode->getAttribute("x")->getFloatValue();
	startRotation.y= rotationNode->getAttribute("y")->getFloatValue();
	startRotation.z= rotationNode->getAttribute("z")->getFloatValue();
	camera.setOrientation(Quaternion(EulerAngles(
		degToRad(startRotation.x),
		degToRad(startRotation.y),
		degToRad(startRotation.z))));
		
	if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s %d]\n",__FILE__,__FUNCTION__,__LINE__);
}

MenuState::~MenuState() {
	if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s %d]\n",__FILE__,__FUNCTION__,__LINE__);
	GraphicComponent::clearRegisteredComponents(this->containerName);
	if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s %d]\n",__FILE__,__FUNCTION__,__LINE__);
}

void MenuState::consoleAddLine(string line) {
	bool onlyWantChatMsgs = console.getOnlyChatMessagesInStoredLines();
	if(onlyWantChatMsgs == true) {
		console.setOnlyChatMessagesInStoredLines(false);
	}
	console.addLine(line);
	if(onlyWantChatMsgs == true) {
		console.setOnlyChatMessagesInStoredLines(true);
	}
}

void MenuState::reloadUI() {
	//console.resetFonts();
}

void MenuState::setActiveInputLabel(GraphicLabel *newLabel, GraphicLabel **activeInputLabelPtr) {
	GraphicLabel *activeInputLabelEdit = *activeInputLabelPtr;
	if(newLabel != NULL) {
		if(newLabel == activeInputLabelEdit) {
			return;
		}
		string text= newLabel->getText();
		size_t found = text.find_last_of("_");
		if (found == string::npos || found != text.length()-1) {
			text += "_";
		}
		newLabel->setText(text);
		//textCharLength = extractKeyPressedUnicodeLength(text);
		newLabel->setTextCharLengthList(extractKeyPressedUnicodeLength(text));
	}
	if(activeInputLabelEdit != NULL && activeInputLabelEdit->getText().empty() == false) {
		string text= activeInputLabelEdit->getText();
		size_t found = text.find_last_of("_");
		if (found != string::npos && found == text.length()-1) {
			//printf("Removing trailing edit char, found = %d [%d][%s]\n",found,text.length(),text.c_str());
			text = text.substr(0,found);
		}
		activeInputLabelEdit->setText(text);
		//textCharLength = extractKeyPressedUnicodeLength(text);
		activeInputLabelEdit->setTextCharLengthList(extractKeyPressedUnicodeLength(text));
		activeInputLabelEdit->setEditModeEnabled(false);
	}
	if(newLabel != NULL) {
		*activeInputLabelPtr = newLabel;
		newLabel->setEditModeEnabled(true);
	}
	else {
		*activeInputLabelPtr = NULL;
	}
}


bool MenuState::keyPressEditLabel(SDL_KeyboardEvent c, GraphicLabel **activeInputLabelPtr){
	if(isKeyPressed(SDLK_ESCAPE,c,false) == true ||
			isKeyPressed(SDLK_RETURN,c,false) == true ) {
		GraphicLabel *activeInputLabel = *activeInputLabelPtr;
		setActiveInputLabel(NULL,activeInputLabelPtr);
		//textCharLength.clear();
		activeInputLabel->clearTextCharLengthList();
		return true;
	}
	return false;
}

bool MenuState::textInputEditLabel(string input, GraphicLabel **activeInputLabelPtr) {
	bool eventHandled = false;
	GraphicLabel *activeInputLabel = *activeInputLabelPtr;
	if(activeInputLabel != NULL) {
		int maxTextSize= activeInputLabel->getMaxEditWidth();

		//if((c>='0' && c<='9') || (c>='a' && c<='z') || (c>='A' && c<='Z') ||
		//   (c=='-') || (c=='(') || (c==')')) {
		//if(isAllowedInputTextKey(key))
		{
			if((int)activeInputLabel->getTextCharLengthList().size() < maxTextSize) {
				string text= activeInputLabel->getText();

				if(text.size() > 0) {
					size_t found = text.find_last_of("_");
					if (found == string::npos || found != text.length()-1) {
						text += input;
						activeInputLabel->addTextCharLengthToList(input.length());
					}
					else {
						text = text.substr(0,found) + input + "_";
						//int lastCharLen = activeInputLabel->getTextCharLengthList()[activeInputLabel->getTextCharLengthList().size()-1];
						activeInputLabel->deleteTextCharLengthFromList();
						activeInputLabel->addTextCharLengthToList(input.length());
						activeInputLabel->addTextCharLengthToList(1);
					}
				}
				else {
					text = input;
					activeInputLabel->addTextCharLengthToList(input.length());
				}
				//delete [] utfStr;

				activeInputLabel->setText(text);

				eventHandled = true;
			}
		}
	}
	return eventHandled;
}

bool MenuState::keyDownEditLabel(SDL_KeyboardEvent c, GraphicLabel **activeInputLabelPtr) {
	bool eventHandled = false;
	GraphicLabel *activeInputLabel = *activeInputLabelPtr;
	if(activeInputLabel != NULL) {
		string text = activeInputLabel->getText();
		if(isKeyPressed(SDLK_BACKSPACE,c) == true && text.length() > 0) {
			//printf("BSPACE text [%s]\n",text.c_str());

			bool hasUnderscore = false;
			bool delChar = false;
			size_t found = text.find_last_of("_");
			if (found == string::npos || found != text.length()-1) {
				//printf("A text.length() = %d textCharLength.size() = %d\n",text.length(),textCharLength.size());
				if(activeInputLabel->getTextCharLengthList()[activeInputLabel->getTextCharLengthList().size()-1] >= 1) {
					delChar = true;
				}
			}
			else {
				//printf("B text.length() = %d textCharLength.size() = %d\n",text.length(),textCharLength.size());
				hasUnderscore = true;
				if(activeInputLabel->getTextCharLengthList().size() >= 2 && activeInputLabel->getTextCharLengthList()[activeInputLabel->getTextCharLengthList().size()-2] >= 1) {
					delChar = true;
				}
			}
			if(delChar == true) {
				if(hasUnderscore) {
					//if(textCharLength.size() > 1) {
					if(activeInputLabel->getTextCharLengthList().size() > 1) {
						//printf("Underscore erase start\n");
						for(unsigned int i = 0; i < (unsigned int)activeInputLabel->getTextCharLengthList()[activeInputLabel->getTextCharLengthList().size()-2]; ++i) {
							text.erase(text.end() -2);
						}
						//printf("AFTER DEL textCharLength.size() = %d textCharLength[textCharLength.size()-1] = %d text.length() = %d\n",textCharLength.size(),textCharLength[textCharLength.size()-1],text.length());
						activeInputLabel->deleteTextCharLengthFromList();
						activeInputLabel->deleteTextCharLengthFromList();
						activeInputLabel->addTextCharLengthToList(1);
					}
				}
				else {
					for(unsigned int i = 0; i < (unsigned int)activeInputLabel->getTextCharLengthList()[activeInputLabel->getTextCharLengthList().size()-1]; ++i) {
						text.erase(text.end() -1);
					}
					activeInputLabel->deleteTextCharLengthFromList();
				}
			}
			activeInputLabel->setText(text);
			eventHandled = true;
		}
	}
	return eventHandled;
}

}}//end namespace
