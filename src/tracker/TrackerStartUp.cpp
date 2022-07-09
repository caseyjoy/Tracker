/*
 *  tracker/TrackerStartUp.cpp
 *
 *  Copyright 2009 Peter Barth
 *
 *  This file is part of Milkytracker.
 *
 *  Milkytracker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Milkytracker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Milkytracker.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 *  TrackerStartUp.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sun Mar 20 2005.
 *
 */

#include <iostream>

#include <toml.hpp>

#include "Tracker.h"
#include "XMFile.h"
#include "TrackerSettingsDatabase.h"
#include "PPSystem.h"
#include "Screen.h"
#include "PatternEditorControl.h"
#include "PlayerMaster.h"
#include "SystemMessage.h"
#include "version.h"

// Logo picture
#if defined(__EXCLUDE_BIGLOGO__) || defined(__LOWRES__)
	#include "LogoSmall.h"
#else
	#include "LogoBig.h"
#endif

// I will probably need to update the resolution in place though
/*PPSize Tracker::getWindowSizeFromDatabase() {
	PPSize size(PPScreen::getDefaultWidth(), PPScreen::getDefaultHeight());

	const std::string fname("tracker_test.toml");
	const toml::value data = toml::parse(fname);

	// This isn't really ideal, it should load in the whole settings file
	// before setting the resolution
	toml::value resolution = toml::find(data, "settings", "resolution");
	settingsDatabase->store("XRESOLUTION", toml::find<int> (resolution, "width"));
	settingsDatabase->store("YRESOLUTION", toml::find<int> (resolution, "height"));

	size.height = settingsDatabase->restore("YRESOLUTION")->getIntValue();
	size.width  = settingsDatabase->restore("XRESOLUTION")->getIntValue();

	return size;
}*/
PPSize Tracker::getWindowSizeFromDatabase() {
	// TODO: Fall back to old config file if the new one didn't exist / didn't have a resolution?
	PPSize size(PPScreen::getDefaultWidth(), PPScreen::getDefaultHeight());

	size.width  = settingsDatabase->restore("XRESOLUTION")->getIntValue();
	size.height = settingsDatabase->restore("YRESOLUTION")->getIntValue();

	std::cout << "###" << size.height << "\n";

	return size;
}


/* bool Tracker::getFullScreenFlagFromDatabase() {
	bool fullScreen = false;
	
	if (XMFile::exists(System::getConfigFileName()))
	{
		TrackerSettingsDatabase* settingsDatabaseCopy = new TrackerSettingsDatabase(*settingsDatabase);		
		XMFile f(System::getConfigFileName());	
		settingsDatabaseCopy->serialize(f);			
		fullScreen = settingsDatabaseCopy->restore("FULLSCREEN")->getBoolValue();
		delete settingsDatabaseCopy;
	}

	return fullScreen;
} */
bool Tracker::getFullScreenFlagFromDatabase() {	
	bool fullScreen = false;
	fullScreen = settingsDatabase->restore("FULLSCREEN")->getBoolValue();
	return fullScreen;
}

/* pp_int32 Tracker::getScreenScaleFactorFromDatabase()
{
	pp_int32 scaleFactor = 1;
	
	if (XMFile::exists(System::getConfigFileName()))
	{
		TrackerSettingsDatabase* settingsDatabaseCopy = new TrackerSettingsDatabase(*settingsDatabase);		
		XMFile f(System::getConfigFileName());	
		settingsDatabaseCopy->serialize(f);			
		scaleFactor = settingsDatabaseCopy->restore("SCREENSCALEFACTOR")->getIntValue();
		delete settingsDatabaseCopy;
	}

	return scaleFactor;
} */
pp_int32 Tracker::getScreenScaleFactorFromDatabase() {
	pp_int32 scaleFactor = 1;
	scaleFactor = settingsDatabase->restore("SCREENSCALEFACTOR")->getIntValue();
	return scaleFactor;
}

/* bool Tracker::getShowSplashFlagFromDatabase() {
	bool showSplash = true;
	
	if (XMFile::exists(System::getConfigFileName()))
	{
		TrackerSettingsDatabase* settingsDatabaseCopy = new TrackerSettingsDatabase(*settingsDatabase);		
		XMFile f(System::getConfigFileName());	
		settingsDatabaseCopy->serialize(f);			
		showSplash = settingsDatabaseCopy->restore("SHOWSPLASH")->getBoolValue();
		delete settingsDatabaseCopy;
	}

	return showSplash;
} */
bool Tracker::getShowSplashFlagFromDatabase() {
	bool showSplash = true;
	showSplash = settingsDatabase->restore("SHOWSPLASH")->getBoolValue();
	return showSplash;
}

#define SPLASH_WAIT_TIME 1000

