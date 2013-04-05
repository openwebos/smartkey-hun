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

#ifndef SMKY_FILEPAIRS_H
#define SMKY_FILEPAIRS_H

#include "Database.h"
#include <list>
#include <ext/hash_map> //I know about replacement to <unordered_map>, but not sure yet about c++11 support for this project

namespace __gnu_cxx
{
template<> struct hash<std::string>
{
    hash<char*> h;
    size_t operator()(const std::string &str) const
    {
        return h(str.c_str());
    };
};
}

namespace SmartKey
{
using namespace __gnu_cxx;

#define SmkyHashMap hash_map<std::string,std::string >

/**
 * Read/write dictionary with pairs of words stored into text file.
 */
class SmkyFilePairs
{
private:
    //is engine was initialized successfuly ?
    bool m_initialized;
    bool m_changed;

    SmkyHashMap m_dictionary;

public:

    SmkyFilePairs (void);
    virtual ~SmkyFilePairs (void);

    // return m_initialized
    virtual bool isInitialized (void);

    //load dictionary according to current locale settings
    virtual bool load (std::string i_locale_path_file);

    //save dictionary
    virtual bool save (std::string i_db_file);

    //size: number of pairs
    int size (void);

    //add pair
    virtual void add (std::string i_key, std::string i_value);

    //remove pair by key
    virtual bool remove (std::string i_key);

    //find
    virtual std::string find (const std::string& shortcut);

    //find by prefix
    virtual std::string find_by_prefix (const std::string& prefix);

    //export all pairs from the dictionary to list
    virtual void exportToList (std::list<Entry>& entries);

protected:
    //release all allocated objects
    void _clean (void);

    //read pairs from text file and add them to m_dictionary
    void _importFileDB (std::string i_db_file);

};

/**
* return true if dictionary was already loaded
*/
inline bool SmkyFilePairs::isInitialized (void)
{
    return(m_initialized);
}

/**
* size: number of pairs
*/
inline int SmkyFilePairs::size (void)
{
    return m_dictionary.empty() ? 0 : m_dictionary.size();
}

}

#endif

