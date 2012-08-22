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

#ifndef SPELL_CHECK_ENGINE_H
#define SPELL_CHECK_ENGINE_H

#include <string>
#include <list>
#include <vector>

namespace SmartKey {

enum SmartKeyErrorCode {
	SKERR_SUCCESS = 0,			// No error
	SKERR_FAILURE = 1,			// General failure (try not to use)
	SKERR_DISABLED = 2,			// SmartKey service disabled.
	SKERR_NOMEMORY = 3,			// Insufficient memory
	SKERR_UNSUPPORTED = 4,		// Feature not supported
	SKERR_MISSING_PARAM = 5,	// Missing one or more parameters
	SKERR_BAD_PARAM = 6,		// Invalid parameter
	SKERR_WORD_EXISTS = 7,		// Word already exists
	SKERR_NO_MATCHING_WORDS = 8,// No matching words (like for delete, etc.)
};

struct SpellCheckWordInfo;
class TapDataArray;

enum EShiftState {
    eShiftState_off = 0,
    eShiftState_once,
    eShiftState_lock
};

struct LocaleSettings {

    std::string m_deviceLanguage;
    std::string m_deviceCountry;
    std::string m_inputLanguage;
    std::string m_keyboardLayout;

    std::string getLanguageCountryLocale() const    { return m_inputLanguage + '_' + m_deviceCountry; }
    std::string getFullLocale() const               { return m_deviceLanguage + '_' + m_deviceCountry + ", " + m_keyboardLayout + '-' + m_inputLanguage; }

    // find file at path starting with pathPrefix + languageCode + '_' + countryCode + pathSuffix
    // try appropriate language & country code combos until a file (even empty) is found, or return ""
    std::string findLocalResource(const std::string& pathPrefix, const char * pathSuffix) const;

};

/**
 * The data (mostly a dictionary) used by the SpellCheckEngine.
 */
class Database
{
public:
	/**
	 * Add a word to this database.
	 *
	 * @return true if successfully added, false if not.
	 */
	virtual SmartKeyErrorCode learnWord(const std::string& word) = 0;

	/**
	 * Un-learn a word.
	 *
	 * @return true if successfully added, false if not.
	 */
	virtual SmartKeyErrorCode forgetWord(const std::string& word) = 0;

	/**
	 * Write the database to filesystem.
	 */
	virtual SmartKeyErrorCode save() = 0;

	virtual SmartKeyErrorCode setLocaleSettings(const LocaleSettings& localeSettings) = 0;


	/**
	 * Allow database to expect a particular number of entries, so that it can optimize its storage
	 */
	virtual SmartKeyErrorCode setExpectedCount(int count)		{ return SKERR_UNSUPPORTED; }

protected:
	Database() {}
};


class UserDatabase : public Database
{
public:

	/**
	 * Return a list of words in this database.
	 *
	 * @return true if successfully added, false if not.
	 */
	virtual SmartKeyErrorCode getEntries(int offset, int limit, std::list<std::string>& entries) = 0;
	
	/**
	 * Return the number of entries in this db.
	 */
	virtual SmartKeyErrorCode getNumEntries(int& entries) = 0;

    /**
     * Updates usage statistics of a word. It will add it to the database if the 
     * word is not already known.
     */
    virtual SmartKeyErrorCode updateWordUsage(const std::string& word) = 0;

protected:

	UserDatabase() {}

};

class AutoSubDatabase : public Database
{
public:
	enum WhichEntries {
		UserEntries,	///< Entries added by the user.
		StockEntries,	///< Stock entries.
		AllEntries		///< All entries.
	};

	struct Entry {
		std::string shortcut;
		std::string substitution;
	};

	/**
	 * Add an auto-replace entry to this database.
	 *
	 * @return true if successfully added, false if not.
	 */
	virtual SmartKeyErrorCode addEntry(const Entry& entry) = 0;

	/**
	 * Return a list of entires from this database.
	 *
	 * @return true if successfully added, false if not.
	 */
	virtual SmartKeyErrorCode getEntries(int offset, int limit, WhichEntries which, std::list<Entry>& entries) = 0;

	/**
	 * Return the number of auto-replace entries.
	 */
	virtual SmartKeyErrorCode getNumEntries(WhichEntries which, int& entries) = 0;

	/**
	 * Return the substitution for the given shortcut.
	 *
	 * The substitution string returned will have it's case adjusted to match the shortcut.
	 */
	virtual std::string findEntry(const std::string& shortcut) = 0;

protected:
	AutoSubDatabase() {}
};


/**
 * The main engine that does the spell checking of a word.
 */
class SpellCheckEngine
{
public:

	virtual ~SpellCheckEngine() {}

	/**
	 * Check the spelling of a word.
	 *
	 * @param word       The word to check the spelling of.
	 * @param result     The spelling results for word.
	 * @param maxGuesses The maximum number of guesses to return.
	 *
	 * @return true if successful, false if not.
	 */
	virtual SmartKeyErrorCode checkSpelling(const std::string& word, SpellCheckWordInfo& result, int maxGuesses) = 0;

	/**
	 * Attempt to auto-correct a word based on key locality.
	 *
	 * @param word       The word to try to correct.
	 * @param context    The context prior to the word to correct (beginning of sentence?)
	 * @param result     The spelling results for word.
	 * @param maxGuesses The maximum number of guesses to return.
	 *
	 * @return true if successful, false if not.
	 */
	virtual SmartKeyErrorCode autoCorrect(const std::string& word, const std::string& context, SpellCheckWordInfo& result, int maxGuesses) = 0;

	/**
	 * Process a trace on a virtual keyboard.
	 *
	 * @param points		The trace sequence.
	 * @param shiftState	The state of shift.
	 * @param result		The results of the tap sequence.
	 * @param maxGuesses	The maximum number of results to return.
	 */
	virtual SmartKeyErrorCode processTrace(const std::vector<unsigned int>& points, EShiftState shift, const std::string& firstChars, const std::string& lastChars, SpellCheckWordInfo& result, int maxGuesses) = 0;

	/**
	 * Process a sequence of taps on a virtual keyboard.
	 *
	 * @param taps			The sequence of taps.
	 * @param result		The results of the tap sequence.
	 * @param maxGuesses	The maximum number of results to return.
	 */
	virtual SmartKeyErrorCode processTaps(const TapDataArray& taps, SpellCheckWordInfo& result, int maxGuesses) = 0;

	/**
	 * Find the best word completion for a given prefix.
	 *
	 * @param prefix		The prefix to try to complete.
	 * @param result		The completed word based on the prefix.
	 */
	virtual SmartKeyErrorCode getCompletion(const std::string& prefix, std::string &result) = 0;

	/**
	 * Return the user (read/write) database.
	 */
	virtual UserDatabase* getUserDatabase() = 0;

	/**
	 * Return the auto substitution (read/write) database.
	 */
	virtual AutoSubDatabase* getAutoSubDatabase() = 0;

	/**
	 * Return the manufacturer database (where contacts and other volatile words are stored).
	 */
	virtual Database* getManufacturerDatabase() = 0;

	/**
	 * Set the current locale, including input language, keyboard layout, UI language & country code.
	 * @param localeSettings	The UI language & country codes, input language code, keyboard layout...
	 */
	virtual bool setLocaleSettings(const LocaleSettings& localeSettings, bool isVirtualKeyboard) = 0;

	/**
	 * Get supported correction languages as a json message. Example: {"languages":{"en_un","es_un","fr_un","de_un","it_un"}}
	 */
	virtual const char * getSupportedLanguages() const = 0;
};

}

#endif
