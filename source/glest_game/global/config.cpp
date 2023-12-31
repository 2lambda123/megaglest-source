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

#include "config.h"

#include "util.h"
#include "game_constants.h"
#include "platform_util.h"
#include "game_util.h"
#include <map>
#include "conversion.h"
#include "window.h"
#include <stdexcept>
#include <fstream>
#include "leak_dumper.h"

using namespace Shared::Platform;
using namespace Shared::Util;
using namespace std;

namespace Glest{ namespace Game{

int GameConstants::networkFramePeriod				= 20;
int GameConstants::updateFps						= 40;
int GameConstants::cameraFps						= 100;

const float GameConstants::normalMultiplier			= 1.0f;
const float GameConstants::easyMultiplier			= 0.5f;
const float GameConstants::ultraMultiplier			= 1.5f;
const float GameConstants::megaMultiplier			= 2.5f;

const char *GameConstants::folder_path_maps         = "maps";
const char *GameConstants::folder_path_scenarios    = "scenarios";
const char *GameConstants::folder_path_techs        = "techs";
const char *GameConstants::folder_path_tilesets     = "tilesets";
const char *GameConstants::folder_path_tutorials    = "tutorials";

const char *GameConstants::NETWORK_SLOT_UNCONNECTED_SLOTNAME = "???";
const char *GameConstants::NETWORK_SLOT_CLOSED_SLOTNAME = "Closed";

const char *GameConstants::folder_path_screenshots	= "screens/";
const char *GameConstants::folder_path_setups	= "setups/";

const char *GameConstants::OBSERVER_SLOTNAME		= "*Observer*";
const char *GameConstants::RANDOMFACTION_SLOTNAME	= "*Random*";

const char *GameConstants::steamCacheInstanceKey  			= "steamInstanceCache";
const char *GameConstants::preCacheThreadCacheLookupKey  		= "preCacheThreadCache";
const char *GameConstants::ircClientCacheLookupKey  			= "ircClientCache";
const char *GameConstants::playerTextureCacheLookupKey  		= "playerTextureCache";
const char *GameConstants::factionPreviewTextureCacheLookupKey  = "factionPreviewTextureCache";
const char *GameConstants::characterMenuScreenPositionListCacheLookupKey  = "characterMenuScreenPositionListCache";
const char *GameConstants::application_name				= "MegaGlest";

const char * GameConstants::LOADING_SCREEN_FILE 	   = "loading_screen";
const char * GameConstants::LOADING_SCREEN_FILE_FILTER = "loading_screen*.*";
const char * GameConstants::PREVIEW_SCREEN_FILE 	   = "preview_screen";
const char * GameConstants::PREVIEW_SCREEN_FILE_FILTER = "preview_screen*.*";
const char * GameConstants::HUD_SCREEN_FILE 	   	   = "hud";
const char * GameConstants::HUD_SCREEN_FILE_FILTER     = "hud*.*";


const char *GameConstants::pathCacheLookupKey           = "pathCache_";
const char *GameConstants::path_data_CacheLookupKey     = "data";
const char *GameConstants::path_ini_CacheLookupKey      = "ini";
const char *GameConstants::path_logs_CacheLookupKey     = "logs";

const char *GameConstants::saveNetworkGameFileServer			= "megaglest-saved-server.xml";
const char *GameConstants::saveNetworkGameFileServerCompressed 	= "megaglest-saved-server.zip";

const char *GameConstants::saveNetworkGameFileClient			= "megaglest-saved-client.xml";
const char *GameConstants::saveNetworkGameFileClientCompressed 	= "megaglest-saved-client.zip";

const char *GameConstants::saveGameFileDefault 			= "megaglest-saved.xml";
const char *GameConstants::saveGameFileAutoTestDefault 	= "megaglest-auto-saved_%s.xml";
const char *GameConstants::saveGameFilePattern 			= "megaglest-saved_%s.xml";

const char *Config::glest_ini_filename                  = "glest.ini";
const char *Config::glestuser_ini_filename              = "glestuser.ini";

const char *Config::glestkeys_ini_filename              = "glestkeys.ini";
const char *Config::glestuserkeys_ini_filename          = "glestuserkeys.ini";

const char *Config::ACTIVE_MOD_PROPERTY_NAME			= "current_mod_name";

