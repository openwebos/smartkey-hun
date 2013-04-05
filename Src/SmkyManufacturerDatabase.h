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

#ifndef SMKY_MAN_DATABASE_H
#define SMKY_MAN_DATABASE_H

#include "Database.h"
#include <string>
#include "StringUtils.h"
#include "SmkyKeywordsBundle.h"

namespace SmartKey
{

/**
 * Wraps an  Manufacturer database.
 */
class SmkyManufacturerDatabase
{
private:
    SmkyKeywordsBundle m_dictionary;

public:

    SmkyManufacturerDatabase (void);
    virtual ~SmkyManufacturerDatabase (void);

    //add word
    virtual void learnWord (const std::string& word);

    //remove word
    virtual bool forgetWord (const std::string& word);

    //find
    virtual bool findEntry (const std::string& word);

    //find word by prefix
    virtual std::string findWordByPrefix (const std::string& prefix);

    //save dictionary
    virtual SmartKeyErrorCode save (void);

    //notification about locale change
    virtual void changedLocaleSettings (void);

    //--
    virtual SmartKeyErrorCode setExpectedCount (int count); //?

private:

    //get path to locale independent dictionary
    std::string _getIndependDbPath (void) const;

    //get path to locale dependent dictionary
    std::string _getDependDbPath (void) const;

    //
    SmartKeyErrorCode _loadDefaultData (void);

    //
    SmartKeyErrorCode _loadWords (const char * path, std::list<std::string> & wordList);

};

/**
* get path to locale independent dictionary
*
* @return string
*   path
*/
inline std::string SmkyManufacturerDatabase::_getIndependDbPath (void) const
{
    return(Settings::getInstance()->getDBFilePath(Settings::DICT_MANUFACTURER, Settings::DICT_LOCALE_INDEPEND));
}

/**
* get path to locale dependent dictionary
*
* @return string
*   path
*/
inline std::string SmkyManufacturerDatabase::_getDependDbPath (void) const
{
    return(Settings::getInstance()->getDBFilePath(Settings::DICT_MANUFACTURER, Settings::DICT_LOCALE_DEPEND));
}

/**
 * find
 *
* @return bool
*   true if found
 */
inline bool SmkyManufacturerDatabase::findEntry (const std::string& word)
{
    return(m_dictionary.find(word));
}

/**
* See if word with prefix exists into database.
*
* @param word
*   word to find
*
* @return string
*   not empty if the word with prefix found into database.
*/
inline std::string SmkyManufacturerDatabase::findWordByPrefix (const std::string& prefix)
{
    return( m_dictionary.find_by_prefix(prefix) );
}

}

#endif

