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

#ifndef SMKY_KEYWORDS_BUNDLE_H
#define SMKY_KEYWORDS_BUNDLE_H

#include "Settings.h"
#include "SmkyFileKeywords.h"

namespace SmartKey
{
/**
 * wrapper around SmkyFileKeywords to support locale-independent and locale-dependent dictionaries,
 * idia is to combine locale independent dictionary with locale dependent using one interface.
 * m_independent_dict is for static words set and never changed.
 */
class SmkyKeywordsBundle
{
private:
    //locale-independent dictionary
    SmkyFileKeywords m_independent_dict;

    //locale-dependent dictionary
    SmkyFileKeywords m_dependent_dict;

public:

    virtual ~SmkyKeywordsBundle (void) {};

    //load dictionary according to current locale settings
    virtual void load (std::string i_independent_dict, std::string i_dependent_dict);

    //save dictionary
    virtual bool save (std::string i_dependent_dict);

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

    //export all strings to external list
    virtual void exportToList (std::list<std::string>& o_entries);

};

/**
* load bundle of strings
*/
inline void SmkyKeywordsBundle::load (std::string i_independent_dict, std::string i_dependent_dict)
{
    m_dependent_dict.load(i_dependent_dict);

    if(!m_independent_dict.isInitialized())
        m_independent_dict.load(i_independent_dict);
}

/**
* save bundle of strings
*/
inline bool SmkyKeywordsBundle::save (std::string i_dependent_dict)
{
    return( m_dependent_dict.save(i_dependent_dict) );
}

/**
* size: number of pairs
*/
inline int SmkyKeywordsBundle::size (void)
{
    return (m_independent_dict.size() + m_dependent_dict.size());
}

/**
* add a new string
*/
inline void SmkyKeywordsBundle::add (std::string i_key)
{
    m_dependent_dict.add(i_key);
}

/**
* remove string
*/
inline bool SmkyKeywordsBundle::remove (std::string i_key)
{
    return(m_dependent_dict.remove(i_key));
}

/**
* find string
*/
inline bool SmkyKeywordsBundle::find (const std::string& shortcut)
{
    return(m_independent_dict.find(shortcut) || m_dependent_dict.find(shortcut));
}

/**
* is word with specified prefix exist in bundle ?
*
* @param shortcut
*   key word to search
*
* @return std::string
*   word assotiated with prefix word
*/
inline std::string SmkyKeywordsBundle::find_by_prefix (const std::string& prefix)
{
    std::string retval = m_independent_dict.find_by_prefix(prefix);
    if(retval.length() > 0)
        return( retval );

    return( m_dependent_dict.find_by_prefix(prefix) );
}

/**
* export all strings to external list
*/
inline void SmkyKeywordsBundle::exportToList (std::list<std::string>& o_entries)
{
    m_independent_dict.exportToList(o_entries);
    m_dependent_dict.exportToList(o_entries);
}

}

#endif

