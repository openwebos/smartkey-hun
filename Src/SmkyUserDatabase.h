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

#ifndef SMK_USER_DATABASE_H
#define SMK_USER_DATABASE_H

#include <stdint.h>
#include "SmkyMockImplementation.h"
#include "SmkyDatabase.h"

namespace SmartKey
{

class Settings;

/**
 * Wraps the reorder user database (RUDB).
 */
class SmkyUserDatabase : public UserDatabase, public SmkyDatabase
{
public:
	SmkyUserDatabase(SMKY_LINFO& lingInfo, const Settings& settings);
	virtual ~SmkyUserDatabase();

	SMKY_STATUS	init();

	virtual SmartKeyErrorCode learnWord(const std::string& word);
	virtual SmartKeyErrorCode forgetWord(const std::string& word);
	virtual SmartKeyErrorCode getEntries(int offset, int limit, std::list<std::string>& entries);
	virtual SmartKeyErrorCode getNumEntries(int& entries);
            SmartKeyErrorCode findWord(const std::string& word) const;
	virtual SmartKeyErrorCode save();
	virtual SmartKeyErrorCode setLocaleSettings(const LocaleSettings& localeSettings);
    virtual SmartKeyErrorCode updateWordUsage(const std::string& word);

	static SmartKeyErrorCode smkyErrorToSmartKeyError(SMKY_STATUS err);

private:

	std::string getDbPath() const;
	SmartKeyErrorCode loadEntries(std::list<std::string>& entries) const;
	SMKY_STATUS	saveDb();

	const Settings& m_settings;
	uint8_t *     m_pDatabaseData;
};

}

#endif

