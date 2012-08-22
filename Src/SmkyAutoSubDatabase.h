/* @@@LICENSE
*
*      Copyright (c) 2010-2012 Hewlett-Packard Development Company, L.P.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */

#ifndef _AUTO_SUB_DATABASE_H
#define _AUTO_SUB_DATABASE_H

#include <glib.h>
#include <stdint.h>
#include <map>
#include "SmkyDatabase.h"


namespace SmartKey
{

class Settings;

/**
 * Wraps the auto substitute database.
 */
class SmkyAutoSubDatabase : public AutoSubDatabase, public SmkyDatabase
{
public:
	SmkyAutoSubDatabase(SMKY_LINFO& lingInfo, const LocaleSettings& localeSettings, const Settings& settings);
	virtual ~SmkyAutoSubDatabase();

	SMKY_STATUS	init();

	virtual SmartKeyErrorCode learnWord(const std::string& word) { return SKERR_FAILURE; }
	virtual SmartKeyErrorCode forgetWord(const std::string& word);
	virtual SmartKeyErrorCode addEntry(const Entry& entry);
	virtual SmartKeyErrorCode getEntries(int offset, int limit, WhichEntries which, std::list<Entry>& entries);
	virtual SmartKeyErrorCode getNumEntries(WhichEntries which, int& entries);
	virtual SmartKeyErrorCode save();
	virtual SmartKeyErrorCode setLocaleSettings(const LocaleSettings& localeSettings);
	SMKY_STATUS findEntry(const gunichar2* shortcut, uint16_t shortcutLen, std::string& substitution);
	virtual std::string findEntry(const std::string& shortcut);
	bool didCreateDatabase() const;
	std::string getLdbSubstitution(const std::string& shortcut) const;
	std::string getHardCodedSubstitution(const std::string& shortcut) const;

private:

	static void matchGuessCaseToInputWord(const uint16_t* inputWord, uint16_t inputLen, uint16_t* guess, uint16_t guessLen);
	static bool isWordAllUppercase(const uint16_t* word, uint16_t wordLen);
	SMKY_STATUS duplicateShortEntries();
	SMKY_STATUS loadDefaultData();
	SMKY_STATUS loadHardCodedEntries();
	SMKY_STATUS loadEntries(WhichEntries which, std::list<Entry>& entries) const;
	SMKY_STATUS cacheLdbEntries();

	SMKY_STATUS addEntry(const std::string& shortcut, const std::string& substitution);
	std::string getDbPath() const;
	SMKY_STATUS	saveDb();

	const Settings& m_settings;
	uint8_t*      m_pDatabaseData;
	bool        m_createdDatabase;
	LocaleSettings m_locale;
	std::map<std::string,std::string>	m_ldbEntries;
	std::map<std::string,std::string>	m_hardCodedEntries;
};

}

#endif

