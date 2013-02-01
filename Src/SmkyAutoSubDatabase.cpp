/* @@@LICENSE
*
*      Copyright (c) 2010-2012 Hewlett-Packard Development Company, L.P.
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
#include <glib/gstdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wctype.h>
#include <algorithm>
#include "SmkyAutoSubDatabase.h"
#include "SmkyUserDatabase.h"
#include "Settings.h"
#include "SmartKeyService.h"
#include "StringUtils.h"

#if 0 // Debug settings
#define DEBUG_STUB g_debug
#else
#define DEBUG_STUB(fmt, args...) (void)0
#endif

namespace SmartKey
{

// 10KB = approx 100 entries.
const size_t k_DatabaseSize = 30 * 1024;

/**
* SmkyAutoSubDatabase()
*
* @param SMKY_LINFO& lingInfo
*   <perameter description>
*
* @param const LocaleSettings& localeSettings
*   <perameter description>
*
* @param const Settings& settings
*   <perameter description>
*/
SmkyAutoSubDatabase::SmkyAutoSubDatabase(SMKY_LINFO& lingInfo, const LocaleSettings& localeSettings, const Settings& settings) :
	SmkyDatabase(lingInfo)
	, m_settings(settings)
	, m_pDatabaseData(NULL)
	, m_createdDatabase(false)
	, m_locale(localeSettings)
{
}

/**
* ~SmkyAutoSubDatabase()
*
*/
SmkyAutoSubDatabase::~SmkyAutoSubDatabase()
{
	SMKY_STATUS wStatus = saveDb();
	if (wStatus) {
		g_warning("ERROR %d saving user db.", wStatus);
	}

	if (NULL != m_pDatabaseData) free(m_pDatabaseData);
}

/**
* didCreateDatabase()
* <here is function description>
*
* @return bool
*       - TRUE  : if database was created
*       - FALSE : if not
*/
bool SmkyAutoSubDatabase::didCreateDatabase() const
{
	return m_createdDatabase;
}

/**
* getDbPath()
* <here is function description>
*
* @return std::string
*    <return value description>
*/
std::string SmkyAutoSubDatabase::getDbPath() const
{
	return m_settings.readWriteDataDir + "/smky/auto_sub_db_" + m_locale.getLanguageCountryLocale() + ".bin";
}

/**
* isWordAllUppercase()
* <here is function description>
*
* @param *word
*   <perameter description>
*
* @param wordLen
*   <perameter description>
*
* @return bool
*       - TRUE  : all letters are in upper case
*       - FALSE : if not
*/
bool SmkyAutoSubDatabase::isWordAllUppercase (const uint16_t* word, uint16_t wordLen)
{
	if (word == NULL)
		return false;

	for (uint16_t i = 0; i < wordLen; i++) {
		if (!iswupper(word[i]))
			return false;
	}

	return true;
}

/**
* matchGuessCaseToInputWord()
* Try to match the case of the guess to the input word. This way "cant" becomes "can't" and
* "Cant" becomes "Can't".
*
* @param inputWord
*   <perameter description>
*
* @param inputLen
*   <perameter description>
*
* @param guessLen
*   <perameter description>
*/
void SmkyAutoSubDatabase::matchGuessCaseToInputWord (const uint16_t* inputWord, uint16_t inputLen, uint16_t* guess, uint16_t guessLen)
{
	if (inputWord != NULL && guess != NULL && inputWord > 0 && iswupper(inputWord[0])) {

		// Default algorithm is to uppercase the first character of the guess if the first character 
		// of the input is uppercase.
		uint16_t numChars = 1;

		if (inputLen > 1 && isWordAllUppercase(inputWord, inputLen)) {
			// But if the input word is all uppercase then we will uppercase the entire guess.
			numChars = guessLen;
		}

		for (uint16_t i = 0; i < numChars; i++) {
			guess[i] = towupper(guess[i]);
		}
	}
}

