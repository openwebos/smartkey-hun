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

#include <glib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <wctype.h>
#include <math.h>
#include <algorithm>
#include <errno.h>
#include "StringUtils.h"
#include "SmkySpellCheckEngine.h"
#include "SmkyDatabase.h"
#include "SmkyUserDatabase.h"
#include "SmkyAutoSubDatabase.h"
#include "SmkyManufacturerDatabase.h"
#include "Settings.h"
#include "SmartKeyService.h"
#include "SpellCheckClient.h"

const uint16_t INVALID_KDB = -1;

// We only ever use the first page of our keyboard db's (at least for now).
const uint16_t k_wLangPageNo = 0;

static const Settings* g_pSettings;

// debug macros for calls. Sets the wStatus variable that must be defined already. Declare a local status variable of type SMKY_STATUS.
#define SMKY_VERIFY(x) (G_LIKELY((wStatus = x) == SMKY_STATUS_NONE) || (g_warning("'%s' returned error #%d, in %s of %s line %d", #x, wStatus, __FUNCTION__, __FILE__, __LINE__), false))
#define SMKY_VERIFY2(x, OKerror) (G_LIKELY((wStatus = x) == SMKY_STATUS_NONE) || (wStatus == OKerror) || (g_warning("'%s' returned error #%d, in %s of %s line %d", #x, wStatus, __FUNCTION__, __FILE__, __LINE__), false))


