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

#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include <cctype>
#include "SmkyHunspellDatabase.h"
#include "Settings.h"

using namespace SmartKey;

/**
* SmkyHunspellDatabase
*/
SmkyHunspellDatabase::SmkyHunspellDatabase (void)
{
    //m_initialized = false;
    mp_dict_base = NULL;

    _loadDictionary();
}

/**
* ~SmkyHunspellDatabase
*/
SmkyHunspellDatabase::~SmkyHunspellDatabase (void)
{
    _clean();
}

/**
* clean
*/
void SmkyHunspellDatabase::_clean (void)
{
    if (mp_dict_base)
    {
        g_debug("Hunspell: going to release current dictionary..");
        delete mp_dict_base;
        mp_dict_base = NULL;
        g_debug("Hunspell: done, no dictionaries");
    }

    m_initialized = false;
}

/**
* load dictionary
*/
void SmkyHunspellDatabase::_loadDictionary (void)
{
    //
    // constract path to dictionary and aff-file 'on the fly' using settings
    //
    Settings* p_settings = Settings::getInstance();

    string locale = p_settings->localeSettings.getLanguageCountryLocale();
    string aff_path = p_settings->getDBFilePath(Settings::DICT_HUNSPELL, Settings::DICT_HUNSPELL_AFF);
    string dict_path = p_settings->getDBFilePath(Settings::DICT_HUNSPELL, Settings::DICT_HUNSPELL_DIC);

    _clean();

    if ( (g_file_test(aff_path.c_str(), G_FILE_TEST_EXISTS)) &&
            (g_file_test(dict_path.c_str(), G_FILE_TEST_EXISTS)) )
    {
        g_debug("Hunspell: going to load dictionary for locale '%s'", locale.c_str());

        mp_dict_base = new Hunspell( aff_path.c_str(), dict_path.c_str(), NULL );
        m_initialized = mp_dict_base != NULL;

        if (m_initialized)
        {
            g_debug("Hunspell: dictionaries was loaded successfuly.");
        }
    }
    else
    {
        //
        // hunspell dictionary was not found for the selected locale,
        //
        g_debug("Hunspell: .aff and .dic files for locale are missing.");
    }
}

/**
* notification about locale change
*/
void SmkyHunspellDatabase::changedLocaleSettings (void)
{
    g_debug("Hunspell: got notification: locale settings changed");
    _loadDictionary();
}

/**
* find
*
* @param word
*   word to find
*
* @return bool
*   true if it was found
*/
bool SmkyHunspellDatabase::findEntry (const std::string& word)
{
    if (m_initialized)
    {
        const char* p_search_word = word.c_str();
        int info;

        int res = mp_dict_base->spell( p_search_word, &info );

        return( res != 0 ); //is good word?
    }
    else
        g_debug("Hunspell: dictionary is not loaded, find entry request ignored");

    return false;
}

/**
* find guesses
*
* @param word
*   word to search for
*
* @param result
*   output value: result of search
*
* @param maxGuesses
*   limit number of guesses for result
*
* @return SmartKeyErrorCode
*   - SKERR_SUCCESS if SmkyHunspellDatabase instance is initialized
*   - SKERR_FAILURE if not
*/
SmartKeyErrorCode SmkyHunspellDatabase::findGuesses (const std::string& word, SpellCheckWordInfo& result, int maxGuesses)
{
    if (m_initialized)
    {
        int res;
        WordGuess word_guess;
        char** p_slst;
        const char* p_search_word = word.c_str();

        res = mp_dict_base->suggest( &p_slst, p_search_word );

        if (res > 0) // have suggestion(s)!
        {
            for (int i = 0; i < std::min(res, maxGuesses); ++i)
            {
                word_guess.guess = p_slst[i];
                result.guesses.push_back(word_guess);
            }

            mp_dict_base->free_list( &p_slst, res );
        }
    }
    else
        g_debug("Hunspell: dictionary is not loaded, spell check request ignored");

    return( m_initialized ? SKERR_SUCCESS : SKERR_FAILURE );
}



