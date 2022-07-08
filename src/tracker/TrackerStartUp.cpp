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



/*PPSize Tracker::getWindowSizeFromDatabase() {
	PPSize size(PPScreen::getDefaultWidth(), PPScreen::getDefaultHeight());
	if (XMFile::exists(System::getConfigFileName())) {

	}

	return size;
}*/


PPSize Tracker::getWindowSizeFromDatabase()
{
	PPSize size(PPScreen::getDefaultWidth(), PPScreen::getDefaultHeight());
	
	if (XMFile::exists(System::getConfigFileName()))
	{
		TrackerSettingsDatabase* settingsDatabaseCopy = new TrackerSettingsDatabase(*settingsDatabase);		
		XMFile f(System::getConfigFileName());	
		settingsDatabaseCopy->serialize(f);			
		size.height = settingsDatabaseCopy->restore("YRESOLUTION")->getIntValue();
		size.width = settingsDatabaseCopy->restore("XRESOLUTION")->getIntValue();
		delete settingsDatabaseCopy;
	}

	return size;
}

bool Tracker::getFullScreenFlagFromDatabase()
{
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
}

pp_int32 Tracker::getScreenScaleFactorFromDatabase()
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
}

bool Tracker::getShowSplashFlagFromDatabase()
{
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
	settingsDatabaseCopy = new TrackerSettingsDatabase(*settingsDatabase);

	try
	{
		const std::string fname("tracker_test.toml");
		const toml::value data = toml::parse(fname);

		toml::value playmode = toml::find(data, "settings", "playmode");
		settingsDatabase->store("PLAYMODEKEEPSETTINGS", (PPString)toml::find<std::string>(playmode, "KEEPSETTINGS").c_str());
		settingsDatabase->store("PLAYMODE_ADVANCED_ALLOW8xx", toml::find<bool>(playmode, "ADVANCED_ALLOW8xx"));
		settingsDatabase->store("PLAYMODE_ADVANCED_ALLOWE8x", toml::find<bool>(playmode, "ADVANCED_ALLOWE8x"));
		settingsDatabase->store("PLAYMODE_ADVANCED_PTPITCHLIMIT", toml::find<bool>(playmode, "ADVANCED_PTPITCHLIMIT"));
		//strd::array ADVANCED_PTPANNING = 
		//settingsDatabase->store("PLAYMODE_ADVANCED_PTPANNING", 
		//toml::find<std::vector<int>>(playmode, "ADVANCED_PTPANNING"));

		toml::value quick_options = toml::find(data, "settings", "quick_options");
		settingsDatabase->store("FOLLOWSONG",  toml::find<bool>(quick_options, "FOLLOWSONG"));
		settingsDatabase->store("WRAPAROUND",  toml::find<bool>(quick_options, "WRAPAROUND"));
		settingsDatabase->store("LIVESWITCH",  toml::find<bool>(quick_options, "LIVESWITCH"));
		settingsDatabase->store("PROSPECTIVE", toml::find<bool>(quick_options, "PROSPECTIVE"));

		toml::value disk_operations = toml::find(data, "settings", "disk_operations");
		settingsDatabase->store("INTERNALDISKBROWSERSETTINGS",toml::find<int>(disk_operations, "INTERNALDISKBROWSERSETTINGS"));
		settingsDatabase->store("INTERNALDISKBROWSERLASTPATH",toml::find<std::string>(disk_operations, "INTERNALDISKBROWSERLASTPATH").c_str());

		toml::value HD_recorder = toml::find(data, "settings", "HD_recorder");
		settingsDatabase->store("MIXFREQ",       toml::find<int> (HD_recorder, "MIXFREQ"));
		settingsDatabase->store("MIXERVOLUME",   toml::find<int> (HD_recorder, "MIXERVOLUME"));
		settingsDatabase->store("MIXERSHIFT",    toml::find<int> (HD_recorder, "MIXERSHIFT"));
		settingsDatabase->store("INTERPOLATION", toml::find<int> (HD_recorder, "INTERPOLATION"));
		settingsDatabase->store("RAMPING",       toml::find<bool>(HD_recorder, "RAMPING"));
		settingsDatabase->store("ALLOWMUTING",   toml::find<bool>(HD_recorder, "ALLOWMUTING"));

		toml::value sample_editor = toml::find(data, "settings", "sample_editor");
		settingsDatabase->store("SAMPLEEDITORDECIMALOFFSETS", toml::find<int> (sample_editor, "DECIMALOFFSETS"));
		// settingsDatabase->store("SAMPLEEDITORLASTVALUES", toml::find<int> (sample_editor, "LASTVALUES"));	

		//settingsDatabase->store("optimizer
		settingsDatabase->store("EXTENDEDORDERLIST",     toml::find<bool>(data, "settings", "EXTENDEDORDERLIST"));
		settingsDatabase->store("TITLEPAGE",		     toml::find<bool>(data, "settings", "TITLEPAGE"));
		settingsDatabase->store("ACTIVECOLORS", 	     (PPString)toml::find<std::string> (data, "settings", "ACTIVECOLORS").c_str());
		settingsDatabase->store("ENVELOPEEDITORSCALE",   toml::find<int> (data, "settings", "ENVELOPEEDITORSCALE"));
		settingsDatabase->store("ROWINSERTADD", 	     toml::find<int> (data, "settings", "ROWINSERTADD"));
		// settingsDatabase->store("PREDEFENVELOPEVOLUME",  toml::find<int> (data, "settings", "PREDEFENVELOPEVOLUME"));
		// settingsDatabase->store("PREDEFENVELOPEPANNING", toml::find<int> (data, "settings", "PREDEFENVELOPEPANNING"));
		// settingsDatabase->store("EFFECTMACRO", 			 toml::find<int> (data, "settings", "EFFECTMACRO"));
		// settingsDatabase->store("PREDEFCOLORPALETTE", 	 toml::find<int> (data, "settings", "PREDEFCOLORPALETTE"));



		// std::cout << "data:" << data << "\n";
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
	delete settingsDatabase;
	settingsDatabase = settingsDatabaseCopy;
	settingsDatabaseCopy = NULL;


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
	//applySettings(settingsDatabase, NULL, true, false);

	// update version information
	settingsDatabase->store("VERSION", MILKYTRACKER_VERSION);
	
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
