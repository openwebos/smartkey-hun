/* @@@LICENSE
*
*      Copyright (c) 2013 Hewlett-Packard Development Company, L.P.
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

#ifndef DATABASE_H
#define DATABASE_H

#include <string>

namespace SmartKey
{

enum WhichEntries
{
    UserEntries,    ///< Entries added by the user.
    StockEntries,   ///< Stock entries.
    AllEntries      ///< All entries.
};

struct Entry
{
    std::string shortcut;
    std::string substitution;
};

enum SmartKeyErrorCode
{
    SKERR_SUCCESS = 0,          // No error
    SKERR_FAILURE = 1,          // General failure (try not to use)
    SKERR_DISABLED = 2,         // SmartKey service disabled.
    SKERR_NOMEMORY = 3,         // Insufficient memory
    SKERR_UNSUPPORTED = 4,      // Feature not supported
    SKERR_MISSING_PARAM = 5,    // Missing one or more parameters
    SKERR_BAD_PARAM = 6,        // Invalid parameter
    SKERR_WORD_EXISTS = 7,      // Word already exists
    SKERR_NO_MATCHING_WORDS = 8,// No matching words (like for delete, etc.)
};

/**
 * The data (mostly a dictionary) used by the SpellCheckEngine.
 */
class Database
{
public:
    /**
     * Add a word to this database.
     *
     * @return true if successfully added, false if not.
     */
    virtual void learnWord (const std::string& word) {}

    /**
     * Un-learn a word.
     *
     * @return true if successfully added, false if not.
     */
    virtual void forgetWord (const std::string& word) {}

    /**
     * Write the database to filesystem.
     */
    virtual SmartKeyErrorCode save (void)
    {
        return SKERR_UNSUPPORTED;
    }

    /**
     * Notification about locale settings were changed.
     */
    virtual void changedLocaleSettings (void) {}

    /**
     * Allow database to expect a particular number of entries, so that it can optimize its storage
     */
    virtual SmartKeyErrorCode setExpectedCount(int count)
    {
        return SKERR_UNSUPPORTED;
    }

protected:
    Database() {}
};


}

#endif
