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

#ifndef SMKY_HUNSPELL_DATABASE_H
#define SMKY_HUNSPELL_DATABASE_H

#include <string>
#include "Database.h"
#include "SpellCheckClient.h"
#include <hunspell/hunspell.hxx>

namespace SmartKey
{
/**
 * Wraps an Hunspell database.
 */
class SmkyHunspellDatabase
{
private:
    //is engine was initialized and db loaded successfuly ?
    bool      m_initialized;

    //hunspell object
    Hunspell* mp_dict_base;

public:

    SmkyHunspellDatabase (void);
    virtual ~SmkyHunspellDatabase (void);
    void changedLocaleSettings (void);

    bool isLoaded (void);
    bool findEntry (const std::string& word);

    SmartKeyErrorCode findGuesses (const std::string& word, SpellCheckWordInfo& result, int maxGuesses);

private:
    //release all allocated objects
    void _clean (void);

    //load dictionary according to current locale settings
    void _loadDictionary (void);

};

/**
 * Is loaded default database ?
 * indicate situation when loaded database is different to combination of
 * Settings::m_inputLanguage and Settings::m_deviceCountry
 */
inline bool SmkyHunspellDatabase::isLoaded (void)
{
    return m_initialized;
}

}

#endif

