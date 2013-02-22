/* @@@LICENSE
*
*      Copyright (c) 2009-2013 Hewlett-Packard Development Company, L.P.
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

#ifndef SMKY_SETTINGS_H
#define SMKY_SETTINGS_H

#include <glib.h>
#include <string>

namespace SmartKey
{
using namespace std;

/**
 * Store information about current locale settings used
 */
struct LocaleSettings
{
    //has virtual keyboard? (value came from the preferences change)
    bool   m_hasVirtualKeyboard;

    //device language
    string m_deviceLanguage;

    //device country
    string m_deviceCountry;

    //input language
    string m_inputLanguage;

    //keyboard layout
    string m_keyboardLayout;

    string getLanguageCountryLocale() const
    {
        return m_inputLanguage + '_' + m_deviceCountry;
    }

    string getFullLocale() const
    {
        return m_deviceLanguage + '_' + m_deviceCountry + ", " + m_keyboardLayout + '-' + m_inputLanguage;
    }
};

/**
 * Store dictionaries file names
 */
struct DictionariesFileNames
{
    //whitelist file name
    string m_whitelistdb_name;

    //localedb file name
    string m_localedb_name;

    //auto substitution db file name
    string m_autosubdb_name;

    //manufacturer db file name
    string m_mandb_name;

    //user db file name
    string m_userdb_name;

    //user db (context) file name
    string m_contextdb_name;

    DictionariesFileNames (void);
};

/**
 * Store path to dictionaries (starting from the default folder)
 */
struct DictionariesRelativePaths
{
    //relative path to whitelist
    string m_whitelist;

    //relative path to locale specific dictionaries
    string m_locale;

    //relative path to autosubstitution dictionary
    string m_autosub;

    //relative path to autosubstitution dictionary (hardcoded values)
    string m_autosub_hc;

    //relative path to manufacturer dictionary
    string m_manufacturer;

    //relative path to user dictionaries (user and context)
    string m_user;

    DictionariesRelativePaths (void);
};

/**
 * All the settings
 */
class Settings
{
public:
    enum DICTIONARY
    {
        DICT_AUTOSUB = 0
        ,DICT_AUTOSUB_HC
        ,DICT_LOCALE
        ,DICT_WHITE
        ,DICT_MANUFACTURER
        ,DICT_HUNSPELL
        ,DICT_USER
        ,DICT_USER_CONTEXT
    };

    enum DICT_KIND
    {
        DICT_LOCALE_INDEPEND = 0
        ,DICT_LOCALE_DEPEND
        ,DICT_HUNSPELL_AFF
        ,DICT_HUNSPELL_DIC
    };

public:

    //path to service data root folder (read only access allowed)
    string readOnlyDataDir;

    //path to service data root folder (read/write access allowed)
    string readWriteDataDir;

    //path to hunspell dictionaries
    string hunspellDirectory;

    //locale settings
    LocaleSettings localeSettings;

    //relative path to dictionaries starting from service data root folder (readOnlyDataDir or readWriteDataDir)
    DictionariesRelativePaths directories;

    //all our dictionaries file names
    DictionariesFileNames fileNames;

public:
    static Settings* getInstance(void)
    {
        static Settings m_instance;
        return(&m_instance);
    };

    //load settings from the configuration file
    bool load (const std::string& settingsFile);

    //get complete path + filename of requested db
    string getDBFilePath  (DICTIONARY i_dictionary, DICT_KIND i_kind = DICT_LOCALE_INDEPEND);

private:

    Settings (void);
    Settings (Settings const&);       // don't implement
    void operator= (Settings const&); // don't implement
    void operator& (Settings const&); // don't implement

    //find locale resource
    std::string _findLocalResource (const std::string& pathPrefix, const char * pathSuffix) const;

};

}  // namespace SmartKey

#endif  // SMKY_SETTINGS_H