/**
* findEntry()
* <here is function description>
*
* @param shortcut
*   <perameter description>
*
* @param shortcutLen
*   <perameter description>
*
* @param substitution
*   <perameter description>
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkyAutoSubDatabase::findEntry (const gunichar2* shortcut, uint16_t shortcutLen, std::string& substitution)
{

    // TODO:
    //  a)  Look short-cut up in m_lingInfo
    //  b)  If found, make sure case of substitution matches input using matchGuessCaseToInputWord()
    //  c)  If not found, check the hardcode entries list m_hardCodedEntries
    //  d)  If found in hard coded list make sure case of substitution matches input
    //  e)  Return true status

    return SMKY_STATUS_NONE;
}

/**
* findEntry()
* <here is function description>
*
* @param shortcut
*   <perameter description>
*
* @return std::string
*   <return value description>
*/
std::string SmkyAutoSubDatabase::findEntry (const std::string& shortcut)
{

    // TODO:
    //  a)  Wrapper around the other findEntry() method
    //  b)  If found, return the substitution
    //  c)  If not found, return empty string

	return "";
}

/**
* addEntry()
* <here is function description>
*
* @param entry
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyAutoSubDatabase::addEntry (const Entry& entry)
{

    // TODO:
    //  a)  Wrapper for other addEntry method
    //  b)  Return true status

    return SKERR_SUCCESS;
}

/**
* addEntry()
* <here is function description>
*
* @param shortcut
*   <perameter description>
*
* @param substitution
*   <perameter description>
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkyAutoSubDatabase::addEntry(const std::string& shortcut, const std::string& substitution)
{
    // TODO:
    //  a)  Add short-cut and substitution to m_lingInfo
    //  b)  Return true status

    return SMKY_STATUS_NONE;
}

/**
* compare_entries()
* Loads <b>all</b> user entries into the provided list - unsorted.
* 
* @param which 
*     defines type of entry:  UserEntries or StockEntries
*/
SMKY_STATUS SmkyAutoSubDatabase::loadEntries(WhichEntries which, std::list<Entry>& entries) const
{
    entries.clear();

    // TODO:
    //  a)  Read though m_lingInfo for type of entry
    //  b)  Each entry in list is a pair of shortcut and substitution
    //  c)  Write to entries list
    //  d)  Return true status

    return SMKY_STATUS_NONE;
}

/**
* compare_entries()
* <here is function description>
*
* @param first
*   <perameter description>
*
* @param second
*   <perameter description>
*
* @return bool
    TRUE :
    FALSE:
*/
static bool compare_entries (const SmkyAutoSubDatabase::Entry& first, const SmkyAutoSubDatabase::Entry& second)
{
	return StringUtils::compareStrings(first.shortcut, second.shortcut);
}

/**
 * Get all user entries
 */
SmartKeyErrorCode SmkyAutoSubDatabase::getEntries(int offset, int limit, WhichEntries which, std::list<Entry>& entries)
{
	if (offset < 0 || limit < 0)
		return SKERR_BAD_PARAM;

	entries.clear();

	std::list<Entry> allEntries;
	SMKY_STATUS wStatus = loadEntries(which, allEntries);
	if (wStatus == SMKY_STATUS_NONE) {
		allEntries.sort(compare_entries);
		std::list<Entry>::const_iterator i = allEntries.begin();
		// Skip up to the offset
		while (i != allEntries.end() && offset-- > 0) {
			++i;
		}

		// Now read the entries
		while (i != allEntries.end() && limit-- > 0) {
			entries.push_back(*i++);
		}
	}

	return SmkyUserDatabase::smkyErrorToSmartKeyError(wStatus);
}

