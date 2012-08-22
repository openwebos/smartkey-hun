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

#ifndef SMKY_MAN_DATABASE_H
#define SMKY_MAN_DATABASE_H

#include "SmkyMockImplementation.h"
#include "SmkyDatabase.h"
#include <string>
#include <map>
#include "StringUtils.h"
#include <hunspell/hunspell.hxx>

namespace SmartKey
{

class Settings;

/**
 * Wraps an  Manufacturer database.
 */
class SmkyManufacturerDatabase : public SmkyDatabase
{
public:

	SmkyManufacturerDatabase(SMKY_LINFO& lingInfo, const Settings& settings);

	virtual ~SmkyManufacturerDatabase();

	virtual SmartKeyErrorCode learnWord(const std::string& word);
	virtual SmartKeyErrorCode forgetWord(const std::string& word);
	virtual SmartKeyErrorCode save();
	virtual SmartKeyErrorCode setLocaleSettings(const LocaleSettings& localeSettings);
	virtual SmartKeyErrorCode setExpectedCount(int count);

	void logStatistics() const;

	void setQuery(const std::string & query);			// make query to hunspell to be return only relevant terms
	void setFistAndLastLetters(const std::string & firstLetters, const std::string & lastLetters, int minLength);
	bool getExactMatch(std::string & outExactMatch);	// from the last query, find the exact match if any...

private:

	SMKY_STATUS loadDefaultData(const LocaleSettings& localeSettings);
	SMKY_STATUS loadWords(const char * path, std::list<std::string> & wordList);

	SMKY_STATUS dbCallback( SMKY_REQ_MODE eMdbRequestType,
		uint16_t    wWordLen,
		uint16_t    wMaxWordLen,
		uint16_t  *psBuildTxtBuf,
		uint16_t   *pwActWordLen,
		uint32_t   *pdwWordListIdx);

	static SMKY_STATUS ManufacturerDbCallback(
		SMKY_LINFO *pLingInfo,
		SMKY_REQ_MODE eMdbRequestType,
		uint16_t    wWordLen,
		uint16_t    wMaxWordLen,
		uint16_t  *psBuildTxtBuf,
		uint16_t   *pwActWordLen,
		uint32_t   *pdwWordListIdx);

	void	createHunspellDictionary();
	void	deleteHunspellDictionary();

	std::map<std::string, int>	m_words;			///< The word mapped to the number of times it's used.
	std::list<std::string>      m_stockManDbWords;	///< The words we load from our text files that are hard-coded entries.
	std::list<std::string>		m_universalWords;	///< The words we load four a text file that are hard-coded entries for ALL locales.
	const Settings& m_settings;
	Hunspell *					m_hunspell;
	int							m_hunspellSize;
	int							m_expectedContactCount;	///< How many countacts do we expect to have, to help hunspell create a properly sized hashmap

	char **						m_lastHunspellResult;
	int							m_lastHunspellResultCount;
	std::string					m_lastHunspellQuery;
	std::vector<const char *>	m_lastFirstLastLetterResults;
};

}

#endif

