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

#ifndef SMK_USER_DATABASE_H
#define SMK_USER_DATABASE_H

#include <stdint.h>
#include "Database.h"
#include "SmkyFileKeywords.h"
#include "Settings.h"

namespace SmartKey
{
/**
 * Wraps the reorder user database (RUDB).
 */
class SmkyUserDatabase
{
private:
    // collect words added by user
    SmkyFileKeywords m_user_database;

    // collect words added by system (ex: contact names)
    SmkyFileKeywords m_context_database;

public:
    SmkyUserDatabase (void);
    virtual ~SmkyUserDatabase (void);

    //init
    virtual void init (void);

    //learn word
    virtual void learnWord (const std::string& word);

    //learn context word
    virtual void learnContextWord (const std::string& word);

    //forget word
    virtual bool forgetWord (const std::string& word);

    //forget context word
    virtual bool forgetContextWord (const std::string& word);

    //get entries
    virtual SmartKeyErrorCode getEntries (int offset, int limit, std::list<std::string>& entries);

    //get number of the entries
    virtual void getNumEntries (int& o_entries);

    //find word
    virtual bool findWord (const std::string& word);

    //notification about locale settings change
    virtual void changedLocaleSettings (void);

    //save
    virtual SmartKeyErrorCode save (void);

    //not used at the moment
    virtual SmartKeyErrorCode updateWordUsage (const std::string& word);

private:


    //internal: get path to user dictionary
    std::string       _getDbPath (void) const;

    //internal: get path to context dictionary
    std::string       _getContextDbPath (void) const;

    //internal: get entries
    SmartKeyErrorCode _loadEntries (std::list<std::string>& o_entries);
};

/**
* get path to user dictionary
*
* @return string
*   path
*/
inline std::string SmkyUserDatabase::_getDbPath (void) const
{
    return(Settings::getInstance()->getDBFilePath(Settings::DICT_USER));
}

/**
* get path to context dictionary
*
* @return string
*   path
*/
inline std::string SmkyUserDatabase::_getContextDbPath (void) const
{
    return(Settings::getInstance()->getDBFilePath(Settings::DICT_USER_CONTEXT));
}

/**
* save both user and context dictionaries
*
* @return SmartKeyErrorCode
*   SKERR_SUCCESS if done
*/
inline SmartKeyErrorCode SmkyUserDatabase::save (void)
{
    return ( (m_user_database.save( _getDbPath() ) &&
              m_context_database.save( _getContextDbPath() )) ? SKERR_SUCCESS : SKERR_FAILURE);
}

/**
* See if a word exists in the user database.
*
* @param word
*   word to find
*
* @return SmartKeyErrorCode
*   true if the word is in the database. false if not (or on error).
*/
inline bool SmkyUserDatabase::findWord (const std::string& i_word)
{
    return ( m_user_database.find(i_word) || m_context_database.find(i_word));
}

/**
* learn user word
*
* @param word
*   word to add
*/
inline void SmkyUserDatabase::learnWord (const std::string& word)
{
    m_user_database.add(word);
}

/**
* learn context word
*
* @param word
*   word to add
*/
inline void SmkyUserDatabase::learnContextWord (const std::string& word)
{
    m_context_database.add(word);
}

/**
* forget user word
*
* @param word
*   word to forget
*
* @return SmartKeyErrorCode
*   SKERR_SUCCESS if done
*/
inline bool SmkyUserDatabase::forgetWord (const std::string& word)
{
    return(m_user_database.remove(word));
}

/**
* forget context word
*
* @param word
*   SKERR_SUCCESS if done
*/
inline bool SmkyUserDatabase::forgetContextWord (const std::string& word)
{
    return(m_context_database.remove(word));
}

/**
* get number of entries into user dictionary
*
* @param entries
*   number of entries
*/
inline void SmkyUserDatabase::getNumEntries (int& o_entries)
{
    o_entries = m_user_database.size();
}

}

#endif

