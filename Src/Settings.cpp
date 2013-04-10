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

#include <glib.h>
#include "Settings.h"
#include <algorithm>

using namespace SmartKey;

//=[KeyFileReader]======================================================================================================

/**
 * A simple wrapper around glib's settings.
 */
class KeyFileReader
{
    GKeyFile* keyfile;

public:

    KeyFileReader() : keyfile(NULL)
    {
    }

    ~KeyFileReader()
    {
        if (keyfile)
            g_key_file_free( keyfile );
    }

    /**
    * open the settings configuration file
    *
    * @param std::string& settingsFile
    *   file to open
    *
    * @return bool
    *   true if done
    */
    bool open (const std::string& settingsFile)
    {
        keyfile = g_key_file_new();
        if(!keyfile)
            return false;
        GKeyFileFlags flags = GKeyFileFlags( G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS);

        GError* error(NULL);
        if( !g_key_file_load_from_file( keyfile, settingsFile.c_str(), flags, &error ) )
        {
            g_key_file_free( keyfile );
            if (error)
            {
                g_error_free(error);
            }
            return false;
        }

        return true;
    }

    /**
    * read string value
    *
    * @param cat
    *   category name
    *
    * @param name
    *   parameter name
    *
    * @param var
    *   output: parameter's value
    *
    * @return bool
    *   true if done
    */
    bool ReadString (const char* cat, const char* name, std::string& var)
    {
        GError* error(NULL);
        gchar* _vs=g_key_file_get_string(keyfile,cat,name,&error);
        if( !error && _vs )
        {
            var = static_cast<const char*>(_vs);
            g_free(_vs);
            return true;
        }
        else
        {
            g_error_free(error);
            return false;
        }
    }

    /**
    * read boolean value
    *
    * @param cat
    *   category name
    *
    * @param name
    *   parameter name
    *
    * @param var
    *   output: parameter's value
    *
    * @return bool
    *   true if done
    */
    bool ReadBoolean (const char* cat, const char* name, bool& var)
    {
        GError* error(NULL);
        gboolean _vb=g_key_file_get_boolean(keyfile,cat,name,&error);
        if( !error )
        {
            var=_vb;
            return true;
        }
        else
        {
            g_error_free(error);
            return false;
        }
    }

    /**
    * read integer value
    *
    * @param cat
    *   category name
    *
    * @param name
    *   parameter name
    *
    * @param var
    *   output: parameter's value
    *
    * @return bool
    *   true if done
    */
    bool ReadInteger (const char* cat, const char* name, int& var)
    {
        GError* error(NULL);
        int _v=g_key_file_get_integer(keyfile,cat,name,&error);
        if( !error )
        {
            var=_v;
            return true;
        }
        else
        {
            g_error_free(error);
            return false;
        }
    }

    /**
    * read double value
    *
    * @param cat
    *   category name
    *
    * @param name
    *   parameter name
    *
    * @param var
    *   output: parameter's value
    *
    * @return bool
    *   true if done
    */
    bool ReadDouble (const char* cat, const char* name, double& var)
    {
        GError* error(NULL);
        double _v=g_key_file_get_double(keyfile,cat,name,&error);
        if( !error )
        {
            var=_v;
            return true;
        }
        else
        {
            g_error_free(error);
            return false;
        }
    }

    /**
    * read float value
    *
    * @param cat
    *   category name
    *
    * @param name
    *   parameter name
    *
    * @param var
    *   output: parameter's value
    *
    * @return bool
    *   true if done
    */
    bool ReadFloat (const char* cat, const char* name,float& var)
    {
        GError* error(NULL);
        float _v=(float)g_key_file_get_double(keyfile,cat,name,&error);
        if( !error )
        {
            var=_v;
            return true;
        }
        else
        {
            g_error_free(error);
            return false;
        }
    }
};

//=[DictionariesFileNames]==============================================================================================

/**
 * DictionariesFileNames
 */
DictionariesFileNames::DictionariesFileNames (void)
{
    m_whitelistdb_name = "whitelist-entries";
    m_localedb_name = "locale-words";
    m_autosubdb_name = "text-edit-autoreplace";
    m_mandb_name = "man-db-entries";
    m_userdb_name = "user-words";
    m_contextdb_name = "context-words";
}

//=[DictionariesRelativePaths]==========================================================================================

