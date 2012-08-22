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

#ifndef SMKY_SPELL_CHECK_ENGINE_H
#define SMKY_SPELL_CHECK_ENGINE_H

#include <string>
#include <set>
#include "SmkyDatabase.h"
#include "StringUtils.h"
#ifdef ET9_ALPHABETIC_MODULE
 #include <webosDeviceKeymap.h>
#endif
#include "SmkyMockImplementation.h"
#include "SpellCheckEngine.h"
#include "SpellCheckClient.h"

namespace SmartKey {

class SmkyUserDatabase;
class SmkyAutoSubDatabase;
class SmkyManufacturerDatabase;
class Settings;
struct SpellCheckWordInfo;
class TapDataArray;

const size_t SEL_LIST_SIZE = 32;

/**
 * Our wrapper around the  spell check engine.
 */
class SmkySpellCheckEngine : public SpellCheckEngine
{
public:

	SmkySpellCheckEngine(const Settings& settings);
	virtual ~SmkySpellCheckEngine();

	virtual SmartKeyErrorCode checkSpelling(const std::string& word, SpellCheckWordInfo& result, int maxGuesses);
	virtual SmartKeyErrorCode autoCorrect(const std::string& word, const std::string& context, SpellCheckWordInfo& result, int maxGuesses);
	virtual SmartKeyErrorCode processTrace(const std::vector<unsigned int>& points, EShiftState shift, const std::string& firstChars, const std::string& lastChars, SpellCheckWordInfo& result, int maxGuesses);
	virtual SmartKeyErrorCode processTaps(const TapDataArray& taps, SpellCheckWordInfo& result, int maxGuesses);
	virtual SmartKeyErrorCode getCompletion(const std::string& prefix, std::string& result);

	virtual	UserDatabase* getUserDatabase();
	virtual AutoSubDatabase* getAutoSubDatabase();
	virtual Database* getManufacturerDatabase();
	virtual bool setLocaleSettings(const LocaleSettings& localeSettings, bool isVirtualKeyboard);
	virtual const char * getSupportedLanguages() const;

private:

	struct LanguageInfo {
		LanguageInfo(const std::string& lang, uint16_t wLangId, SMKY_LINFO& lingInfo) :
			  m_lang(lang)
			, m_langId(wLangId)
			, m_initAttempted(false)
			, m_langLoadStatus(SMKY_STATUS_NONE)
		    , m_lingInfo(lingInfo)
		{
		}

		std::string m_lang;
		uint16_t    m_langId;
		bool      m_initAttempted;
		SMKY_STATUS m_langLoadStatus;
		SMKY_LINFO& m_lingInfo;

		SMKY_STATUS initLanguage();
	};

	enum SMKYsetup
	{
		smkysetup_none,
		smkysetup_checkSpellingOrAutoCorrect,
		smkysetup_processTaps,
		smkysetup_processTrace,
		smkysetup_completion
	};

	SMKY_STATUS	init();
	LanguageInfo* getLanguageInfo(const std::string& languageCode);
	LanguageInfo* getLanguageInfo(uint16_t wLangId);
    SmartKeyErrorCode loadWhitelist(const LocaleSettings& localeSettings);
    SmartKeyErrorCode loadLocaleWords(const LocaleSettings& localeSettings);
	SMKY_STATUS typeWord(const uint16_t * word, uint16_t count);

    SMKY_STATUS getSelectionResults(SpellCheckWordInfo& result, int maxGuesses);

	static bool wordIsAllDigits(const std::string& word);
	static SmartKeyErrorCode loadWords(const std::string& fname, std::set<std::string>& words);

	std::list<LanguageInfo> m_langInfo;
	SmkyUserDatabase*    m_userDb;
	SmkyManufacturerDatabase* m_manDb;
	SmkyAutoSubDatabase* m_autoSubDb;
	SMKY_STATUS		    m_wSmkyInitStatus;
	const Settings&     m_settings;
	uint16_t              m_primaryLanguage;
    uint16_t              m_pendingKdb;
    SMKYsetup            m_smkysetup;
    bool                m_keyboardIsVirtual;
	std::set<std::string> m_whitelist;	//< The list of whitelisted words to not spell check
	std::set<std::string> m_localeWords; //< Words from locales other than the current
};

}

#endif
