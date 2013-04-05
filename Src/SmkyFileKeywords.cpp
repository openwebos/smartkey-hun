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

#include "SmkyFileKeywords.h"
#include <glib.h>
#include <fstream>

#if 0 // Debug settings
#define DEBUG_STUB g_debug
#else
#define DEBUG_STUB(fmt, args...) (void)0
#endif

using namespace SmartKey;
using namespace std;

/**
*
*/
SmkyFileKeywords::SmkyFileKeywords()
{
    m_initialized = false;
    m_changed = false;
}

/**
* ~
*/
SmkyFileKeywords::~SmkyFileKeywords()
{
    _clean();
}

/**
* clean
*
*/
void SmkyFileKeywords::_clean (void)
{
    if (!m_dictionary.empty())
    {
        g_debug("FileKeywordsDB: going to release current dictionary..");
        m_dictionary.clear();
        g_debug("FileKeywordsDB: done, no dictionaries");
    }

    m_initialized = false;
    m_changed = false;
}

/**
* read pairs from text file and add them to set<>
*
* @param i_db_file
*   = path + filename
*/
void SmkyFileKeywords::_importFileDB (std::string i_db_file)
{
    ifstream fin(i_db_file.c_str());

    if (fin.is_open())
    {
        std::string line;

        while ( !fin.eof() )
        {
            getline(fin, line);
            m_dictionary.insert( line );
        }
    }
    else
    {
        g_debug("FileKeywordsDB: can't open dictionary file: %s", i_db_file.c_str());
    }
}

/**
* load dictionary from the file
*
* @return bool
*   true if loaded
*/
bool SmkyFileKeywords::load (string i_locale_path_file)
{
    if(i_locale_path_file.length() == 0)
    {
        return(false);
    }

    _clean();

    if ( g_file_test( i_locale_path_file.c_str(), G_FILE_TEST_EXISTS ) )
    {
        g_debug("FileKeywordsDB: going to load dictionary for locale");
        _importFileDB( i_locale_path_file );
    }

    m_initialized = !m_dictionary.empty();

    if (m_initialized)
    {
        g_debug("FileKeywordsDB: dictionary was loaded successfuly.");
    }

    return(m_initialized);
}

/**
* save dictionary to file
*
* @return bool
*   true if saved
*/
bool SmkyFileKeywords::save (std::string i_db_file)
{
    bool retval = false;

    if(i_db_file.length() == 0)
    {
        return(false);
    }

    if ( !m_changed )
    {
        return(true);
    }

    if ( !m_dictionary.empty() )
    {
        SmkyHashSet::iterator it;

        std::fstream file(i_db_file.c_str(), std::ios::out);

        for ( it = m_dictionary.begin(); it != m_dictionary.end(); ++it )
        {
            file << *it << std::endl;
        }

        retval = true;
    }

    return(retval);
}

/**
* add a new word
*/
void SmkyFileKeywords::add (std::string i_key)
{
    m_dictionary.insert( i_key );
    m_changed = true;
}

/**
* remove pair by key
*
* @return bool
*   true if removed
*/
bool SmkyFileKeywords::remove (std::string i_key)
{
    if (!m_dictionary.empty())
    {
        SmkyHashSet::iterator it;
        it = m_dictionary.find( i_key );

        if (it != m_dictionary.end())
        {
            m_dictionary.erase( it );
            m_changed = true;
            return(true);
        }
    }

    return(false);
}

/**
* is word is present in dictionary ?
*
* @param shortcut
*   input word
*
* @return bool
*   true if exist
*/
bool SmkyFileKeywords::find (const std::string& shortcut)
{
    if (!m_dictionary.empty())
    {
        SmkyHashSet::iterator it;
        it = m_dictionary.find(shortcut);

        if (it != m_dictionary.end())
        {
            return(true);
        }
    }

    return(false);
}

/**
* is word with specified prefix exist in dictionary ?
*
* @param shortcut
*   key word to search
*
* @return std::string
*   word assotiated with prefix word
*/

std::string SmkyFileKeywords::find_by_prefix (const std::string& prefix)
{
    if (!m_dictionary.empty())
    {
        SmkyHashSet::iterator it;
        std::pair<std::string::const_iterator, std::string::const_iterator> res;

        for (it = m_dictionary.begin(); it != m_dictionary.end(); ++it )
        {
            res = std::mismatch(prefix.begin(), prefix.end(), (*it).begin());

            if (res.first == prefix.end())
            {
                return(*it);
            }
        }
    }

    return "";
}

/**
* export all words from the dictionary
*
* @param o_entries
*   output: list of words
*/
void SmkyFileKeywords::exportToList (std::list<string>& o_entries)
{
    if ( !m_dictionary.empty() )
    {
        SmkyHashSet::iterator it;

        for ( it = m_dictionary.begin(); it != m_dictionary.end(); ++it )
        {
            o_entries.push_back(*it);
        }
    }
}

