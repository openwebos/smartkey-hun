/* @@@LICENSE
*
*      Copyright (c) 2010-2013 Hewlett-Packard Development Company, L.P.
*      Copyright (c) 2013 LG Electronics
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
#include "Database.h"
#include "SmkyAutoSubDatabase.h"
#include "SmkyManufacturerDatabase.h"
#include "SmkyHunspellDatabase.h"
#include "Settings.h"
#include "SmartKeyService.h"
#include "SmkySpellCheckEngine.h"
#include <pbnjson.hpp>

// debug macros for calls. Sets the wStatus variable that must be defined already. Declare a local status variable of type SMKY_STATUS.
#define SMKY_VERIFY(x) (G_LIKELY((wStatus = x) == SMKY_STATUS_NONE) || (g_warning("'%s' returned error #%d, in %s of %s line %d", #x, wStatus, __FUNCTION__, __FILE__, __LINE__), false))
#define SMKY_VERIFY2(x, OKerror) (G_LIKELY((wStatus = x) == SMKY_STATUS_NONE) || (wStatus == OKerror) || (g_warning("'%s' returned error #%d, in %s of %s line %d", #x, wStatus, __FUNCTION__, __FILE__, __LINE__), false))

using namespace SmartKey;

/**
* SmkySpellCheckEngine
*/
SmkySpellCheckEngine::SmkySpellCheckEngine (void)
{
    mp_hunspDb = new SmkyHunspellDatabase();
    mp_autoSubDb = new SmkyAutoSubDatabase();
    mp_userDb = new SmkyUserDatabase();
    mp_manDb = new SmkyManufacturerDatabase();

    m_initialized = mp_hunspDb && mp_autoSubDb && mp_userDb && mp_manDb;

    if(m_initialized)
        g_message("SpellCheckEngine successfully initialized.");
    else
    {
        g_warning("ERROR initializing SpellCheckEngine.");
        _clean();
    }

    //init locale words
    m_locale_dictionary.load(Settings::getInstance()->getDBFilePath(Settings::DICT_LOCALE),
                             Settings::getInstance()->getDBFilePath(Settings::DICT_LOCALE, Settings::DICT_LOCALE_DEPEND));

    //load whitelist
    m_white_dictionary.load(Settings::getInstance()->getDBFilePath(Settings::DICT_WHITE),
                            Settings::getInstance()->getDBFilePath(Settings::DICT_WHITE, Settings::DICT_LOCALE_DEPEND));

    //init list of supported languages (need to move it to configuration file!)
    m_languages.add("en");
    m_languages.add("es");
    m_languages.add("fr");
    m_languages.add("de");
    m_languages.add("it");
}

/**
* SmkySpellCheckEngine
*/
SmkySpellCheckEngine::~SmkySpellCheckEngine()
{
    _clean();
}

/**
* clean
*/
void SmkySpellCheckEngine::_clean (void)
{
    m_initialized = false;

    if (mp_hunspDb)
    {
        delete mp_hunspDb;
        mp_hunspDb = NULL;
    }

    if (mp_autoSubDb)
    {
        delete mp_autoSubDb;
        mp_autoSubDb = NULL;
    }

    if (mp_userDb)
    {
        delete mp_userDb;
        mp_userDb = NULL;
    }

    if (mp_manDb)
    {
        delete mp_manDb;
        mp_manDb = NULL;
    }
}

/**
* get supported languages
*
* @return char*
*   return string like '{"languages":["en_un","es_un","fr_un","de_un","it_un"]}'
*/
const char*  SmkySpellCheckEngine::getSupportedLanguages()
{
    string retval = "{\"languages\":[";

    list<std::string> entries;
    m_languages.exportToList(entries);

    for ( list<std::string>::iterator it = entries.begin(); it != entries.end(); ++it )
    {
        if(it != entries.begin())
            retval += ",";

        retval += "\"" + *it + "_un\"";
    }

    retval += "]}";

    return (retval.c_str()); //"{\"languages\":[\"en_un\",\"es_un\",\"fr_un\",\"de_un\",\"it_un\"]}";
}

/**
* is current language supported ?
*
* @return bool
*   true if current language is supported
*/
bool SmkySpellCheckEngine::_isCurrentLanguageSupported (void)
{
    return(m_languages.find(Settings::getInstance()->localeSettings.m_inputLanguage));
}

