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

#ifndef _AUTO_SUB_DATABASE_H
#define _AUTO_SUB_DATABASE_H

#include <glib.h>
#include <stdint.h>
#include "Database.h"
#include "SmkyFilePairs.h"
#include "SmkyKeywordsBundle.h"

namespace SmartKey
{

/**
 * Wraps the auto substitute database.
 */
class SmkyAutoSubDatabase
{
private:

    //auto replace - can be modified by user
    SmkyFilePairs m_autosub_dictionary;

    //auto replace hc
    SmkyFilePairs m_autosub_hc_dictionary;

public:
    SmkyAutoSubDatabase (void);
    virtual ~SmkyAutoSubDatabase (void);

    //is used default locale?
    bool isUsedDefault (void);

    //init
    SmartKeyErrorCode init (void);

    //learn word
    virtual void learnWord (const std::string& word);

    //forget word
    virtual bool forgetWord (const std::string& word);

    //add entry
    virtual SmartKeyErrorCode addEntry (const Entry& entry);

    //get entries
    virtual SmartKeyErrorCode getEntries (int offset, int limit, WhichEntries which, std::list<Entry>& entries);

    //get num entries
    virtual SmartKeyErrorCode getNumEntries (WhichEntries which, int& entries);

    //save
    virtual SmartKeyErrorCode save (void);

    //notify about locale change
    virtual void changedLocaleSettings (void);

    //find entry
    virtual std::string findEntry (const std::string& shortcut);

    //find word by prefix
    virtual std::string findWordByPrefix (const std::string& prefix);

    //get ldb substitution
    std::string getLdbSubstitution (std::string& shortcut);

    //get hardcoded substitution
    std::string getHardCodedSubstitution (std::string& shortcut);

private:

    //get path to dictionary
    string _getDbPath (void) const;

    //get path to hc-dictionary
    string _getHcDbPath (void) const;

    //test word
    static bool isWordAllUppercase (const uint16_t* word, uint16_t wordLen);

    //load
    SmartKeyErrorCode loadEntries (WhichEntries which, std::list<Entry>& entries);

};

/**
* get path to dictionary
*
* @return string
*   path
*/
inline std::string SmkyAutoSubDatabase::_getDbPath (void) const
{
    return(Settings::getInstance()->getDBFilePath(Settings::DICT_AUTOSUB));
}

/**
* get path to hc-dictionary
*
* @return string
*   path
*/
inline std::string SmkyAutoSubDatabase::_getHcDbPath (void) const
{
    return(Settings::getInstance()->getDBFilePath(Settings::DICT_AUTOSUB_HC));
}

/**
* add a new word
*
* @param word
*   word
*/
inline void SmkyAutoSubDatabase::learnWord (const std::string& word)
{
    g_debug("SmkyAutoSubDatabase::learnWord isn't supported");
}

/**
* remove word
*
* @param shortcut
*   word
*/
inline bool SmkyAutoSubDatabase::forgetWord (const std::string& shortcut)
{
    return(m_autosub_dictionary.remove(shortcut));
}

/**
* find entry pair
*
* @param shortcut
*   word to find
*
* @return std::string
*   pair word
*/
inline std::string SmkyAutoSubDatabase::findEntry (const std::string& shortcut)
{
    std::string retval = m_autosub_dictionary.find(shortcut);

    if( retval.length() == 0 )
        return(m_autosub_hc_dictionary.find(shortcut));

    return( retval );
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
inline std::string SmkyAutoSubDatabase::findWordByPrefix (const std::string& prefix)
{
    std::string retval = m_autosub_dictionary.find_by_prefix(prefix);
    if(retval.length() > 0)
        return( retval );

    return( m_autosub_hc_dictionary.find_by_prefix(prefix) );
}

/**
* add word
*
* @param entry
*   word to add
*
* @return SmartKeyErrorCode
*   return SKERR_SUCCESS always
*/
inline SmartKeyErrorCode SmkyAutoSubDatabase::addEntry (const Entry& entry)
{
    m_autosub_dictionary.add(entry.shortcut, entry.substitution);
    return SKERR_SUCCESS;
}

/**
* save
*
* @return SmartKeyErrorCode
*   SKERR_SUCCESS if done
*/
inline SmartKeyErrorCode SmkyAutoSubDatabase::save (void)
{
    return ( m_autosub_dictionary.save(Settings::getInstance()->getDBFilePath(Settings::DICT_AUTOSUB)) ? SKERR_SUCCESS : SKERR_FAILURE );
}


}

#endif