 const char *Config::colorPicking = "color";
 const char *Config::selectBufPicking = "selectbuf";
 const char *Config::frustumPicking = "frustum";

map<string,string> Config::customRuntimeProperties;

// =====================================================
// 	class Config
// =====================================================

const string defaultNotFoundValue = "~~NOT FOUND~~";

map<ConfigType,Config> Config::configList;

Config::Config() {
	fileLoaded.first 			= false;
	fileLoaded.second 			= false;
	cfgType.first 				= cfgMainGame;
	cfgType.second 				= cfgUserGame;
	fileName.first 				= "";
	fileName.second 			= "";
	fileNameParameter.first 	= "";
	fileNameParameter.second 	= "";
	custom_path_parameter       = "";
}

bool Config::tryCustomPath(std::pair<ConfigType,ConfigType> &type, std::pair<string,string> &file, string custom_path) {
	bool wasFound = false;
    if((type.first == cfgMainGame && type.second == cfgUserGame &&
        file.first == glest_ini_filename && file.second == glestuser_ini_filename) ||
       (type.first == cfgMainKeys && type.second == cfgUserKeys &&
       	file.first == glestkeys_ini_filename && file.second == glestuserkeys_ini_filename)) {

    	string linuxPath = custom_path;
    	if(SystemFlags::VERBOSE_MODE_ENABLED) printf("-=-=-=-=-=-=-= looking for file in possible location  [%s]\n",linuxPath.c_str());

#if defined(__linux__)
		string nixFile = linuxPath + "linux_" + file.first;
		if(SystemFlags::VERBOSE_MODE_ENABLED) printf("-=-=-=-=-=-=-= looking for linux specific file in possible location  [%s]\n",nixFile.c_str());

    	if(wasFound == false && fileExists(nixFile) == true) {
        	file.first = nixFile;
        	file.second = linuxPath + file.second;
        	wasFound = true;
    	}
#elif defined(__WIN32__)
		string winFile = linuxPath + "windows_" + file.first;
		if(SystemFlags::VERBOSE_MODE_ENABLED) printf("-=-=-=-=-=-=-= looking for windows specific file in possible location  [%s]\n",winFile.c_str());

		if(wasFound == false && fileExists(winFile) == true) {
        	file.first = winFile;
        	file.second = linuxPath + file.second;
        	wasFound = true;
    	}

#endif
    	if(wasFound == false && fileExists(linuxPath + file.first) == true) {
        	file.first = linuxPath + file.first;
        	file.second = linuxPath + file.second;
        	wasFound = true;
    	}
    }
	return wasFound;
}

Config::Config(std::pair<ConfigType,ConfigType> type, std::pair<string,string> file, std::pair<bool,bool> fileMustExist,string custom_path) {
	fileLoaded.first 			= false;
	fileLoaded.second 			= false;
	cfgType 					= type;
	fileName 					= file;
	fileNameParameter 			= file;
	custom_path_parameter 		= custom_path;

    if(getGameReadWritePath(GameConstants::path_ini_CacheLookupKey) != "") {
    	fileName.first = getGameReadWritePath(GameConstants::path_ini_CacheLookupKey) + fileName.first;
    	fileName.second = getGameReadWritePath(GameConstants::path_ini_CacheLookupKey) + fileName.second;
    }

  	bool foundPath = false;
  	string currentpath = custom_path;

    if(custom_path != "") {
      	foundPath = tryCustomPath(cfgType, fileName, custom_path);
    }

    if(foundPath == false) {
		currentpath = extractDirectoryPathFromFile(Properties::getApplicationPath());
		foundPath = tryCustomPath(cfgType, fileName, currentpath);
    }

#if defined(CUSTOM_DATA_INSTALL_PATH)
  	if(foundPath == false) {
#ifndef NO_APPIMAGE
  		foundPath = tryCustomPath(cfgType, fileName, Properties::appendAppDirPath(formatPath(TOSTRING(CUSTOM_DATA_INSTALL_PATH))));
#else
  		foundPath = tryCustomPath(cfgType, fileName, formatPath(TOSTRING(CUSTOM_DATA_INSTALL_PATH)));
#endif
  	}
#endif

#if defined(MG_CMAKE_INSTALL_PREFIX)
  	if(foundPath == false) {
  		foundPath = tryCustomPath(cfgType, fileName, formatPath(TOSTRING(MG_CMAKE_INSTALL_PREFIX)));
  	}
#endif

// Look in standard linux shared paths for ini files
#if defined(__linux__)
    if(foundPath == false) {
    	foundPath = tryCustomPath(cfgType, fileName, "/usr/share/megaglest/");
    }
    if(foundPath == false) {
    	foundPath = tryCustomPath(cfgType, fileName, "/usr/share/games/megaglest/");
    }
    if(foundPath == false) {
    	foundPath = tryCustomPath(cfgType, fileName, "/usr/local/share/megaglest/");
    }
    if(foundPath == false) {
    	foundPath = tryCustomPath(cfgType, fileName, "/usr/local/share/games/megaglest/");
    }
#endif

    if(SystemFlags::VERBOSE_MODE_ENABLED) printf("foundPath = [%d]\n",foundPath);

    if(fileMustExist.first == true && fileExists(fileName.first) == false) {
    	//string currentpath = extractDirectoryPathFromFile(Properties::getApplicationPath());
    	fileName.first = currentpath + fileName.first;
    }

#if defined(WIN32)
	//string test = "C:\\Code\\megaglest\\mk\\windoze\\.\\..\\..\\data\\glest_game\\glest.ini";
	//updatePathClimbingParts(test);

	updatePathClimbingParts(fileName.first);
#endif

    if(SystemFlags::VERBOSE_MODE_ENABLED) printf("-=-=-=-=-=-=-= About to load fileName.first = [%s]\n",fileName.first.c_str());

    if(fileMustExist.first == true ||
    	(fileMustExist.first == false && fileExists(fileName.first) == true)) {
    	properties.first.load(fileName.first);
    	fileLoaded.first = true;
    }

    if(cfgType.first == cfgMainGame) {
        if( properties.first.getString("UserData_Root", defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
        	string userData = properties.first.getString("UserData_Root");
        	if(userData != "") {
        		endPathWithSlash(userData);
        	}
        	fileName.second = userData + fileNameParameter.second;
        }
        else if(properties.first.getString("UserOverrideFile", defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
        	string userData = properties.first.getString("UserOverrideFile");
        	if(userData != "") {
        		endPathWithSlash(userData);
        	}
        	fileName.second = userData + fileNameParameter.second;
        }

#if defined(WIN32)
		updatePathClimbingParts(fileName.second);
#endif

    }
    else if(cfgType.first == cfgMainKeys) {
        Config &mainCfg = Config::getInstance();
    	if( mainCfg.getString("UserData_Root", defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
        	string userData = mainCfg.getString("UserData_Root");
        	if(userData != "") {
        		endPathWithSlash(userData);
        	}
        	fileName.second = userData + fileNameParameter.second;
        }
        else if(mainCfg.getString("UserOverrideFile", defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
        	string userData = mainCfg.getString("UserOverrideFile");
        	if(userData != "") {
        		endPathWithSlash(userData);
        	}
        	fileName.second = userData + fileNameParameter.second;
        }

#if defined(WIN32)
	updatePathClimbingParts(fileName.second);
#endif

    }

    if(SystemFlags::VERBOSE_MODE_ENABLED) printf("-=-=-=-=-=-=-= About to load fileName.second = [%s]\n",fileName.second.c_str());

    if(fileMustExist.second == true ||
    	(fileMustExist.second == false && fileExists(fileName.second) == true)) {
    	properties.second.load(fileName.second);
    	fileLoaded.second = true;
    }

    try {
		if(fileName.second != "" && fileExists(fileName.second) == false) {
			if(SystemFlags::VERBOSE_MODE_ENABLED) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s Line: %d] attempting to auto-create cfgFile.second = [%s]\n",__FILE__,__FUNCTION__,__LINE__,fileName.second.c_str());

#if defined(WIN32) && !defined(__MINGW32__)
			wstring wstr = utf8_decode(fileName.second);
			FILE *fp = _wfopen(wstr.c_str(), L"w");
			std::ofstream userFile(fp);
#else
			std::ofstream userFile;
			userFile.open(fileName.second.c_str(), ios_base::out | ios_base::trunc);
#endif
			userFile.close();
#if defined(WIN32) && !defined(__MINGW32__)
			if(fp) {
				fclose(fp);
			}
#endif
			fileLoaded.second = true;
			properties.second.load(fileName.second);
		}
    }
    catch(const exception &ex) {
    	SystemFlags::OutputDebug(SystemFlags::debugError,"In [%s::%s Line: %d] Error [%s]\n",__FILE__,__FUNCTION__,__LINE__,ex.what());
    	if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s Line: %d] ERROR trying to auto-create cfgFile.second = [%s]\n",__FILE__,__FUNCTION__,__LINE__,fileName.second.c_str());
    }
}

Config &Config::getInstance(std::pair<ConfigType,ConfigType> type, std::pair<string,string> file, std::pair<bool,bool> fileMustExist, string custom_path) {
	if(configList.find(type.first) == configList.end()) {
		if(SystemFlags::VERBOSE_MODE_ENABLED) if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s Line: %d]\n",__FILE__,__FUNCTION__,__LINE__);

		Config config(type, file, fileMustExist, custom_path);

		if(SystemFlags::VERBOSE_MODE_ENABLED) if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s Line: %d]\n",__FILE__,__FUNCTION__,__LINE__);

		configList.insert(map<ConfigType,Config>::value_type(type.first,config));

		if(SystemFlags::VERBOSE_MODE_ENABLED) if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s Line: %d]\n",__FILE__,__FUNCTION__,__LINE__);
	}