/**
* test word for all digits inside
*
* @param word
*   word to test
*
* @return bool
*   true if it is
*/
bool SmkySpellCheckEngine::_wordIsAllDigits (const std::string& word)
{
    std::string::const_iterator i;
    for (i = word.begin(); i != word.end(); ++i)
    {
        if (!(isdigit(*i) || *i == '.'))
            return false;
    }

    return true;
}

/**
* check spelling
*
* @param word
*   word to check
*
* @param result
*   output: result
*
* @param maxGuesses
*   number of words in result
*
* @return SmartKeyErrorCode
*   SKERR_SUCCESS if done
*/
SmartKeyErrorCode SmkySpellCheckEngine::checkSpelling (const std::string& word, SpellCheckWordInfo& result, int maxGuesses)
{
    result.clear();

    if ( !m_initialized )
        return SKERR_FAILURE;

    // DONE:
    //  a) If current languuage not supported in a loaded dictionary, set result.inDictionary=true; and return success
    if ( !_isCurrentLanguageSupported() )
    {
        result.inDictionary = true;
        return SKERR_SUCCESS;
    }

    //  b) If whitelist is non-empty. check it.  If found set result.inDictionary=true; and return success
    if ( m_white_dictionary.find(word) )
    {
        result.inDictionary = true;
        return SKERR_SUCCESS;
    }

    //  c) Check word in auto sub dictionary.
    string auto_subdb_word = mp_autoSubDb->findEntry(word);
    WordGuess word_guess;

    if ( auto_subdb_word.length() > 0 )
    {
        // If found: add to result.guesses: .spellCorrection=false; .autoReplace=true; .autoAccept=true;
        word_guess.guess = auto_subdb_word;
        word_guess.spellCorrection = false;
        word_guess.autoReplace = true;
        word_guess.autoAccept = true;
        result.guesses.push_back(word_guess);
    }

    //  d) If word is all digits, set result.inDictionary=true; and return success
    if (_wordIsAllDigits(word))
    {
        result.inDictionary = true;
        return SKERR_SUCCESS;
    }

    //  e) If no result found look word up in dictionaries: manufacturer and user (person and context)
    if ( mp_manDb->findEntry(word) || mp_userDb->findWord(word) )
    {
        result.inDictionary = true;
        return SKERR_SUCCESS;
    }

    //  f) If word not found in dictionaries, get a list of guesses from dictionaries.
    if ( mp_hunspDb->findGuesses(word, result, maxGuesses) == SKERR_SUCCESS)
    {
        result.inDictionary = true;
        return SKERR_SUCCESS;
    }

    //  g) If word not found in dictionaries, but auto-sub had a match, set that to auto accept
    //      otherwise select best matching guess for autoaccept flag

    //  [Igor: this is done already in section c) ]

    //  h) Return valid error code
    return SKERR_SUCCESS;
}

