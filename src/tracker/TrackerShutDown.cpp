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

	saveConfig(settingsDatabase);

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

