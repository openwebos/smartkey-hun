/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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
#include "SpellCheckEngine.h"

Settings::Settings()
    :backupDirectory("/etc/palm/smartkey")
	,locale("en_us")
	,readOnlyDataDir("/usr/palm/smartkey")
	,readWriteDataDir("/var/palm/data/smartkey")
	,smkyDataDirectory("/usr/palm/Smky/dbs")
{
}

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
    * open()
    * <here is function description>
    *
    * @param std::string& settingsFile
    *   <perameter description>
    *
    * @return bool
    *   <return value description>
    */
	bool open(const std::string& settingsFile)
	{
		keyfile = g_key_file_new();
		if(!keyfile)
			return false;
		GKeyFileFlags flags = GKeyFileFlags( G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS);
		
		GError* error(NULL);
		if( !g_key_file_load_from_file( keyfile, settingsFile.c_str(), flags, &error ) )
		{
			g_key_file_free( keyfile );
			if (error) {
				g_error_free(error);
			}
			return false;
		}

		return true;
	}

    /**
    * ReadString()
    * <here is function description>
    *
    * @param cat
    *   <perameter description>
    *
    * @param name
    *   <perameter description>
    *
    * @param var
    *   <perameter description>
    *
    * @return bool
    *   <return value description>
    */
	bool ReadString(const char* cat, const char* name, std::string& var)
	{
		GError* error(NULL);
		gchar* _vs=g_key_file_get_string(keyfile,cat,name,&error);
		if( !error && _vs ) {
			var = static_cast<const char*>(_vs);
			g_free(_vs);
			return true;
		}
		else {
			g_error_free(error);
			return false;
		}
	}

    /**
    * ReadBoolean()
    * <here is function description>
    *
    * @param cat
    *   <perameter description>
    *
    * @param name
    *   <perameter description>
    *
    * @param var
    *   <perameter description>
    *
    * @return bool
    *   <return value description>
    */
	bool ReadBoolean(const char* cat, const char* name, bool& var)
	{
		GError* error(NULL);
		gboolean _vb=g_key_file_get_boolean(keyfile,cat,name,&error);
		if( !error ) {
			var=_vb;
			return true;
		}
		else {
			g_error_free(error);
			return false;
		}
	}

    /**
    * ReadInteger()
    * <here is function description>
    *
    * @param cat
    *   <perameter description>
    *
    * @param name
    *   <perameter description>
    *
    * @param var
    *   <perameter description>
    *
    * @return bool
    *   <return value description>
    */
	bool ReadInteger(const char* cat, const char* name, int& var)
	{
		GError* error(NULL);
		int _v=g_key_file_get_integer(keyfile,cat,name,&error);
		if( !error ) {
			var=_v;
			return true;
		}
		else {
			g_error_free(error);
			return false;
		}
	}

    /**
    * ReadDouble()
    * <here is function description>
    *
    * @param cat
    *   <perameter description>
    *
    * @param name
    *   <perameter description>
    *
    * @param var
    *   <perameter description>
    *
    * @return bool
    *   <return value description>
    */
	bool ReadDouble(const char* cat, const char* name, double& var)
	{
		GError* error(NULL);
		double _v=g_key_file_get_double(keyfile,cat,name,&error);
		if( !error ) {
			var=_v;
			return true;
		}
		else {
			g_error_free(error);
			return false;
		}
	}

    /**
    * ReadFloat()
    * <here is function description>
    *
    * @param cat
    *   <perameter description>
    *
    * @param name
    *   <perameter description>
    *
    * @param var
    *   <perameter description>
    *
    * @return bool
    *   <return value description>
    */
	bool ReadFloat(const char* cat, const char* name,float& var)
	{
		GError* error(NULL);
		float _v=(float)g_key_file_get_double(keyfile,cat,name,&error);
		if( !error ) {
			var=_v;
			return true;
		}
		else {
			g_error_free(error);
			return false;
		}
	}
};


/**
* load()
* <here is function description>
*
* @param settingsFile
*   <perameter description>
*
* @return bool
*   <return value description>
*/
bool Settings::load(const std::string& settingsFile)
{
	KeyFileReader	reader;

	if (!reader.open(settingsFile))
		return false;

	reader.ReadString( "General", "backupDirectory", backupDirectory );
	reader.ReadString( "General", "Locale", locale );
	reader.ReadString( "General", "smkyDataDirectory", smkyDataDirectory );

	return true;
}

/**
* findLocalResource()
* <here is function description>
*
* @param pathPrefix
*   <perameter description>
*
* @param pathSuffix
*   <perameter description>
*
* @return std::string
*   <return value description>
*/
std::string SmartKey::LocaleSettings::findLocalResource(const std::string& pathPrefix, const char * pathSuffix) const
{
	// standard case: input language 'en', country 'us' -> 'en_us', or input language 'fr', country 'ca' -> 'fr_ca'
	std::string	path = pathPrefix + getLanguageCountryLocale() + pathSuffix;
	if (g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
		return path;

	// degenerate case: input language + country code "non-classic", try the country of language: input language 'fr', country 'us' -> 'fr_us' failed, so try 'fr_fr'
	// works with 'fr, 'de', 'it' and even 'es'...
	path = pathPrefix + m_inputLanguage + '_' + m_inputLanguage + pathSuffix;
	if (g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
		return path;

	// we're desperate. Try 'us' as a country...
	path = pathPrefix + m_inputLanguage + "_us" + pathSuffix;
	if (g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
		return path;

	// give up!
	return std::string();
}