/**
 * DictionariesRelativePaths
 */
DictionariesRelativePaths::DictionariesRelativePaths (void)
{
    m_whitelist = "whitelist";
    m_locale = "locale";
    m_autosub = "autoreplace";
    m_autosub_hc = "autoreplace-hc";
    m_manufacturer = "manufacturer";
    m_user = "";
}

//=[Settings]===========================================================================================================

/**
 * Settings
 */
Settings::Settings (void)
    :readOnlyDataDir("/usr/palm/smartkey/DefaultData")
    ,readWriteDataDir("/var/palm/smartkey/DefaultData")
    ,hunspellDirectory("/usr/palm/smartkey/hunspell")
{
    localeSettings.m_inputLanguage = "en";
    localeSettings.m_deviceCountry = "us";
    localeSettings.m_inputLanguage = "en";
    localeSettings.m_keyboardLayout = "qwerty";
}

/**
* Find local resource: try to find existing path with specified prefix and suffix based on locale settings
*
* The sequence of finding a Hunspell-dictionary should be:
*   a) Check for the perfect fit, with language and country; example: "sv_FI" for Swedish (sv) language in Finland (FI)
*   b) If that fails, check for the language by itself; example "sv" for Swedish (sv) language
*   c) If that fails, check for the language with the country code from the language code; example "fr_FR" for French in France.
*   d) If that fails, check for the language with the US country code; example "es_US" for US Spanish
*
* For other dictionaries:
*   a) standard case: input language 'en', country 'us' -> 'en_us', or input language 'fr', country 'ca' -> 'fr_ca'
    b) degenerate case: input language + country code "non-classic", try the country of language: input language 'fr',
       country 'us' -> 'fr_us' failed, so try 'fr_fr' works with 'fr, 'de', 'it' and even 'es'...
*
* @param pathPrefix
*   path prefix
*
* @param pathSuffix
*   path suffix
*
* @return std::string
*   path is not empty if found
*/
std::string Settings::_findLocalResource (const std::string& pathPrefix, const char * pathSuffix) const
{
    //standard case
    std::string path = pathPrefix + localeSettings.m_inputLanguage + '_' + localeSettings.m_deviceCountry + pathSuffix;
    if (g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
        return path;

    //degenerate case
    path = pathPrefix + localeSettings.m_inputLanguage + '_' + localeSettings.m_inputLanguage + pathSuffix;
    if (g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
        return path;

    //check with capitalized country
    std::string capCountry = localeSettings.m_deviceCountry;
    std::transform(capCountry.begin(), capCountry.end(), capCountry.begin(), ::toupper);

    path = pathPrefix + localeSettings.m_inputLanguage + '_' + capCountry + pathSuffix;
    if (g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
        return path;

    //file name can be represented by two letters:
    path = pathPrefix + localeSettings.m_inputLanguage + pathSuffix;
    if (g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
        return path;

    //check for the language with the country code from the language code
    capCountry = localeSettings.m_inputLanguage;
    std::transform(capCountry.begin(), capCountry.end(), capCountry.begin(), ::toupper);

    path = pathPrefix + localeSettings.m_inputLanguage + '_' + capCountry + pathSuffix;
    if (g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
        return path;

    // we're desperate. Try 'us' as a country...
    path = pathPrefix + localeSettings.m_inputLanguage + "_us" + pathSuffix;
    if (g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
        return path;

    path = pathPrefix + localeSettings.m_inputLanguage + "_US" + pathSuffix;
    if (g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
        return path;

    // give up!
    return std::string();
}

/**
* get complete path + filename of requested db
*
* @param i_dictionary
*   Settings::DICTIONARY
*
* @param i_kind
*   Settings::DICT_KIND
*
* @return string
*   path + filename
*/
string Settings::getDBFilePath (DICTIONARY i_dictionary, DICT_KIND i_kind)
{
    string retval = "";
    string prefix;
    string suffix;

    switch (i_dictionary)
    {
    case (DICT_AUTOSUB) :
    {
        prefix = readWriteDataDir + "/" + directories.m_autosub + "/";
        suffix = "/" + fileNames.m_autosubdb_name;
        retval = _findLocalResource(prefix, suffix.c_str());
    }
    break;

    case (DICT_AUTOSUB_HC) :
    {
        prefix = readOnlyDataDir + "/" + directories.m_autosub_hc + "/";
        suffix = "/" + fileNames.m_autosubdb_name;
        retval = _findLocalResource(prefix, suffix.c_str());
    }
    break;

    case (DICT_LOCALE) :
    {
        if(i_kind == DICT_LOCALE_INDEPEND)
            retval = readOnlyDataDir + "/" + directories.m_locale + "/" + fileNames.m_localedb_name;
        else
        {
            prefix = readOnlyDataDir + "/" + directories.m_locale + "/";
            suffix = "/" + fileNames.m_localedb_name;
            retval = _findLocalResource(prefix, suffix.c_str());
        }
    }
    break;

    case (DICT_WHITE) :
    {
        if(i_kind == DICT_LOCALE_INDEPEND)
            retval = readOnlyDataDir + "/" + directories.m_whitelist + "/" + fileNames.m_whitelistdb_name;
        else
        {
            prefix = readOnlyDataDir + "/" + directories.m_whitelist + "/";
            suffix = "/" + fileNames.m_whitelistdb_name;
            retval = _findLocalResource(prefix, suffix.c_str());
        }
    }
    break;

    case (DICT_MANUFACTURER) :
    {
        if(i_kind == DICT_LOCALE_INDEPEND)
            retval = readOnlyDataDir + "/" + directories.m_manufacturer + "/" + fileNames.m_mandb_name;
        else
        {
            prefix = readOnlyDataDir + "/" + directories.m_manufacturer + "/";
            suffix = "/" + fileNames.m_mandb_name;
            retval = _findLocalResource(prefix, suffix.c_str());
        }
    }
    break;

    case (DICT_HUNSPELL) :
    {
        if (i_kind == DICT_HUNSPELL_AFF)
        {
            prefix = hunspellDirectory + "/";
            suffix = ".aff";
            retval = _findLocalResource(prefix, suffix.c_str());
        }

        if (i_kind == DICT_HUNSPELL_DIC)
        {
            prefix = hunspellDirectory + "/";
            suffix = ".dic";
            retval = _findLocalResource(prefix, suffix.c_str());
        }
    }
    break;

    case (DICT_USER) :
    {
        if (directories.m_user.length() > 0 )
            retval = readWriteDataDir + "/" + directories.m_user + "/" + fileNames.m_userdb_name;
        else
            retval = readWriteDataDir + "/" + fileNames.m_userdb_name;
    }
    break;

    case (DICT_USER_CONTEXT) :
    {
        if (directories.m_user.length() > 0 )
            retval = readWriteDataDir + "/" + directories.m_user + "/" + fileNames.m_contextdb_name;
        else
            retval = readWriteDataDir + "/" + fileNames.m_contextdb_name;

    }
    break;
    }

    return (retval);
}

/**
* load settings from the configuration file
*
* @param settingsFile
*   path + name of the configuration file
*
* @return bool
*   true if loaded without problem,
*   false otherwise
*/
bool Settings::load (const std::string& settingsFile)
{
    KeyFileReader reader;

    if (!reader.open(settingsFile))
        return false;

    Settings* p_settings = Settings::getInstance();

    reader.ReadString( "General", "hunspellDirectory", p_settings->hunspellDirectory );

    reader.ReadString( "General", "whitelistdbPath", p_settings->directories.m_whitelist );
    reader.ReadString( "General", "whitelistdbName", p_settings->fileNames.m_whitelistdb_name );

    reader.ReadString( "General", "localedbPath", p_settings->directories.m_locale );
    reader.ReadString( "General", "localedbName", p_settings->fileNames.m_localedb_name );

    reader.ReadString( "General", "autosubdbPath", p_settings->directories.m_autosub );
    reader.ReadString( "General", "autosubhcdbPath", p_settings->directories.m_autosub_hc );
    reader.ReadString( "General", "autosubdbName", p_settings->fileNames.m_autosubdb_name );

    reader.ReadString( "General", "mandbPath", p_settings->directories.m_manufacturer );
    reader.ReadString( "General", "mandbName", p_settings->fileNames.m_mandb_name );

    reader.ReadString( "General", "userdbPath", p_settings->directories.m_user );
    reader.ReadString( "General", "userdbName", p_settings->fileNames.m_userdb_name );
    reader.ReadString( "General", "contextdbName", p_settings->fileNames.m_contextdb_name );

    return true;
}

