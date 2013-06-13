/* @@@LICENSE
*
*      Copyright (c) 2010-2013 LG Electronics, Inc.
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

#include <set>
#include <string>
#include "SmkyManufacturerDatabase.h"
#include "SmkyUserDatabase.h"
#include "SmkyAutoSubDatabase.h"
#include "StringUtils.h"
#include "SmkyKeywordsBundle.h"
#include "SpellCheckClient.h"

namespace SmartKey
{
class SmkyHunspellDatabase;
struct SpellCheckWordInfo;

const size_t SEL_LIST_SIZE = 32;

enum EShiftState
{
    eShiftState_off = 0,
    eShiftState_once,
    eShiftState_lock
};

/**
 * Our wrapper around the spell check engine.
 */
class SmkySpellCheckEngine
{
private:
    //is engine was initialized successfuly ?
    bool                      m_initialized;

    //list of supported languages
    SmkyFileKeywords          m_languages;

    //hunspell dictionary
    SmkyHunspellDatabase*     mp_hunspDb;

    //user db
    SmkyUserDatabase*         mp_userDb;

    //man db
    SmkyManufacturerDatabase* mp_manDb;

    //sub db
    SmkyAutoSubDatabase*      mp_autoSubDb;

    //locale words
    SmkyKeywordsBundle        m_locale_dictionary;

    //white list
    SmkyKeywordsBundle        m_white_dictionary;

public:

    SmkySpellCheckEngine(void);
    virtual ~SmkySpellCheckEngine();

    //spell check word
    virtual SmartKeyErrorCode checkSpelling (const std::string& word, SpellCheckWordInfo& result, int maxGuesses);

    //try to correct word
    virtual SmartKeyErrorCode autoCorrect (const std::string& word, const std::string& context, SpellCheckWordInfo& result, int maxGuesses);

    //get completion for the word
    virtual SmartKeyErrorCode getCompletion (const std::string& prefix, std::string& result);

    //get user db instance
    virtual SmkyUserDatabase* getUserDatabase (void);

    //get autosub db instance
    virtual SmkyAutoSubDatabase* getAutoSubDatabase (void);

    //get manufacturer db instance
    virtual SmkyManufacturerDatabase* getManufacturerDatabase (void);

    //used for notification class instance about locale change
    virtual void changedLocaleSettings (void);

    //get list of supported languages
    virtual const char* getSupportedLanguages (void);

    //process trace
    virtual SmartKeyErrorCode processTrace (const std::vector<unsigned int>& points, EShiftState shift, const std::string& firstChars, const std::string& lastChars, SpellCheckWordInfo& result, int maxGuesses);

    //process taps
    virtual SmartKeyErrorCode processTaps (const TapDataArray& taps, SpellCheckWordInfo& result, int maxGuesses);


private:
    //is current language supported?
    bool _isCurrentLanguageSupported (void);

    //release all allocated objects
    void  _clean (void);

    //get selection results
    SmartKeyErrorCode _getSelectionResults (SpellCheckWordInfo& result, int maxGuesses);

    //verify word for all digits
    static bool _wordIsAllDigits (const std::string& word);

    //get path to locale independent db
    std::string _getLocaleIndependDbPath (void) const;

    //get path to locale dependent db
    std::string _getLocaleDependDbPath (void) const;

    //get path to locale independent whitelist db
    std::string _getWhitelistIndependDbPath (void) const;

    //get path to locale dependent whitelist db
    std::string _getWhitelistDependDbPath (void) const;
};

/**
* Return the auto-substitution (read/write) database.
*
* @return AutoSubDatabase*
*   <return value description>
*/
inline SmkyUserDatabase* SmkySpellCheckEngine::getUserDatabase (void)
{
    return(mp_userDb);
}

/**
* Return the auto-substitution (read/write) database.
*
* @return AutoSubDatabase*
*   <return value description>
*/
inline SmkyAutoSubDatabase* SmkySpellCheckEngine::getAutoSubDatabase (void)
{
    return(mp_autoSubDb);
}

/**
* return manufacturer database
*
* @return SmkyManufacturerDatabase*
*   <return value description>
*/
inline SmkyManufacturerDatabase* SmkySpellCheckEngine::getManufacturerDatabase (void)
{
    return(mp_manDb);
}

/**
* get path to locale independent db
*
* @return string
*   <return value description>
*/
inline std::string SmkySpellCheckEngine::_getLocaleIndependDbPath (void) const
{
    return(Settings::getInstance()->getDBFilePath(Settings::DICT_LOCALE));
}

/**
* get path to locale dependent db
*
* @return string
*   <return value description>
*/
inline std::string SmkySpellCheckEngine::_getLocaleDependDbPath (void) const
{
    return(Settings::getInstance()->getDBFilePath(Settings::DICT_LOCALE, Settings::DICT_LOCALE_DEPEND));
}

/**
* get path to locale independent whitelist db
*
* @return string
*   <return value description>
*/
inline std::string SmkySpellCheckEngine::_getWhitelistIndependDbPath (void) const
{
    return(Settings::getInstance()->getDBFilePath(Settings::DICT_WHITE));
}

/**
* get path to locale dependent whitelist db
*
* @return string
*   <return value description>
*/
inline std::string SmkySpellCheckEngine::_getWhitelistDependDbPath (void) const
{
    return(Settings::getInstance()->getDBFilePath(Settings::DICT_WHITE, Settings::DICT_LOCALE_DEPEND));
}

}

#endif
