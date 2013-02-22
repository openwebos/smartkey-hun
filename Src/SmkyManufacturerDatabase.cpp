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

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <glib.h>
#include <stdlib.h>
#include <algorithm>
#include "SmkyManufacturerDatabase.h"
#include "SmkyUserDatabase.h"
#include "Settings.h"
#include "StringUtils.h"

using namespace SmartKey;

/**
*
*/
SmkyManufacturerDatabase::SmkyManufacturerDatabase (void)
{
    //load database
    m_dictionary.load(_getIndependDbPath(), _getDependDbPath());
}

/**
* ~
*/
SmkyManufacturerDatabase::~SmkyManufacturerDatabase()
{
}

/**
* do nothing
*
* @param int count
*
* @return SmartKeyErrorCode
*/
SmartKeyErrorCode SmkyManufacturerDatabase::setExpectedCount (int count)
{
    return (SKERR_SUCCESS);
}

/**
* learn word
*
* @param std::string& word
*   word to add
*/
void SmkyManufacturerDatabase::learnWord (const std::string& word)
{
    if (g_utf8_validate(word.c_str(), -1, NULL))
        m_dictionary.add(word);
    else
        g_warning("SmkyManufacturerDatabase::learnWord: NOT learning invalid utf8 word '%s'", word.c_str());
}

/**
* forget word
*
* @param std::string& word
*
*/
bool SmkyManufacturerDatabase::forgetWord (const std::string& word)
{
    return(m_dictionary.remove(word));
}

/**
* save
*
* @return bool
*   true if saved
*/
SmartKeyErrorCode SmkyManufacturerDatabase::save (void)
{
    return( m_dictionary.save(_getDependDbPath()) ? SKERR_SUCCESS : SKERR_FAILURE );
}

/**
* notification about locale change
*/
void SmkyManufacturerDatabase::changedLocaleSettings (void)
{
    save();
    m_dictionary.load(_getIndependDbPath(), _getDependDbPath());
}


