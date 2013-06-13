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

#ifndef SMKY_FILEKEYWORDS_H
#define SMKY_FILEKEYWORDS_H

#include <ext/hash_set> //I know about replacement to <unordered_set>, but not sure yet about c++11 support for this project
#include <string>
#include <list>

namespace SmartKey
{
using namespace __gnu_cxx;

#define SmkyHashSet hash_set<std::string, SmkyHasher, SmkyComparator>

class SmkyHasher
{
public:
    size_t operator()(const std::string &r) const
    {
        return h(r.c_str());
    };

private:
    __gnu_cxx::hash<char*> h;
};

class SmkyComparator
{
public:
    bool operator()(const std::string& str1, const std::string& str2) const
    {
        return (str1.compare(str2) == 0);
    }
};

/**
 * Read/write dictionary with words stored into text file (one per line).
 */
class SmkyFileKeywords
{
private:
    //is engine was initialized successfuly ?
    bool m_initialized;
    bool m_changed;

    SmkyHashSet m_dictionary;

public:

    SmkyFileKeywords (void);
    virtual ~SmkyFileKeywords (void);

    // return m_initialized
    virtual bool isInitialized (void);

    //load dictionary according to current locale settings
    virtual bool load (std::string i_locale_path_file);

    //save dictionary
    virtual bool save (std::string i_db_file);

    //size
    virtual int size (void);

    //add pair
    virtual void add (std::string i_key);

    //remove pair by key
    virtual bool remove (std::string i_key);

    //find
    virtual bool find (const std::string& shortcut);

    //find by prefix
    virtual std::string find_by_prefix (const std::string& prefix);

    //export all strings from the dictionary to list
    virtual void exportToList (std::list<std::string>& o_entries);

protected:
    //release all allocated objects
    void _clean (void);

    //read pairs from text file and add them to m_dictionary
    void _importFileDB (std::string i_db_file);

};

/**
* return true if dictionary was already loaded
*/
inline bool SmkyFileKeywords::isInitialized (void)
{
    return(m_initialized);
}

/**
* size: number of pairs
*/
inline int SmkyFileKeywords::size (void)
{
    return m_dictionary.empty() ? 0 : m_dictionary.size();
}

}

#endif

