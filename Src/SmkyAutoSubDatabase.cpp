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

#include "SmkyAutoSubDatabase.h"
#include "SmkyUserDatabase.h"
#include "Settings.h"
#include "SmartKeyService.h"
#include <string>

using namespace SmartKey;

/**
 * SmkyAutoSubDatabase
 */
SmkyAutoSubDatabase::SmkyAutoSubDatabase (void)
{
    m_autosub_dictionary.load( _getDbPath() );
    m_autosub_hc_dictionary.load( _getHcDbPath() );
}

/**
 * ~SmkyAutoSubDatabase
 */
SmkyAutoSubDatabase::~SmkyAutoSubDatabase (void)
{
    save();
}

/**
* Loads <b>all</b> user entries into the provided list - unsorted.
*
* @param which
*     defines type of entry:  UserEntries or StockEntries
*
* @param entries
*     write to this entries list
*
* @return SMKY_STATUS
*   = SKERR_SUCCESS always
*/
SmartKeyErrorCode SmkyAutoSubDatabase::loadEntries (WhichEntries which, std::list<Entry>& entries)
{
    entries.clear();

    // TODO:
    //  a)  Read though m_lingInfo for type of entry
    //  b)  Each entry in list is a pair of shortcut and substitution
    //  c)  Write to entries list
    //  d)  Return true status

    switch (which)
    {
    case (UserEntries) :
    {
        m_autosub_dictionary.exportToList(entries);
    }
    break;

    case (StockEntries) :
    {
        m_autosub_hc_dictionary.exportToList(entries);
    }
    break;

    case (AllEntries) :
    {
        m_autosub_hc_dictionary.exportToList(entries);
        m_autosub_dictionary.exportToList(entries);
    }
    break;
    }

    return SKERR_SUCCESS;
}

/**
* compare two words from the Entry instances
*
* @param first
*   first Entry instance
*
* @param second
*   second Entry instance
*
* @return bool
    TRUE :
    FALSE:
*/
static bool compare_entries (const Entry& first, const Entry& second)
{
    return StringUtils::compareStrings(first.shortcut, second.shortcut);
}

/**
 * Get all user entries starting from the offset
 */
SmartKeyErrorCode SmkyAutoSubDatabase::getEntries (int offset, int limit, WhichEntries which, std::list<Entry>& entries)
{
    if (offset < 0 || limit < 0)
        return SKERR_BAD_PARAM;

    entries.clear();

    std::list<Entry> allEntries;
    SmartKeyErrorCode wStatus = loadEntries(which, allEntries);
    if (wStatus == SKERR_SUCCESS)
    {
        allEntries.sort(compare_entries);
        std::list<Entry>::const_iterator i = allEntries.begin();
        // Skip up to the offset
        while (i != allEntries.end() && offset-- > 0)
        {
            ++i;
        }

        // Now read the entries
        while (i != allEntries.end() && limit-- > 0)
        {
            entries.push_back(*i++);
        }
    }

    return (wStatus);
}

/**
* Return the number of user entries
*
*
* @param which
*   input value: defines type of entry:  UserEntries or StockEntries
*
* @param entries
*   output value: number of words
*
* @return SmartKeyErrorCode
*   = SKERR_SUCCESS always
*/
SmartKeyErrorCode SmkyAutoSubDatabase::getNumEntries (WhichEntries which, int& entries)
{
    // TODO:
    //  a)  Get the number of entries of the specified type from m_lingInfo
    //  b)  Return true status

    switch (which)
    {
    case (UserEntries) :
    {
        entries = m_autosub_dictionary.size();
    }
    break;

    case (StockEntries) :
    {
        entries = m_autosub_hc_dictionary.size();
    }
    break;

    case (AllEntries) :
    {
        entries = m_autosub_dictionary.size() + m_autosub_hc_dictionary.size();
    }
    break;
    }

    return SKERR_SUCCESS;
}

/**
* init
*
* @return SmartKeyErrorCode
*   always SKERR_SUCCESS
*/
SmartKeyErrorCode SmkyAutoSubDatabase::init (void)
{
    return SKERR_SUCCESS;
}

/**
* Find the substitution (if there is one) for the given shortcut.
*
* @param shortcut
*   shortcut
*
* @return std::string
*   The substitution mapped by the shortcut or an empty string if no mapping.
*/
std::string SmkyAutoSubDatabase::getLdbSubstitution (std::string& shortcut)
{
    std::string lcshortut = StringUtils::utf8tolower(shortcut);
    return (m_autosub_dictionary.find(lcshortut));
}

/**
* Find the substitution (if there is one) for the given shortcut.
*
* @param shortcut
*   <perameter description>
*
* @return std::string
*   The substitution mapped by the shortcut or an empty string if no mapping.
*/
std::string SmkyAutoSubDatabase::getHardCodedSubstitution (std::string& shortcut)
{
    std::string lcshortut = StringUtils::utf8tolower(shortcut);
    return(m_autosub_hc_dictionary.find(shortcut));
}

/**
* notification about locale change
*/
void SmkyAutoSubDatabase::changedLocaleSettings (void)
{
    g_debug("AutoSubDB: got notification about locale settings change");

    save();
    m_autosub_dictionary.load( _getDbPath() );
    m_autosub_hc_dictionary.load( _getHcDbPath() );
}