/**
* getNumEntries()
* Return the number of user entries
*
* 
* @param which 
*     defines type of entry:  UserEntries or StockEntries
*
* @param entries
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyAutoSubDatabase::getNumEntries (WhichEntries which, int& entries)
{
	entries = 0;

    // TODO:
    //  a)  Get the number of entries of the specified type from m_lingInfo
    //  b)  Return true status

    return SmkyUserDatabase::smkyErrorToSmartKeyError(SMKY_STATUS_NONE);
}

/**
* duplicateShortEntries()
* <here is function description>
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkyAutoSubDatabase::duplicateShortEntries()
{

    // TODO:
    //  a)  This may not been needed with HunSpell, so perhaps no action

    return SMKY_STATUS_NONE;
}

/**
* loadDefaultData()
* <here is function description>
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkyAutoSubDatabase::loadDefaultData()
{
	SMKY_STATUS wStatus = SMKY_STATUS_NONE;

	int count = 0;
	std::string fname =  m_locale.findLocalResource(m_settings.readOnlyDataDir + "/smky/DefaultData/autoreplace/", "/text-edit-autoreplace");
	FILE* f = fopen(fname.c_str(),"rb");
	if (f) {
		char linebuffer[128];
		while (fgets(linebuffer, sizeof(linebuffer), f)) {
			char* shortcut = strtok(linebuffer,"|\x0d\x0a" );
			char* replacement = strtok(0,"\x0d\x0a" );
			if (shortcut != NULL && replacement != NULL && strlen(shortcut) > 0 && strlen(replacement) > 0) {
				SMKY_STATUS s = addEntry(shortcut, replacement);
				if (wStatus == SMKY_STATUS_NONE)
					wStatus = s, ++count;
			}
		}
	}
	else {
		wStatus = SMKY_STATUS_READ_DB_FAIL;
	}

	if (wStatus == SMKY_STATUS_NONE) {
		wStatus = duplicateShortEntries();
	}
	
	g_debug("Loaded %d auto-replace entries for locale '%s' from '%s'. Result: %u", count, m_locale.getFullLocale().c_str(), fname.c_str(), wStatus);

	return wStatus;
}

/**
* loadHardCodedEntries()
* <here is function description>
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkyAutoSubDatabase::loadHardCodedEntries()
{
	std::string fname =  m_locale.findLocalResource(m_settings.readOnlyDataDir + "/smky/DefaultData/autoreplace-hc/", "/text-edit-autoreplace");

	SMKY_STATUS wStatus;
	FILE* f = fopen(fname.c_str(),"rb");
	if (f) {
		wStatus = SMKY_STATUS_NONE;
		char linebuffer[128];
		while (fgets(linebuffer, sizeof(linebuffer), f)) {
			const char* shortcut = strtok(linebuffer,"|\x0d\x0a" );
			const char* replacement = strtok(0,"\x0d\x0a" );
			if (shortcut != NULL && replacement != NULL && strlen(shortcut) > 0 && strlen(replacement) > 0) {
				m_hardCodedEntries[shortcut] = replacement;
			}
		}

		g_debug("Loaded %u hard-code AR entries from '%s'", m_hardCodedEntries.size(), fname.c_str());
	}
	else {
		wStatus = SMKY_STATUS_READ_DB_FAIL;
	}

	return wStatus;
}

/**
* init()
* <here is function description>
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkyAutoSubDatabase::init()
{
    g_assert(m_pDatabaseData == NULL);
	m_pDatabaseData = static_cast<uint8_t*>(calloc(k_DatabaseSize, sizeof(uint8_t)));
	if (NULL == m_pDatabaseData)
		return SMKY_STATUS_NO_MEMORY;

	m_createdDatabase = true;
	std::string path = getDbPath();
	if (g_file_test(path.c_str(), G_FILE_TEST_EXISTS)) {
		size_t size = SmartKeyService::fileSize(path);
		if (size != InvalidFileSize) {
			if (size <= k_DatabaseSize) {
				int fd = open(path.c_str(), O_RDONLY);
				if (-1 != fd) {
					ssize_t bytesRead = read(fd, m_pDatabaseData, size);
					close(fd);
					if (static_cast<size_t>(bytesRead) != size) {
						memset(m_pDatabaseData, 0, k_DatabaseSize);
					}
					else {
						m_createdDatabase = false;
					}
				}
			}
		}
	}

    //
    // TODO: Initialize m_lingInfo from m_pDatabaseData
    //

	if (m_createdDatabase) {
		g_message("First time creating auto-sub database for locale %s. Loading default data...", m_locale.getFullLocale().c_str());
		loadDefaultData();
	}
	else {
		g_debug("Just loaded auto-sub database for locale %s.", m_locale.getFullLocale().c_str());
	}

	if (wStatus == SMKY_STATUS_NONE)
		cacheLdbEntries();

	if (wStatus == SMKY_STATUS_NONE)
		loadHardCodedEntries();

   return wStatus;
}

/**
* save()
* <here is function description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyAutoSubDatabase::save()
{
	return SmkyUserDatabase::smkyErrorToSmartKeyError(saveDb());
}

/**
* saveDb()
* <here is function description>
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkyAutoSubDatabase::saveDb()
{
	if (m_pDatabaseData == NULL)
		return SMKY_STATUS_INVALID_MEMORY;

	double start = SmartKeyService::getTime();

	std::string path = getDbPath();
	std::string tmpPath = path + ".tmp";

	std::string parentDir = SmartKeyService::dirName(tmpPath);
	if (!parentDir.empty()) {
		::g_mkdir_with_parents(parentDir.c_str(), S_IRWXU );
	}
	if (g_file_test(tmpPath.c_str(), G_FILE_TEST_EXISTS))
		g_unlink(tmpPath.c_str());

	SMKY_STATUS wStatus = SMKY_STATUS_WRITE_DB_FAIL;
	int fd = open(tmpPath.c_str(), O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP);
	if (-1 != fd) {
		ssize_t written = write(fd, m_pDatabaseData, k_DatabaseSize);
		fsync(fd);
		close(fd);
		if (size_t(written) == k_DatabaseSize) {
			if (0 == g_rename(tmpPath.c_str(), path.c_str())) {
				wStatus = SMKY_STATUS_NONE;
				double now = SmartKeyService::getTime();
				g_debug("Auto-sub database saved in %g msec.  %u", (now-start) * 1000.0, wStatus);
			}
			else {
				g_warning("Error writing auto-sub db");
				g_unlink(tmpPath.c_str());
			}
		}
	}

	return wStatus;
}

/**
* forgetWord()
* <here is function description>
*
* @param shortcut
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyAutoSubDatabase::forgetWord(const std::string& shortcut)
{
	SMKY_STATUS wStatus;
	
    // TODO:
    //  a)  Delete shortcut from m_lingInfo
    //  b)  Return true status
	
	wStatus = SMKY_STATUS_NO_MEMORY;

	return SmkyUserDatabase::smkyErrorToSmartKeyError(wStatus);
}

/**
* setLocaleSettings()
* <here is function description>
*
* @param localeSettings
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyAutoSubDatabase::setLocaleSettings(const LocaleSettings& localeSettings)
{
	// We don't do anything because when the locale changes the engine destroys and
	// recreates me with a new locale.
	return SKERR_SUCCESS;
}

/**
* cacheLdbEntries()
* <here is function description>
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkyAutoSubDatabase::cacheLdbEntries()
{
	m_ldbEntries.clear();

	std::list<Entry> entries;
	SMKY_STATUS wStatus = loadEntries(StockEntries, entries);
	if (wStatus == SMKY_STATUS_NONE) {
		std::list<Entry>::const_iterator entry;
		for (entry = entries.begin(); entry != entries.end(); ++entry) {
			std::string lcshortut = StringUtils::utf8tolower(entry->shortcut);
			m_ldbEntries[lcshortut] = entry->substitution;
		}
	}

	g_debug("Loaded %u LDB entries", m_ldbEntries.size());
	return wStatus;
}

/**
* getLdbSubstitution()
* Find the substitution (if there is one) for the given shortcut.
*
* @param shortcut
*   <perameter description>
*
* @return std::string
*   The substitution mapped by the shortcut or an empty string if no mapping.
*/
std::string SmkyAutoSubDatabase::getLdbSubstitution(const std::string& shortcut) const
{
	std::string lcshortut = StringUtils::utf8tolower(shortcut);

	std::map<std::string,std::string>::const_iterator entry = m_ldbEntries.find(lcshortut);
	if (entry == m_ldbEntries.end())
		return "";
	else
		return entry->second;
}

/**
* getHardCodedSubstitution()
* Find the substitution (if there is one) for the given shortcut.
*
* @param shortcut
*   <perameter description>
*
* @return std::string
*   The substitution mapped by the shortcut or an empty string if no mapping.
*/
std::string SmkyAutoSubDatabase::getHardCodedSubstitution(const std::string& shortcut) const
{
	std::string lcshortut = StringUtils::utf8tolower(shortcut);

	std::map<std::string,std::string>::const_iterator entry = m_hardCodedEntries.find(lcshortut);
	if (entry == m_hardCodedEntries.end())
		return "";
	else
		return entry->second;
}

}