namespace SmartKey {

/* public */
/**
* SmkySpellCheckEngine()
* <here is function description>
*
* @param settings
*   <perameter description>
*/
SmkySpellCheckEngine::SmkySpellCheckEngine(const Settings& settings) :
	  m_userDb(NULL)
	, m_manDb(NULL)
	, m_autoSubDb(NULL)
	, m_settings(settings)
	, m_primaryLanguage(0)
	, m_pendingKdb(INVALID_KDB)
	, m_smkysetup(smkysetup_none)
	, m_keyboardIsVirtual(false)
{

	g_pSettings = &settings;
	
	m_wSmkyInitStatus = init();
	if (m_wSmkyInitStatus == SMKY_STATUS_NONE)
		g_message("engine successfully initialized.");
	else
		g_warning("ERROR %u initializing engine.", m_wSmkyInitStatus);
}

/* public */
/**
* ~SmkySpellCheckEngine()
* <here is function description>
*/
SmkySpellCheckEngine::~SmkySpellCheckEngine()
{
	delete m_autoSubDb;
	delete m_userDb;
	delete m_manDb;
}

/**
* init()
* <here is function description>
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkySpellCheckEngine::init()
{
	return SMKY_STATUS_NONE;
}


/* public */
/**
* getSupportedLanguages()
* <here is function description>
*
* @return char*
*   <return value description>
*/
const char *  SmkySpellCheckEngine::getSupportedLanguages() const
{
    return "{\"languages\":[\"en_un\",\"es_un\",\"fr_un\",\"de_un\",\"it_un\"]}";
}


/* private */
/**
* wordIsAllDigits()
* <here is function description>
*
* @param word
*   <perameter description>
*
* @return bool
*   <return value description>
*/
bool SmkySpellCheckEngine::wordIsAllDigits(const std::string& word)
{
	std::string::const_iterator i;
	for (i = word.begin(); i != word.end(); ++i) {
		if (!(isdigit(*i) || *i == '.'))
			return false;
	}

	return true;
}

/* public */
/**
* checkSpelling()
* <here is function description>
*
* @param word
*   <perameter description>
*
* @param result
*   <perameter description>
*
* @param maxGuesses
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkySpellCheckEngine::checkSpelling(const std::string& word, SpellCheckWordInfo& result, int maxGuesses)
{
	result.clear();
    return SKERR_SUCCESS;
}

/* public */
/**
* autoCorrect()
* <here is function description>
*
* @param word
*   <perameter description>
*
* @param context
*   <perameter description>
*
* @param result
*   <perameter description>
*
* @param maxGuesses
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkySpellCheckEngine::autoCorrect(const std::string& word, const std::string& context, SpellCheckWordInfo& result, int maxGuesses)
{
	result.clear();
	return SKERR_SUCCESS;
}

#ifdef TARGET_DESKTOP
#define DO_DEBUG_PRIMEWORD 0
#else
#define DO_DEBUG_PRIMEWORD 0
#endif

#if DO_DEBUG_PRIMEWORD
#define DEBUG_PRIMEWORD g_debug
#else
#define DEBUG_PRIMEWORD(fmt, args...) (void)0
#endif

/**
* typeWord()
* <here is function description>
*
* @param *word
*   <perameter description>
*
* @param wordLength
*   <perameter description>
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkySpellCheckEngine::typeWord (const uint16_t *word, uint16_t wordLength)
{
    SMKY_STATUS wStatus = SMKY_STATUS_NONE;
    return wStatus;
}

/* public */
/**
* processTrace()
* <here is function description>
*
* @param points
*   <perameter description>
*
* @param shift
*   <perameter description>
*
* @param firstChars
*   <perameter description>
*
* @param lastChars
*   <perameter description>
*
* @param result
*   <perameter description>
*
* @param maxGuesses
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkySpellCheckEngine::processTrace(const std::vector<unsigned int>& points, EShiftState shift, const std::string& firstChars, const std::string& lastChars, SpellCheckWordInfo& result, int maxGuesses)
{
	result.clear();
	return SKERR_SUCCESS;
}

/* public */
/**
* processTaps()
* <here is function description>
*
* @param taps
*   <perameter description>
*
* @param result
*   <perameter description>
*
* @param maxGuesses
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkySpellCheckEngine::processTaps (const TapDataArray& taps, SpellCheckWordInfo& result, int maxGuesses)
{
	result.clear();

	return SKERR_SUCCESS;
}

/**
* getSelectionResults()
* <here is function description>
*
* @param result
*   <perameter description>
*
* @param maxGuesses
*   <perameter description>
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkySpellCheckEngine::getSelectionResults (SpellCheckWordInfo& result, int maxGuesses)
{
	result.clear();
	SMKY_STATUS wStatus = SMKY_STATUS_NONE;
	return wStatus;
}

/* public */
/**
* getCompletion()
* <here is function description>
*
* @param prefix
*   <perameter description>
*
* @param result
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkySpellCheckEngine::getCompletion (const std::string& prefix, std::string& result)
{
	result.clear();

	if (m_wSmkyInitStatus != SMKY_STATUS_NONE)
		return SmkyUserDatabase::smkyErrorToSmartKeyError(m_wSmkyInitStatus);

	if (prefix.empty())
		return SmkyUserDatabase::smkyErrorToSmartKeyError(SMKY_STATUS_BAD_PARAM);

	return SmkyUserDatabase::smkyErrorToSmartKeyError(SMKY_STATUS_NONE);
}

/* public */
/**
* getUserDatabase()
* Return the user (read/write) database.
*
* @return UserDatabase*
*   <return value description>
*/
UserDatabase* SmkySpellCheckEngine::getUserDatabase()
{
	return m_userDb;
}

/* public */
/**
* getAutoSubDatabase()
* Return the auto-substitution (read/write) database.
*
* @return AutoSubDatabase*
*   <return value description>
*/
AutoSubDatabase* SmkySpellCheckEngine::getAutoSubDatabase()
{
	return m_autoSubDb;
}

/* public */
/**
* getManufacturerDatabase()
* <here is function description>
*
* @return Database*
*   <return value description>
*/
Database* SmkySpellCheckEngine::getManufacturerDatabase()
{
	return m_manDb;
}

/**
* getLanguageInfo()
* <here is function description>
*
* @param wLangId
*   <perameter description>
*
* @return LanguageInfo*
*   <return value description>
*/
SmkySpellCheckEngine::LanguageInfo* SmkySpellCheckEngine::getLanguageInfo (uint16_t wLangId)
{
	std::list<LanguageInfo>::iterator i;
	for (i = m_langInfo.begin(); i != m_langInfo.end(); ++i) {
		if (i->m_langId == wLangId) {
			return &*i;
		}
	}

	return NULL;
}

/**
* getLanguageInfo()
* <here is function description>
*
* @param languageCode
*   <perameter description>
*
* @return LanguageInfo*
*   <return value description>
*/
SmkySpellCheckEngine::LanguageInfo* SmkySpellCheckEngine::getLanguageInfo(const std::string& languageCode)
{
	if (languageCode.length() < 2) {
		return NULL;
	}

	char lang[3];
	lang[0] = tolower(languageCode[0]);
	lang[1] = tolower(languageCode[1]);
	lang[2] = '\0';

	// this currently is only called when the locale changes (which is rare) and
	// this way I can return a pointer in my container (which doesn't change) and
	// not worry that std::map might move the pointer.
	std::list<LanguageInfo>::iterator i;
	for (i = m_langInfo.begin(); i != m_langInfo.end(); ++i) {
		if (i->m_lang == lang) {
			return &*i;
		}
	}

	return NULL;
}

/**
* initLanguage()
* <here is function description>
*
* @return SMKY_STATUS
*   The return code.
*/
SMKY_STATUS SmkySpellCheckEngine::LanguageInfo::initLanguage()
{
	if (m_initAttempted)
		return m_langLoadStatus;

	m_initAttempted = true;

	return (m_langLoadStatus = SMKY_STATUS_NONE);
}

/**
* setLocaleSettings()
* Set the current primary locale.
*
* @param localeSettings
*   <perameter description>
*
* @param isVirtualKeyboard
*   <perameter description>
*
* @return bool
*   <return value description>
*/
bool SmkySpellCheckEngine::setLocaleSettings(const LocaleSettings& localeSettings, bool isVirtualKeyboard)
{
	g_debug("SmkySpellCheckEngine::setLocaleSettings: %s, %s", localeSettings.getFullLocale().c_str(), isVirtualKeyboard ? "virtual keyboard" : "physical keyboard");
	SMKY_STATUS wStatus(SMKY_STATUS_NONE);

	m_keyboardIsVirtual = isVirtualKeyboard;

	LanguageInfo* primaryInfo = getLanguageInfo(localeSettings.m_inputLanguage);

	uint16_t newPrimary = (primaryInfo ? primaryInfo->m_langId : 0);

	if (newPrimary != m_primaryLanguage) {

		if (primaryInfo)
			primaryInfo->initLanguage();

		m_primaryLanguage = newPrimary;
	}
        
	loadWhitelist(localeSettings);
	loadLocaleWords(localeSettings);
	m_smkysetup = smkysetup_none;

	return wStatus == SMKY_STATUS_NONE;
}

/**
* loadWords()
* <here is function description>
*
* @param fname
*   <perameter description>
*
* @param words
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkySpellCheckEngine::loadWords(const std::string& fname, std::set<std::string>& words)
{
	if (fname.empty())
		return SKERR_BAD_PARAM;

	SmartKeyErrorCode err = SKERR_FAILURE;
	FILE* file = fopen(fname.c_str(), "r");
	if (file) {
		char line[128];
		while (fgets(line, G_N_ELEMENTS(line), file) != NULL) {
			if (line[0] != '#') {
				g_strchomp(line);
				words.insert(line);
			}
		}
		fclose(file);
		err = SKERR_SUCCESS;
	}

	return err;
}

/**
* loadLocaleWords()
* Loads a list of words from all locales (other than the current) that have spellings that
* are specific to that locale. We use this list to prevent auto-correcting to those words.
* This is necessary because it has a Global English LDB that often contains words from
* multiple locales (but not with the correct frequency). More info at NOV-116715.
*
* @param localeSettings
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkySpellCheckEngine::loadLocaleWords(const LocaleSettings& localeSettings)
{
	const std::string	localeDir = m_settings.readOnlyDataDir + "/smky/DefaultData/locale";
	const char *		localeSuffix = "/locale-words";

	std::string language = localeSettings.m_inputLanguage;
	if (language.size() != 2)
		return SKERR_FAILURE;

	m_localeWords.clear();

	SmartKeyErrorCode err = SKERR_FAILURE;

	std::set<std::string> currentLocaleWords;
	std::string currentLocalePath = localeSettings.findLocalResource(localeDir + '/', localeSuffix);
	err = loadWords(currentLocalePath, currentLocaleWords);
	if (err == SKERR_SUCCESS) {
		g_debug("Loaded %u preferred words from %s", currentLocaleWords.size(), currentLocalePath.c_str());
		DIR* dir = opendir(localeDir.c_str());
		if (dir) {
			const struct dirent* entry;
			while ((entry=readdir(dir)) != NULL) {
				if (entry->d_type == DT_DIR &&
						strlen(entry->d_name) == 5 &&
						entry->d_name[0] == language[0] && entry->d_name[1] == language[1] &&
						currentLocalePath.compare(0, localeDir.length() + 6, localeDir + '/' + entry->d_name) != 0) {	// language matches

					std::string fname = localeDir + "/" + entry->d_name + localeSuffix;
					std::set<std::string> localeWords;
					err = loadWords(fname, localeWords);
					if (err == SKERR_SUCCESS) {
						g_debug("Loaded %u biased words from %s", localeWords.size(), fname.c_str());
						std::set<std::string>::const_iterator word;
						for (word = localeWords.begin(); word != localeWords.end(); ++word) {
							// If not in common with current locale's words
							// Then put in list of other locale words.
							if (currentLocaleWords.find(*word) == currentLocaleWords.end()) {
								m_localeWords.insert(*word);
							}
						}
					}
				}
			}

			closedir(dir);
		}
		else {
			g_warning("Can't open %s", localeDir.c_str());
		}
		g_debug("Loaded %u locale specific words to avoid for %s", m_localeWords.size(), localeSettings.getFullLocale().c_str());
	}
	else
		g_debug("No locale specific words to avoid for %s", localeSettings.getFullLocale().c_str());


	return err;
}

/**
* loadWhitelist()
* <here is function description>
*
* @param localeSettings
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkySpellCheckEngine::loadWhitelist (const LocaleSettings& localeSettings)
{
	m_whitelist.clear();

	// load missing language words for the current input language. Shared by all language variations, not encrypted.
	// For things that were missing in our English dictionary, like "textfield" which had an "interesting" correction...
	loadWords(m_settings.readOnlyDataDir + "/smky/DefaultData/whitelist/" + localeSettings.m_inputLanguage + "_whitelist-entries", m_whitelist);
	if (m_whitelist.size() > 0)
		g_debug("Loaded %u '%s' white list entries", m_whitelist.size(), localeSettings.m_inputLanguage.c_str());

	SmartKeyErrorCode err = SKERR_FAILURE;
	std::string fname = localeSettings.findLocalResource(m_settings.readOnlyDataDir + "/smky/DefaultData/whitelist/", "/whitelist-entries");
	FILE* f = fopen(fname.c_str(),"r");
	if (f) {
		char linebuffer[128];
		while (fgets(linebuffer, sizeof(linebuffer), f)) {
			StringUtils::chomp(linebuffer, G_N_ELEMENTS(linebuffer));
			std::string word(linebuffer);
			if (!word.empty()) {
				// Unsramble the word
				for (size_t i = 0; i < word.length(); i++) {
					word[i] = word[i]-1;
				}
				m_whitelist.insert(StringUtils::utf8tolower(word));
				//g_debug("Learned from %s white list: %s", fname.c_str(), word.c_str());
			}
		}
		err = SKERR_SUCCESS;
	}

	if (err == SKERR_SUCCESS)
		g_debug("Loaded %u whitelist words", m_whitelist.size());
	else
		g_warning("ERROR %d loading whitelist words.", err);

	return err;
}

}


