/*
 *  tracker/TrackerShutDown.cpp
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
 *  TrackerShutdown.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sun Mar 20 2005.
 *
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <utility> 

#include <toml.hpp>

#include "Tracker.h"
#include "TabManager.h"
#include "MilkyPlay.h"
#include "PlayerController.h"
#include "PlayerMaster.h"
#include "PlayerLogic.h"
#include "TrackerSettingsDatabase.h"
#include "TrackerConfig.h"
#include "PPSystem.h"
#include "PPSavePanel.h"
#include "PPQuitSaveAlert.h"
#include "ModuleEditor.h"
#include "SectionInstruments.h"
#include "PatternEditorControl.h"
#include "EnvelopeEditorControl.h"
#include "SampleEditorControl.h"
#include "SectionDiskMenu.h"
#include "SectionHDRecorder.h"
#include "SectionOptimize.h"
#include "ScopesControl.h"
#include "GlobalColorConfig.h"
#include "ColorPaletteContainer.h"
#include "SectionSettings.h"
#include "SectionSamples.h"
#include "SectionQuickOptions.h"
#include "Tools.h"
#include "TitlePageManager.h"
#include "version.h"

bool Tracker::checkForChanges(ModuleEditor* moduleEditor/* = NULL*/)
{
	if (moduleEditor == NULL)
		moduleEditor = this->moduleEditor;

	// save current file?
	if (moduleEditor->hasChanged())
	{
		
		PPQuitSaveAlert quitSaveAlertDialog(screen);
		PPQuitSaveAlert::ReturnCodes err = quitSaveAlertDialog.runModal();
		
		if (err == PPQuitSaveAlert::ReturnCodeOK)
		{
			
			PPSavePanel savePanel(screen, "Save Extended Module", moduleEditor->getModuleFileName());
			savePanel.addExtension("xm","Fasttracker 2 Module");
			err = savePanel.runModal();
			if (err == PPSavePanel::ReturnCodeOK)
			{
				const SYSCHAR* file = savePanel.getFileName();
				
				if (file)
				{
					moduleEditor->saveSong(file);
				}
			}
			else if (err == PPSavePanel::ReturnCodeCANCEL)
			{
				return false;
			}
			
		}
		else if (err == PPSavePanel::ReturnCodeCANCEL)
		{
			return false;
		}
		
	}
	
	return true;
}

bool Tracker::checkForChangesOpenModule()
{
	bool openTab = (settingsDatabase->restore("TABS_LOADMODULEINNEWTAB")->getBoolValue() &&
					(moduleEditor->hasChanged() || !moduleEditor->isEmpty()));
	
	if (openTab)
		return true;
	
	return checkForChanges();
}