	return configList.find(type.first)->second;
}

void Config::CopyAll(Config *src, Config *dest) {

	dest->properties	= src->properties;
	dest->cfgType		= src->cfgType;
	dest->fileName		= src->fileName;
	dest->fileNameParameter = src->fileNameParameter;
	dest->fileLoaded	= src->fileLoaded;
}

void Config::reload() {
	if(SystemFlags::VERBOSE_MODE_ENABLED) if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s Line: %d]\n",__FILE__,__FUNCTION__,__LINE__);

	std::pair<ConfigType,ConfigType> type = std::make_pair(cfgType.first,cfgType.second);
	Config newconfig(type, std::make_pair(fileNameParameter.first,fileNameParameter.second), std::make_pair(true,false), custom_path_parameter);

	if(SystemFlags::VERBOSE_MODE_ENABLED) if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s Line: %d]\n",__FILE__,__FUNCTION__,__LINE__);

	Config &oldconfig = configList.find(type.first)->second;
	CopyAll(&newconfig, &oldconfig);

	if(SystemFlags::VERBOSE_MODE_ENABLED) if(SystemFlags::getSystemSettingType(SystemFlags::debugSystem).enabled) SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s Line: %d]\n",__FILE__,__FUNCTION__,__LINE__);
}

void Config::save(const string &path){
	if(fileLoaded.second == true) {
		if(path != "") {
			fileName.second = path;
		}
		if(SystemFlags::VERBOSE_MODE_ENABLED) printf("In [%s::%s Line: %d] save file [%s]\n",__FILE__,__FUNCTION__,__LINE__,fileName.second.c_str());
		properties.second.save(fileName.second);
		return;
	}

	if(path != "") {
		fileName.first = path;
	}
	if(SystemFlags::VERBOSE_MODE_ENABLED) printf("In [%s::%s Line: %d] save file [%s]\n",__FILE__,__FUNCTION__,__LINE__,fileName.first.c_str());
	properties.first.save(fileName.first);
}

