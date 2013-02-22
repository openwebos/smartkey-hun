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
#include <glib/gstdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "SmkyUserDatabase.h"
#include "SmartKeyService.h"

using namespace SmartKey;
using namespace std;

/**
* constructor
*/
SmkyUserDatabase::SmkyUserDatabase (void)
{
    init();
}

/**
* ~
*/
SmkyUserDatabase::~SmkyUserDatabase (void)
{
    m_user_database.save( _getDbPath() );
    m_context_database.save( _getContextDbPath() );
}

/**
* init
*/
void SmkyUserDatabase::init (void)
{
    m_user_database.load( _getDbPath() );
    m_context_database.load( _getContextDbPath() );
}

/**
* Loads <b>all</b> user entries into the provided list - unsorted.
*
* @param entries
*   output: entries
*
* @return SmartKeyErrorCode
*   SKERR_SUCCESS if done
*/
SmartKeyErrorCode SmkyUserDatabase::_loadEntries (std::list<string>& o_entries)
{
    o_entries.clear();
    m_user_database.exportToList(o_entries);

    return ( (o_entries.size() > 0) ? SKERR_SUCCESS : SKERR_NO_MATCHING_WORDS );
}

/**
* get entries
*
* @param offset
*   input parameter
*
* @param limit
*   input parameter
*
* @param entries
*   output parameter
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyUserDatabase::getEntries (int offset, int limit, std::list<string>& entries)
{
    if (offset < 0 || limit < 0)
        return SKERR_BAD_PARAM;

    entries.clear();

    // Inefficient to load all entries into RAM, but we need to return a sorted list
    std::list<string> allEntries;
    SmartKeyErrorCode err = _loadEntries(allEntries);
    if (err == SKERR_SUCCESS)
    {

        allEntries.sort(StringUtils::compareStrings);
        std::list<string>::const_iterator i = allEntries.begin();
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

    return ( err );
}

/**
* notification about locale change
*/
void SmkyUserDatabase::changedLocaleSettings (void)
{
    // Nothing to do: We only have one user database for all locales.
}

/**
* this function is not used !
*
* @param word
*   word
*
* @return SmartKeyErrorCode
*   SKERR_SUCCESS if done
*/
SmartKeyErrorCode SmkyUserDatabase::updateWordUsage (const string& word)
{
    if (word.empty())
        return SKERR_BAD_PARAM;

    // TODO:
    //  a)  Update m_lingInfo with usage of this word
    //  b)  Note:  This only necessary if the dictionary system, collects statistics on word usage
    //  c)  Return true status


    return SKERR_SUCCESS;
}



