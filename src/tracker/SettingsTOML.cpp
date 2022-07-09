#include <iostream>
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

// settingsDatabaseCopy = new TrackerSettingsDatabase(*settingsDatabase);
// delete settingsDatabase;
// settingsDatabase = settingsDatabaseCopy;
// settingsDatabaseCopy = NULL;
// return 

// TODO: Return false if it doesn't find the expected file
void Tracker::loadConfig(TrackerSettingsDatabase* settingsDatabase) {
	try {
		const std::string fname("tracker_test.toml");
		const toml::value data = toml::parse(fname);

		toml::value resolution = toml::find(data, "settings", "resolution");
		settingsDatabase->store("XRESOLUTION", toml::find<int> (resolution, "width"));
		settingsDatabase->store("YRESOLUTION", toml::find<int> (resolution, "height"));

        settingsDatabase->store("FULLSCREEN", toml::find<bool> (data, "settings", "FULLSCREEN"));
		settingsDatabase->store("SCREENSCALEFACTOR", toml::find<int> (data, "settings", "SCREENSCALEFACTOR"));
        settingsDatabase->store("SHOWSPLASH", toml::find<bool> (data, "settings", "SHOWSPLASH"));

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
	catch(const std::exception& e) {
		std::cerr << e.what() << '\n';
        //return false;
	}
}

void Tracker::saveConfig(TrackerSettingsDatabase* settingsDatabase) {
    pp_int32 i;

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
            {"FULLSCREEN", settingsDatabase->restore("FULLSCREEN")->getBoolValue()},
            {"SCREENSCALEFACTOR", settingsDatabase->restore("SCREENSCALEFACTOR")->getIntValue()},
            {"SHOWSPLASH", settingsDatabase->restore("SHOWSPLASH")->getBoolValue()},
			{"resolution", {
                {"width", settingsDatabase->restore("XRESOLUTION")->getIntValue()},
                {"height", settingsDatabase->restore("YRESOLUTION")->getIntValue()}
            }},
			{"playmode", {
				{"KEEPSETTINGS", sectionQuickOptions->keepSettings() ? playModeStrings[playMode] : playModeStrings[4]},
				{"ADVANCED_ALLOW8xx", playerController->isPlayModeOptionEnabled(PlayerController::PlayModeOptionPanning8xx)},
				{"ADVANCED_ALLOWE8x", playerController->isPlayModeOptionEnabled(PlayerController::PlayModeOptionPanningE8x)},
				{"ADVANCED_PTPITCHLIMIT", playerController->isPlayModeOptionEnabled(PlayerController::PlayModeOptionForcePTPitchLimit)},
				{"ADVANCED_PTPANNING", panning},
			}},
			{"quick_options", {
					{"PROSPECTIVE", getProspectiveMode() }, 
					{"WRAPAROUND",  getCursorWrapAround() },
					{"FOLLOWSONG",  getFollowSong() },
					{"LIVESWITCH",  playerLogic->getLiveSwitch() }
			}},
			{"disk_operations", {
				{"INTERNALDISKBROWSERSETTINGS", sectionDiskMenu->getConfigUInt32()},
				{"INTERNALDISKBROWSERLASTPATH", (std::string)sectionDiskMenu->getCurrentPathASCII()}
			}},
			{"HD_recorder", {
				// HD_RECORDER_
				{"MIXFREQ", sectionHDRecorder->getSettingsFrequency()},
				{"MIXERVOLUME", sectionHDRecorder->getSettingsMixerVolume()},
				{"MIXERSHIFT", sectionHDRecorder->getSettingsMixerShift()},
				{"INTERPOLATION", sectionHDRecorder->getSettingsResampler()},
				{"RAMPING", sectionHDRecorder->getSettingsRamping()}, // bool
				{"ALLOWMUTING", sectionHDRecorder->getSettingsAllowMuting()} // bool
			}},
			{"sample_editor", {
				{"DECIMALOFFSETS", sectionSamples->getOffsetFormat()},
				// {"LASTVALUES", lastValues}
			}},
			{"optimizer", sectionoptimize},
			{"ENVELOPEEDITORSCALE", sectionInstruments->getEnvelopeEditorControl()->getScale()},
			{"EXTENDEDORDERLIST", extendedOrderlist}, // bool 
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
}