int Config::getInt(const char *key,const char *defaultValueIfNotFound) const {
	if(tempProperties.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
		return tempProperties.getInt(key,defaultValueIfNotFound);
	}
	if(fileLoaded.second == true &&
		properties.second.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
		return properties.second.getInt(key,defaultValueIfNotFound);
	}
    return properties.first.getInt(key,defaultValueIfNotFound);
}

bool Config::getBool(const char *key,const char *defaultValueIfNotFound) const {
	if(tempProperties.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
		return tempProperties.getBool(key,defaultValueIfNotFound);
	}

	if(fileLoaded.second == true &&
		properties.second.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
		return properties.second.getBool(key,defaultValueIfNotFound);
	}

	return properties.first.getBool(key,defaultValueIfNotFound);
}

float Config::getFloat(const char *key,const char *defaultValueIfNotFound) const {
	if(tempProperties.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
		return tempProperties.getFloat(key,defaultValueIfNotFound);
	}

	if(fileLoaded.second == true &&
		properties.second.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
		return properties.second.getFloat(key,defaultValueIfNotFound);
	}

	return properties.first.getFloat(key,defaultValueIfNotFound);
}

const string Config::getString(const char *key,const char *defaultValueIfNotFound) const {
	if(tempProperties.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
		return tempProperties.getString(key,defaultValueIfNotFound);
	}

	if(fileLoaded.second == true &&
		properties.second.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
		return properties.second.getString(key,defaultValueIfNotFound);
	}

	return properties.first.getString(key,defaultValueIfNotFound);
}

