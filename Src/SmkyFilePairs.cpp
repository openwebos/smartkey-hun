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

#include "SmkyFilePairs.h"
#include <glib.h>
#include <fstream>
#include <boost/tokenizer.hpp>

#if 0 // Debug settings
#define DEBUG_STUB g_debug
#else
#define DEBUG_STUB(fmt, args...) (void)0
#endif

using namespace SmartKey;
using namespace std;

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

/**
*
*/
SmkyFilePairs::SmkyFilePairs()
{
    m_initialized = false;
    m_changed = false;
}

/**
* ~
*/
SmkyFilePairs::~SmkyFilePairs()
{
    _clean();
}

/**
* clean
*
*/
void SmkyFilePairs::_clean (void)
{
    if (!m_dictionary.empty())
    {
        g_debug("FilePairsDB: going to release current dictionary..");
        m_dictionary.clear();
        g_debug("FilePairsDB: done, no dictionaries");
    }

    m_initialized = false;
    m_changed = false;
}

/**
* read pairs from text file and add them to map<>
*
* @param i_db_file
*   = path + filename to db
*
*/
void SmkyFilePairs::_importFileDB (std::string i_db_file)
{
    boost::char_separator<char> sep("|");
    ifstream fin(i_db_file.c_str());

    if (fin.is_open())
    {
        std::string line;
        std::string key;

        while ( !fin.eof() )
        {
            getline(fin, line);
            tokenizer tokens(line, sep);
            tokenizer::iterator tok_iter = tokens.begin();

            if (tok_iter == tokens.end())
                continue;

            key = *tok_iter;
            ++tok_iter;

            if (tok_iter != tokens.end())
            {
                m_dictionary.insert( std::pair<std::string,std::string>(key, *tok_iter) );
            }
        }
    }
    else
    {
        g_debug("FilePairsDB: can't open dictionary file: %s", i_db_file.c_str());
    }
}

/**
* load dictionary from the file
*
* @return bool
*   true if loaded
*/
bool SmkyFilePairs::load (string i_locale_path_file)
{
    if(i_locale_path_file.length() == 0)
    {
        return(false);
    }

    _clean();

    if ( g_file_test(i_locale_path_file.c_str(), G_FILE_TEST_EXISTS) )
    {
        g_debug("FilePairsDB: going to load dictionary for locale");
        _importFileDB(i_locale_path_file);
    }

    m_initialized = !m_dictionary.empty();

    if (m_initialized)
    {
        g_debug("FilePairsDB: dictionary was loaded successfuly.");
    }

    return(m_initialized);
}

/**
* save dictionary to the file,
* word pairs are saved in text mode and separated with symbol '|'
*
* @return bool
*   true if saved
*/
bool SmkyFilePairs::save (std::string i_db_file)
{
    if(i_db_file.length() == 0)
    {
        return(false);
    }

    if ( m_changed && (!m_dictionary.empty()) )
    {
        SmkyHashMap::iterator it;

        std::fstream file(i_db_file.c_str(), std::ios::out);

        for ( it = m_dictionary.begin(); it != m_dictionary.end(); ++it )
        {
            file << it->first << "|" << it->second << std::endl;
        }
    }

    return(true);
}

/**
* add pair
*
* @param i_key
*   key word to search
*
* @param i_value
*   value word
*/
void SmkyFilePairs::add (std::string i_key, std::string i_value)
{
    m_dictionary.insert( std::pair<std::string,std::string>(i_key, i_value) );
    m_changed = true;
}

/**
* remove pair by key
*
* @return bool
*   true if removed
*/
bool SmkyFilePairs::remove (std::string i_key)
{
    if (!m_dictionary.empty())
    {
        SmkyHashMap::iterator it;
        it = m_dictionary.find(i_key);

        if (it != m_dictionary.end())
        {
            m_dictionary.erase(it);
            m_changed = true;
            return(true);
        }
    }

    return(false);
}

/**
* is word exist in dictionary ?
*
* @param shortcut
*   key word to search
*
* @return std::string
*   word assotiated with key word
*/
std::string SmkyFilePairs::find (const std::string& shortcut)
{
    if (!m_dictionary.empty())
    {
        SmkyHashMap::iterator it;
        it = m_dictionary.find(shortcut);

        if (it != m_dictionary.end())
        {
            return(it->second);
        }
    }

    return "";
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
std::string SmkyFilePairs::find_by_prefix (const std::string& prefix)
{
    if (!m_dictionary.empty())
    {
        SmkyHashMap::iterator it;
        std::pair<std::string::const_iterator, std::string::const_iterator> res;

        for (it = m_dictionary.begin(); it != m_dictionary.end(); ++it )
        {
            res = std::mismatch(prefix.begin(), prefix.end(), it->second.begin());

            if (res.first == prefix.end())
            {
                return(it->second);
            }
        }
    }

    return "";
}

/**
* export dictionary elements to list
*
* @param entries
*   output: list of entries
*/
void SmkyFilePairs::exportToList (std::list<Entry>& entries)
{
    if (!m_dictionary.empty())
    {
        SmkyHashMap::iterator it;
        Entry entry;

        for (it = m_dictionary.begin(); it != m_dictionary.end(); ++it )
        {
            entry.shortcut = it->first;
            entry.substitution = it->second;
            entries.push_back(entry);
        }
    }
}