/**
* auto correct
*
* @param word
*   word to correct
*
* @param context
*   context (parameter is not used)
*
* @param result
*   result
*
* @param maxGuesses
*   number of words in result
*
* @return SmartKeyErrorCode
*   SKERR_SUCCESS if done
*/
SmartKeyErrorCode SmkySpellCheckEngine::autoCorrect (const std::string& word, const std::string& context, SpellCheckWordInfo& result, int maxGuesses)
{
    result.clear();

    //  a) If current languuage not supported in a loaded dictionary, set result.inDictionary=true; and return success
    if ( !_isCurrentLanguageSupported() )
    {
        result.inDictionary = true;
        return SKERR_SUCCESS;
    }

    //  b) If whitelist is non-empty. check it.  If found set result.inDictionary=true; and return success
    if ( m_white_dictionary.find(word) )
    {
        result.inDictionary = true;
        return SKERR_SUCCESS;
    }

    //  c) Check word in auto sub dictionary.
    //         If found: add to result.guesses: .spellCorrection=false; .autoReplace=true; .autoAccept=true;
    string auto_subdb_word = mp_autoSubDb->findEntry(word);
    WordGuess word_guess;

    if ( auto_subdb_word.length() > 0 )
    {
        word_guess.guess = auto_subdb_word;
        word_guess.spellCorrection = false;
        word_guess.autoReplace = true;
        word_guess.autoAccept = true;
        result.guesses.push_back(word_guess);
    }

    //  d) If word is all digits, set result.inDictionary=true; and return success
    if (_wordIsAllDigits(word))
    {
        result.inDictionary = true;
        return SKERR_SUCCESS;
    }

    //  e) If no result found look word up in dictionaries
    if ( mp_manDb->findEntry(word) || mp_userDb->findWord(word) )
    {
        result.inDictionary = true;
        return SKERR_SUCCESS;
    }

    //  f) If word not found in dictionaries, get a list of guesses from dictionaries.
    if ( mp_hunspDb->findGuesses(word, result, maxGuesses) == SKERR_SUCCESS)
    {
        result.inDictionary = true;
        return SKERR_SUCCESS;
    }

    //  g) If word not found in dictionaries, but auto-sub had a match, set that to auto accept
    //      otherwise select best matching guess for autoaccept flag

    //  [Igor: this is done already in section c) ]

    //  h) Return valid error code
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
* nothing to do yet
*
* @param result
*   result
*
* @param maxGuesses
*   number of words in result
*
* @return SmartKeyErrorCode
*   SKERR_SUCCESS if done
*/
SmartKeyErrorCode SmkySpellCheckEngine::_getSelectionResults (SpellCheckWordInfo& result, int maxGuesses)
{
    result.clear();
    SmartKeyErrorCode wStatus = SKERR_SUCCESS;
    return wStatus;
}

/* public */
/**
* process trace (this function is nothing to do yet)
*
* @param points
*   <parameter description>
*
* @param shift
*   <parameter description>
*
* @param firstChars
*   <parameter description>
*
* @param lastChars
*   <parameter description>
*
* @param result
*   result
*
* @param maxGuesses
*   number of words in result
*
* @return SmartKeyErrorCode
*   SKERR_SUCCESS if done
*/
SmartKeyErrorCode SmkySpellCheckEngine::processTrace(const std::vector<unsigned int>& points, EShiftState shift, const std::string& firstChars, const std::string& lastChars, SpellCheckWordInfo& result, int maxGuesses)
{
    result.clear();
    return SKERR_SUCCESS;
}

/* public */
/**
* process taps (this function is nothing to do yet)
*
* @param taps
*   <parameter description>
*
* @param result
*   result
*
* @param maxGuesses
*   number of words in result
*
* @return SmartKeyErrorCode
*   SKERR_SUCCESS if done
*/
SmartKeyErrorCode SmkySpellCheckEngine::processTaps (const TapDataArray& taps, SpellCheckWordInfo& result, int maxGuesses)
{
    result.clear();

    // TODO:  What does this do?

    return SKERR_SUCCESS;
}

/**
* get completion
*
* @param prefix
*   look for word starting with this prefix
*
* @param result
*   result word
*
* @return SmartKeyErrorCode
*   return code
*/
SmartKeyErrorCode SmkySpellCheckEngine::getCompletion (const std::string& prefix, std::string& result)
{
    result.clear();

    if (!m_initialized)
        return(SKERR_FAILURE);

    if (prefix.empty())
        return(SKERR_BAD_PARAM);

    //  TODO:  This may not be necessary, but just in case:
    //  a) If all digits, don't try to complete
    if (_wordIsAllDigits(prefix))
    {
        return SKERR_SUCCESS;
    }

    //  b) If there's a match in the auto sub database, use that
    string auto_subdb_word = mp_autoSubDb->findEntry(prefix);

    if ( auto_subdb_word.length() > 0 )
    {
        result = auto_subdb_word;
    }
    else
    {
        //  c) Otherwise lookup the prefix (partially entered string) in the dictionaries to get an autocompletion match, if there is one

    }
    //  d) Return valid error code
    return(SKERR_SUCCESS);
}

/**
* this notification tell about locale settings change.
* take a look into Settings::localeSettings - a new values was set there
*/
void SmkySpellCheckEngine::changedLocaleSettings (void)
{
    if (m_initialized)
    {
        //load locale words
        m_locale_dictionary.load( _getLocaleIndependDbPath(), _getLocaleDependDbPath() );

        //load whitelist
        m_white_dictionary.load( _getWhitelistIndependDbPath(), _getWhitelistDependDbPath() );

        mp_hunspDb->changedLocaleSettings();
    }
}