int Config::getInt(const string &key,const char *defaultValueIfNotFound) const{
	if(tempProperties.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
		return tempProperties.getInt(key,defaultValueIfNotFound);
	}

	if(fileLoaded.second == true &&
		properties.second.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
		return properties.second.getInt(key,defaultValueIfNotFound);
	}

	return properties.first.getInt(key,defaultValueIfNotFound);
}

bool Config::getBool(const string &key,const char *defaultValueIfNotFound) const{
	if(tempProperties.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
		return tempProperties.getBool(key,defaultValueIfNotFound);
	}

	if(fileLoaded.second == true &&
		properties.second.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
		return properties.second.getBool(key,defaultValueIfNotFound);
	}

	return properties.first.getBool(key,defaultValueIfNotFound);
}

float Config::getFloat(const string &key,const char *defaultValueIfNotFound) const{
	if(tempProperties.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
		return tempProperties.getFloat(key,defaultValueIfNotFound);
	}

	if(fileLoaded.second == true &&
		properties.second.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
		return properties.second.getFloat(key,defaultValueIfNotFound);
	}

	return properties.first.getFloat(key,defaultValueIfNotFound);
}

const string Config::getString(const string &key,const char *defaultValueIfNotFound) const{
	if(tempProperties.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
		return tempProperties.getString(key,defaultValueIfNotFound);
	}

	if(fileLoaded.second == true &&
		properties.second.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
		return properties.second.getString(key,defaultValueIfNotFound);
	}

	return properties.first.getString(key,defaultValueIfNotFound);
}

SDL_Keycode Config::translateStringToSDLKey(const string &value) const {
	SDL_Keycode result = SDLK_UNKNOWN;

	if(IsNumeric(value.c_str()) == true) {
		result = (SDL_Keycode)strToInt(value);
	}
	else if(value.substr(0,2) == "vk") {
		if(value == "vkLeft") {
			result = SDLK_LEFT;
		}
		else if(value == "vkRight") {
			result = SDLK_RIGHT;
		}
		else if(value == "vkUp") {
			result = SDLK_UP;
		}
		else if(value == "vkDown") {
			result = SDLK_DOWN;
		}
		else if(value == "vkAdd") {
			result = SDLK_PLUS;
		}
		else if(value == "vkSubtract") {
			result = SDLK_MINUS;
		}
		else if(value == "vkEscape") {
			result = SDLK_ESCAPE;
		}
		else if(value == "vkF1") {
			result = SDLK_F1;
		}
		else if(value == "vkF2") {
			result = SDLK_F2;
		}
		else if(value == "vkF3") {
			result = SDLK_F3;
		}
		else if(value == "vkF4") {
			result = SDLK_F4;
		}
		else if(value == "vkF5") {
			result = SDLK_F5;
		}
		else if(value == "vkF6") {
			result = SDLK_F6;
		}
		else if(value == "vkF7") {
			result = SDLK_F7;
		}
		else if(value == "vkF8") {
			result = SDLK_F8;
		}
		else if(value == "vkF9") {
			result = SDLK_F9;
		}
		else if(value == "vkF10") {
			result = SDLK_F10;
		}
		else if(value == "vkF11") {
			result = SDLK_F11;
		}
		else if(value == "vkF12") {
			result = SDLK_F12;
		}
		else if(value == "vkPrint") {
			result = SDLK_PRINTSCREEN;
		}
		else if(value == "vkPause") {
			result = SDLK_PAUSE;
		}
		else {
			string sError = "Unsupported key translation [" + value + "]";
			throw megaglest_runtime_error(sError.c_str());
		}
	}
	else if(value.length() >= 1) {
		if(value.length() == 3 && value[0] == '\'' && value[2] == '\'') {
			result = (SDL_Keycode)value[1];
		}
		else {
			bool foundKey = false;
			if(value.length() > 1) {
				SDL_Keycode lookup = SDL_GetKeyFromName(value.c_str());
				if(lookup != SDLK_UNKNOWN) {
					result = lookup;
					foundKey = true;
				}
			}

			if(foundKey == false) {
				result = (SDL_Keycode)value[0];
			}
		}
	}
	else {
		string sError = "Unsupported key translation" + value;
		throw megaglest_runtime_error(sError.c_str());
	}

	// Because SDL is based on lower Ascii
	//result = tolower(result);
	return result;
}