void Tracker::showSplash()
{
	screen->clear();
	float shade = 0.0f;
	pp_int32 deltaT = 100;
	while (shade <= 256.0f)
	{
		pp_int32 startTime = ::PPGetTickCount();
#if defined(__EXCLUDE_BIGLOGO__) || defined(__LOWRES__)
		screen->paintSplash(LogoSmall::rawData, LogoSmall::width, LogoSmall::height, LogoSmall::width*4, 4, (int)shade); 		
#else
		screen->paintSplash(LogoBig::rawData, LogoBig::width, LogoBig::height, LogoBig::width*3, 3, (int)shade); 		
#endif
		shade+=deltaT * (1.0f/6.25f);
		deltaT = abs((pp_int32)::PPGetTickCount() - startTime);
		if (!deltaT) deltaT++;
	}
#if defined(__EXCLUDE_BIGLOGO__) || defined(__LOWRES__)
	screen->paintSplash(LogoSmall::rawData, LogoSmall::width, LogoSmall::height, LogoSmall::width*4, 4); 		
#else
	screen->paintSplash(LogoBig::rawData, LogoBig::width, LogoBig::height, LogoBig::width*3, 3); 		
#endif
	screen->enableDisplay(false);
}

void Tracker::hideSplash()
{
	screen->clear();
#if defined(__EXCLUDE_BIGLOGO__) || defined(__LOWRES__)
	screen->paintSplash(LogoSmall::rawData, LogoSmall::width, LogoSmall::height, LogoSmall::width*4, 4); 		
#else
	screen->paintSplash(LogoBig::rawData, LogoBig::width, LogoBig::height, LogoBig::width*3, 3); 		
#endif
	screen->enableDisplay(true);
	float shade = 256.0f;
	pp_int32 deltaT = 100;
	while (shade >= 0.0f)
	{
		pp_int32 startTime = ::PPGetTickCount();
#if defined(__EXCLUDE_BIGLOGO__) || defined(__LOWRES__)
		screen->paintSplash(LogoSmall::rawData, LogoSmall::width, LogoSmall::height, LogoSmall::width*4, 4, (int)shade); 		
#else
		screen->paintSplash(LogoBig::rawData, LogoBig::width, LogoBig::height, LogoBig::width*3, 3, (int)shade); 		
#endif
		shade-=deltaT * (1.0f/6.25f);
		deltaT = abs((pp_int32)::PPGetTickCount() - startTime);
		if (!deltaT) deltaT++;
	}
	screen->clear(); 	

	screen->pauseUpdate(true);
	screen->paintControl(getPatternEditorControl(), false);
	screen->paint();
	screen->pauseUpdate(false);
}

void Tracker::initDatabase() {
	loadConfig(settingsDatabase);

	// update version information
	settingsDatabase->store("VERSION", MILKYTRACKER_VERSION);

	// I think this line was only for applying settings from a copy?
	// applySettings(settingsDatabase, NULL, true, false);
}

void Tracker::startUp(bool forceNoSplash/* = false*/) {
	bool noSplash = forceNoSplash ? true : !getShowSplashFlagFromDatabase();

	// put up splash screen if desired
	pp_uint32 startTime = PPGetTickCount();
 
	if (!noSplash) 
		showSplash();
	else
		screen->enableDisplay(false);	

	initUI();	

	pp_int32 dTime;

	if (!noSplash) {
		dTime = (signed)(PPGetTickCount() - startTime);
		if (dTime > SPLASH_WAIT_TIME) dTime = SPLASH_WAIT_TIME;
		if (dTime < 0) dTime = 0;	
		System::msleep(SPLASH_WAIT_TIME/2 - dTime);
		startTime = PPGetTickCount();
	}
	
	// check for toml file, load it into the database

	// create as copy from existing database, so all keys are in there
	// is this necessary?

	//settingsDatabase = loadConfig();  


	/*if (XMFile::exists(System::getConfigFileName())) {
		// create as copy from existing database, so all keys are in there
		settingsDatabaseCopy = new TrackerSettingsDatabase(*settingsDatabase);
		
		// TODO: Check if the new style file exists and load that

		// and then check for the old style one if the

		XMFile f(System::getConfigFileName());
	
		// restore keys from disk
		settingsDatabaseCopy->serialize(f);	
		
		// everything alright, delete old database and take new one
		delete settingsDatabase;
		settingsDatabase = settingsDatabaseCopy;
		settingsDatabaseCopy = NULL;
	}*/

	// apply ALL settings, not just the different ones
	//


	
	// Update info panels
	updateSongInfo(false);
	
	updateWindowTitle();

	// try to start playback engine
	bool masterStart = playerMaster->start();	
	
	// remove splash screen
	//if (!noSplash) {
	//	dTime = (signed)(PPGetTickCount() - startTime);
	//	if (dTime > SPLASH_WAIT_TIME/2) dTime = SPLASH_WAIT_TIME/2;
	//	if (dTime < 0) dTime = 0;
	//
	//	System::msleep(SPLASH_WAIT_TIME/2 - dTime);
	//	hideSplash();
	//}
	//else
	screen->enableDisplay(true);			
	
	screen->paint();
	
	if (!masterStart)
	{
		SystemMessage systemMessage(*screen, SystemMessage::MessageSoundDriverInitFailed);
		systemMessage.show();
	}
}