bool Tracker::shutDown() {
	pp_int32 i;
	ModuleEditor* currentEditor = moduleEditor;
	for (i = 0; i < tabManager->getNumTabs(); i++) {
		moduleEditor = tabManager->getModuleEditorFromTabIndex(i);
		bool res = checkForChanges();

		if (!res)
			return false;
	}
	moduleEditor = currentEditor;

	playerMaster->stop(true);

	std::string playModeStrings[5] = {"AUTO", "PROTRACKER2", "PROTRACKER3", "SCREAMTRACKER3", "FASTTRACKER2"};
	int playMode = playerController->getPlayMode();
	ASSERT(playMode >= 0 && playMode < 5);

	std::vector<int> panning;
	for (i = 0; i < TrackerConfig::numPlayerChannels; i++)
		panning.push_back(playerController->getPanning((int)i));
	// might be able to just grab a slice of the whole thing?
	// std::cout << playerController->getPanningTable();

	//PPDictionary dictionary = sectionSamples->getSampleEditorControl()->getLastValues().convertToDictionary();
	//std::string lastValues = (std::string)dictionary.serializeToString();
	const toml::value lastValues = sectionSamples->getSampleEditorControl()->getLastValues().convertToTOML();

	std::vector<int> sectionoptimize;
	for (i = 0; i < (signed)SectionOptimize::getNumFlagGroups(); i++) {
		sectionoptimize.push_back(sectionOptimize->getOptimizeCheckBoxFlags(i));
	}

	TitlePageManager titlePageManager(*screen);


	TColorPalette palette;
	palette.numColors = GlobalColorConfig::ColorLast;
	for (i = 0; i < palette.numColors; i++)
		palette.colors[i] = GlobalColorConfig::getInstance()->getColor((GlobalColorConfig::GlobalColors)i);	

	std::string paletteString = (std::string)ColorPaletteContainer::encodePalette(palette);

	// TODO: Maybe replace vectors with a specified size array?
	std::vector<std::string> predefenvelopevolume;
	for (i = 0; i < sectionInstruments->getNumPredefinedEnvelopes(); i++) {
		predefenvelopevolume.push_back(
			(std::string)sectionInstruments->getEncodedEnvelope(SectionInstruments::EnvelopeTypeVolume, i));
	}
	std::vector<std::string> predefenvelopepanning;
	for (i = 0; i < sectionInstruments->getNumPredefinedEnvelopes(); i++) {
		predefenvelopepanning.push_back(
			(std::string)sectionInstruments->getEncodedEnvelope(SectionInstruments::EnvelopeTypePanning, i));
	}

	//
	std::vector<int> effectmacro;
	for (i = 0; i < NUMEFFECTMACROS; i++) {
		pp_uint8 eff, op;
		getPatternEditor()->getMacroOperands(i, eff, op);
		pp_int32 val = (((pp_int32)eff) << 8) + (pp_int32)op;
		effectmacro.push_back((int)val);
	}
	std::vector<std::string> predefcolorpalette;
	for (i = 0; i < sectionSettings->getNumPredefinedColorPalettes(); i++) {
		predefcolorpalette.push_back((std::string)sectionSettings->getEncodedPalette(i));
	}

	const toml::value data{
		{"metadata", {
			{"app_version", MILKYTRACKER_VERSION},
			{"config_version", 0}
		}},
		{"settings", {
			{"playmode", {
				{"PLAYMODEKEEPSETTINGS", sectionQuickOptions->keepSettings() ? playModeStrings[playMode] : playModeStrings[4]},
				{"PLAYMODE_ADVANCED_ALLOW8xx", playerController->isPlayModeOptionEnabled(PlayerController::PlayModeOptionPanning8xx)},
				{"PLAYMODE_ADVANCED_ALLOWE8x", playerController->isPlayModeOptionEnabled(PlayerController::PlayModeOptionPanningE8x)},
				{"PLAYMODE_ADVANCED_PTPITCHLIMIT", playerController->isPlayModeOptionEnabled(PlayerController::PlayModeOptionForcePTPitchLimit)},
				{"PLAYMODE_ADVANCED_PTPANNING", panning},
			}},
			{"quick_options", {
					{"PROSPECTIVE", getProspectiveMode() ? 1 : 0},
					{"WRAPAROUND", getCursorWrapAround() ? 1 : 0},
					{"FOLLOWSONG", getFollowSong() ? 1 : 0},
					{"LIVESWITCH", playerLogic->getLiveSwitch() ? 1 : 0}	
			}},
			{"disk_operations", {
				{"INTERNALDISKBROWSERSETTINGS", sectionDiskMenu->getConfigUInt32()},
				{"INTERNALDISKBROWSERLASTPATH", (std::string)sectionDiskMenu->getCurrentPathASCII()}
			}},
			{"HD_recorder", {
				{"HDRECORDER_MIXFREQ", sectionHDRecorder->getSettingsFrequency()},
				{"HDRECORDER_MIXERVOLUME", sectionHDRecorder->getSettingsMixerVolume()},
				{"HDRECORDER_MIXERSHIFT", sectionHDRecorder->getSettingsMixerShift()},
				{"HDRECORDER_RAMPING", sectionHDRecorder->getSettingsRamping() ? 1 : 0},
				{"HDRECORDER_INTERPOLATION", sectionHDRecorder->getSettingsResampler()},
				{"HDRECORDER_ALLOWMUTING", sectionHDRecorder->getSettingsAllowMuting() ? 1 : 0}
			}},
			{"sample_editor", {
				{"SAMPLEEDITORDECIMALOFFSETS", sectionSamples->getOffsetFormat()},
				{"SAMPLEEDITORLASTVALUES", lastValues}
			}},
			{"optimizer", sectionoptimize},
			{"ENVELOPEEDITORSCALE", sectionInstruments->getEnvelopeEditorControl()->getScale()},
			{"EXTENDEDORDERLIST", extendedOrderlist ? 1 : 0},
			{"ROWINSERTADD", getPatternEditorControl()->getRowInsertAdd()},
			{"TITLEPAGE", titlePageManager.getCurrentTitlePage()},
			{"ACTIVECOLORS", paletteString},
			{"PREDEFENVELOPEVOLUME", predefenvelopevolume},
			{"PREDEFENVELOPEPANNING", predefenvelopepanning},
			{"EFFECTMACRO", effectmacro},
			{"PREDEFCOLORPALETTE", predefcolorpalette}
		}}
	};

	//std::cout << "filename is... " << filename << "\n";
 	std::ofstream tomlfile;
	//auto newfilename = filename+".toml"
    tomlfile.open ("tracker_test.toml");
    tomlfile << std::setw(80) << data << std::endl;
    tomlfile.close();
	
	return true;
}

// this is used when the tracker crashes
// The os dependent layer can call this and it will save a backup of the current
// module
// TO-DO: Doesn't handle tabs properly yet, will only save the current tab
mp_sint32 Tracker::saveModule(const PPSystemString& fileName)
{
	return moduleEditor->saveBackup(fileName);
}