SDL_Keycode Config::getSDLKey(const char *key) const {
	if(fileLoaded.second == true &&
		properties.second.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {

		string value = properties.second.getString(key);
		return translateStringToSDLKey(value);
	}
	string value = properties.first.getString(key);
	return translateStringToSDLKey(value);
}

//char Config::getCharKey(const char *key) const {
//	if(fileLoaded.second == true &&
//		properties.second.getString(key, defaultNotFoundValue.c_str()) != defaultNotFoundValue) {
//
//		string value = properties.second.getString(key);
//		return translateStringToCharKey(value);
//	}
//	string value = properties.first.getString(key);
//	return translateStringToCharKey(value);
//}

void Config::setInt(const string &key, int value, bool tempBuffer) {
	if(tempBuffer == true) {
		tempProperties.setInt(key, value);
		return;
	}
	if(fileLoaded.second == true) {
		properties.second.setInt(key, value);
		return;
	}
	properties.first.setInt(key, value);
}

void Config::setBool(const string &key, bool value, bool tempBuffer) {
	if(tempBuffer == true) {
		tempProperties.setBool(key, value);
		return;
	}

	if(fileLoaded.second == true) {
		properties.second.setBool(key, value);
		return;
	}

	properties.first.setBool(key, value);
}

void Config::setFloat(const string &key, float value, bool tempBuffer) {
	if(tempBuffer == true) {
		tempProperties.setFloat(key, value);
		return;
	}

	if(fileLoaded.second == true) {
		properties.second.setFloat(key, value);
		return;
	}

	properties.first.setFloat(key, value);
}

void Config::setString(const string &key, const string &value, bool tempBuffer) {
	if(tempBuffer == true) {
		tempProperties.setString(key, value);
		return;
	}

	if(fileLoaded.second == true) {
		properties.second.setString(key, value);
		return;
	}

	properties.first.setString(key, value);
}

vector<pair<string,string> > Config::getPropertiesFromContainer(const Properties &propertiesObj) const {
    vector<pair<string,string> > result;

	int count = propertiesObj.getPropertyCount();
	for(int i = 0; i < count; ++i) {
	    pair<string,string> property;
        property.first  = propertiesObj.getKey(i);
        property.second = propertiesObj.getString(i);
        result.push_back(property);
	}

    return result;
}

vector<pair<string,string> > Config::getMergedProperties() const {
    vector<pair<string,string> > result = getMasterProperties();
    vector<pair<string,string> > resultUser = getUserProperties();
    for(unsigned int i = 0; i < resultUser.size(); ++i) {
        const pair<string,string> &propertyUser = resultUser[i];
        bool overrideProperty = false;
        for(unsigned int j = 0; j < result.size(); ++j) {
            pair<string,string> &property = result[j];
            // Take the user property and override the original value
            if(property.first == propertyUser.first) {
                overrideProperty = true;
                property.second = propertyUser.second;
                break;
            }
        }
        if(overrideProperty == false) {
            result.push_back(propertyUser);
        }
    }

    return result;
}

vector<pair<string,string> > Config::getMasterProperties() const {
    return getPropertiesFromContainer(properties.first);
}

vector<pair<string,string> > Config::getUserProperties() const {
    return getPropertiesFromContainer(properties.second);
}

void Config::setUserProperties(const vector<pair<string,string> > &valueList) {
	Properties &propertiesObj = properties.second;

	for(unsigned int idx = 0; idx < valueList.size(); ++ idx) {
		const pair<string,string> &nameValuePair = valueList[idx];
		propertiesObj.setString(nameValuePair.first,nameValuePair.second);
	}
}

string Config::getFileName(bool userFilename) const {
    string result = fileName.second;
	if(userFilename == false) {
		result = fileName.first;
    }

	return result;
}

string Config::toString(){
	return properties.first.toString();
}

vector<string> Config::getPathListForType(PathType type, string scenarioDir) {
    vector<string> pathList;
    string data_path = getGameReadWritePath(GameConstants::path_data_CacheLookupKey);

    string userData = getString("UserData_Root","");
    if(userData != "") {
    	endPathWithSlash(userData);
        //if(data_path == "") {
        //	userData = userData;
        //}
        //else {
        //	if(SystemFlags::VERBOSE_MODE_ENABLED) printf("extractLastDirectoryFromPath(userData) [%s] from userData [%s]\n",extractLastDirectoryFromPath(userData).c_str(),userData.c_str());
        //	userData = data_path + extractLastDirectoryFromPath(userData);
        //}
        //if(SystemFlags::VERBOSE_MODE_ENABLED) printf("In [%s::%s Line: %d] userData path [%s]\n",__FILE__,__FUNCTION__,__LINE__,userData.c_str());

        if(isdir(userData.c_str()) == false) {
        	createDirectoryPaths(userData);
        	if(SystemFlags::VERBOSE_MODE_ENABLED) printf("In [%s::%s Line: %d] creating path [%s]\n",__FILE__,__FUNCTION__,__LINE__,userData.c_str());
        }

        string userDataMaps = userData + GameConstants::folder_path_maps;
        if(isdir(userDataMaps.c_str()) == false) {
        	createDirectoryPaths(userDataMaps);
        	if(SystemFlags::VERBOSE_MODE_ENABLED) printf("In [%s::%s Line: %d] creating path [%s]\n",__FILE__,__FUNCTION__,__LINE__,userDataMaps.c_str());
        }
        string userDataScenarios = userData + GameConstants::folder_path_scenarios;
        if(isdir(userDataScenarios.c_str()) == false) {
        	createDirectoryPaths(userDataScenarios);
        	if(SystemFlags::VERBOSE_MODE_ENABLED) printf("In [%s::%s Line: %d] creating path [%s]\n",__FILE__,__FUNCTION__,__LINE__,userDataScenarios.c_str());
        }
        string userDataTechs = userData + GameConstants::folder_path_techs;
        if(isdir(userDataTechs.c_str()) == false) {
        	createDirectoryPaths(userDataTechs);
        	if(SystemFlags::VERBOSE_MODE_ENABLED) printf("In [%s::%s Line: %d] creating path [%s]\n",__FILE__,__FUNCTION__,__LINE__,userDataTechs.c_str());
        }
        string userDataTilesets = userData + GameConstants::folder_path_tilesets;
        if(isdir(userDataTilesets.c_str()) == false) {
        	createDirectoryPaths(userDataTilesets);
        	if(SystemFlags::VERBOSE_MODE_ENABLED) printf("In [%s::%s Line: %d] creating path [%s]\n",__FILE__,__FUNCTION__,__LINE__,userDataTilesets.c_str());
        }
        string userDataTutorials = userData + GameConstants::folder_path_tutorials;
        if(isdir(userDataTutorials.c_str()) == false) {
        	createDirectoryPaths(userDataTutorials);
        	if(SystemFlags::VERBOSE_MODE_ENABLED) printf("In [%s::%s Line: %d] creating path [%s]\n",__FILE__,__FUNCTION__,__LINE__,userDataTutorials.c_str());
        }
    }
    if(scenarioDir != "") {
		if(EndsWith(scenarioDir, ".xml") == true) {
			scenarioDir = extractDirectoryPathFromFile(scenarioDir);
		}

    	//string scenarioLocation = data_path + scenarioDir;
    	string scenarioLocation = scenarioDir;
    	if(SystemFlags::VERBOSE_MODE_ENABLED) printf("Scenario path [%s]\n",scenarioLocation.c_str());
        pathList.push_back(scenarioLocation);
    }

    switch(type) {
        case ptMaps:
            pathList.push_back(data_path+GameConstants::folder_path_maps);
            if(userData != "") {
                pathList.push_back(userData + string(GameConstants::folder_path_maps));
            }
            break;
        case ptScenarios:
            pathList.push_back(data_path+GameConstants::folder_path_scenarios);
            if(userData != "") {
                pathList.push_back(userData + string(GameConstants::folder_path_scenarios));
            }
            break;
        case ptTechs:
            pathList.push_back(data_path+GameConstants::folder_path_techs);
            if(userData != "") {
                pathList.push_back(userData + string(GameConstants::folder_path_techs));
            }
            break;
        case ptTilesets:
            pathList.push_back(data_path+GameConstants::folder_path_tilesets);
            if(userData != "") {
                pathList.push_back(userData + string(GameConstants::folder_path_tilesets));
            }
            break;
        case ptTutorials:
            pathList.push_back(data_path+GameConstants::folder_path_tutorials);
            if(userData != "") {
                pathList.push_back(userData + string(GameConstants::folder_path_tutorials));
            }
            break;
    }

    return pathList;
}

bool Config::replaceFileWithLocalFile(const vector<string> &dirList, string fileNamePart, string &resultToReplace) {
	bool found = false;
	for(unsigned int i = 0; i < dirList.size(); ++i) {
		string path = dirList[i];
		endPathWithSlash(path);
		string newFileName = path + fileNamePart;
		if(fileExists(newFileName) == true) {
			resultToReplace = newFileName;
			found = true;
			break;
		}
	}
	return found;
}



string Config::findValidLocalFileFromPath(string fileName) {
	string result = fileName;

	if(fileName.find("maps/") != fileName.npos ) {
		size_t pos = fileName.find("maps/");
		string fileNamePart = fileName.substr(pos+5);
		Config &config = Config::getInstance();
		vector<string> dirList = config.getPathListForType(ptMaps);
		replaceFileWithLocalFile(dirList, fileNamePart, result);

		if(SystemFlags::VERBOSE_MODE_ENABLED) printf("Found file [%s] @ " MG_SIZE_T_SPECIFIER " [%s]\nNew File [%s]\n",fileName.c_str(),pos,fileNamePart.c_str(),result.c_str());
	}
	else if(fileName.find("tilesets/") != fileName.npos ) {
		size_t pos = fileName.find("tilesets/");
		string fileNamePart = fileName.substr(pos+9);
		Config &config = Config::getInstance();
		vector<string> dirList = config.getPathListForType(ptTilesets);
		replaceFileWithLocalFile(dirList, fileNamePart, result);

		if(SystemFlags::VERBOSE_MODE_ENABLED) printf("Found file [%s] @ " MG_SIZE_T_SPECIFIER " [%s]\nNew File [%s]\n",fileName.c_str(),pos,fileNamePart.c_str(),result.c_str());
	}
	else if(fileName.find("techs/") != fileName.npos ) {
		size_t pos = fileName.find("techs/");
		string fileNamePart = fileName.substr(pos+6);
		Config &config = Config::getInstance();
		vector<string> dirList = config.getPathListForType(ptTechs);
		replaceFileWithLocalFile(dirList, fileNamePart, result);

		if(SystemFlags::VERBOSE_MODE_ENABLED) printf("Found file [%s] @ " MG_SIZE_T_SPECIFIER " [%s]\nNew File [%s]\n",fileName.c_str(),pos,fileNamePart.c_str(),result.c_str());
	}
	else if(fileName.find("scenarios/") != fileName.npos) {
		size_t pos = fileName.find("scenarios/");
		string fileNamePart = fileName.substr(pos+10);
		Config &config = Config::getInstance();
		vector<string> dirList = config.getPathListForType(ptScenarios);
		replaceFileWithLocalFile(dirList, fileNamePart, result);

		if(SystemFlags::VERBOSE_MODE_ENABLED) printf("Found file [%s] @ " MG_SIZE_T_SPECIFIER " [%s]\nNew File [%s]\n",fileName.c_str(),pos,fileNamePart.c_str(),result.c_str());
	}
	else if(fileName.find("tutorials/") != fileName.npos) {
		size_t pos = fileName.find("tutorials/");
		string fileNamePart = fileName.substr(pos+10);
		Config &config = Config::getInstance();
		vector<string> dirList = config.getPathListForType(ptTutorials);
		replaceFileWithLocalFile(dirList, fileNamePart, result);

		if(SystemFlags::VERBOSE_MODE_ENABLED) printf("Found file [%s] @ " MG_SIZE_T_SPECIFIER " [%s]\nNew File [%s]\n",fileName.c_str(),pos,fileNamePart.c_str(),result.c_str());
	}

	return result;
}


// static
string Config::getMapPath(const string &mapName, string scenarioDir, bool errorOnNotFound) {

    Config &config = Config::getInstance();
    vector<string> pathList = config.getPathListForType(ptMaps,scenarioDir);

    for(int idx = 0; idx < (int)pathList.size(); idx++) {
        string map_path = pathList[idx];
    	endPathWithSlash(map_path);

        const string mega = map_path + mapName + ".mgm";
        const string glest = map_path + mapName + ".gbm";
        if (fileExists(mega)) {
            return mega;
        }
        else if (fileExists(glest)) {
            return glest;
        }
    }

	if(errorOnNotFound == true) {
		//abort();
		throw megaglest_runtime_error("Map not found [" + mapName + "]\nScenario [" + scenarioDir + "]");
	}

	return "";
}

}}// end namespace
