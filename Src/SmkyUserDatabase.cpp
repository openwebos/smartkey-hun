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
#include "SmkyUserDatabase.h"
#include "Settings.h"
#include "SmartKeyService.h"

namespace SmartKey
{

// By default words are 64 characters.
const size_t k_DatabaseSize = 50 * 1024; // 50KB / 64 = 800 words

/**
* SmkyUserDatabase()
* <here is function description>
*
* @param lingInfo
*   <perameter description>
*
* @param settings
*   <perameter description>
*/
SmkyUserDatabase::SmkyUserDatabase(SMKY_LINFO& lingInfo, const Settings& settings) :
	SmkyDatabase(lingInfo)
	, m_settings(settings)
	, m_pDatabaseData(NULL)
{
}

/**
* ~SmkyUserDatabase()
* <here is function description>
*/
SmkyUserDatabase::~SmkyUserDatabase()
{
	SMKY_STATUS wStatus = saveDb();
	if (wStatus) {
		g_warning("ERROR %d saving user db.", wStatus);
	}

	if (NULL != m_pDatabaseData) free(m_pDatabaseData);
}

/**
* getDbPath()
* <here is function description>
*
* @return std::string
*   <return value description>
*/
std::string SmkyUserDatabase::getDbPath() const
{
	return m_settings.readWriteDataDir + "/smky/user_db.bin";
}

/**
* init()
* <here is function description>
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkyUserDatabase::init()
{
	g_assert(m_pDatabaseData == NULL);
	m_pDatabaseData = static_cast<uint8_t*>(calloc(k_DatabaseSize, sizeof(uint8_t)));
	if (NULL == m_pDatabaseData)
		return SMKY_STATUS_NO_MEMORY;

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
				}
			}
		}
	}

    // TODO:  initialize m_lingInfo from m_pDatabaseData
	
	return SKERR_SUCCESS;
}

/**
* save()
* <here is function description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyUserDatabase::save()
{
	return smkyErrorToSmartKeyError(saveDb());
}

/**
* saveDb()
* Save the user database out to the filesystem.
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkyUserDatabase::saveDb()
{
	g_message("Saving user database");
	double start = SmartKeyService::getTime();
	if (m_pDatabaseData == NULL)
		return SMKY_STATUS_INVALID_MEMORY;

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
				g_debug("User database saved in %g msec.  %u", (now-start) * 1000.0, wStatus);
			}
			else {
				g_warning("Error writing user db");
				g_unlink(tmpPath.c_str());
			}
		}
	}

	return wStatus;
}

/**
* findWord()
* See if a word exists in the user database.
*
* @param word
*   <perameter description>
*
* @return SmartKeyErrorCode
*   true if the word is in the database. false if not (or on error).
*/
SmartKeyErrorCode SmkyUserDatabase::findWord(const std::string& word) const
{
	if (word.empty())
		return SKERR_BAD_PARAM;

    // TODO:  Look word up in m_lingInfo
	return smkyErrorToSmartKeyError(SMKY_STATUS_NO_MATCHING_WORDS);
}

/**
* learnWord()
* <here is function description>
*
* @param word
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyUserDatabase::learnWord(const std::string& word)
{
	if (word.empty())
		return SKERR_BAD_PARAM;

    // TODO:
    //  a)  Look word up in m_lingInfo, return WORD_EXISTS error code if found
    //  b)  If word doesn't exist, add it to m_lingInfo
    //  c)  Return true status

	return SKERR_SUCCESS;
}

/**
* forgetWord()
* <here is function description>
*
* @param word
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyUserDatabase::forgetWord(const std::string& word)
{
	if (word.empty())
		return SKERR_BAD_PARAM;

    // TODO:
    //  a)  Delete word from m_lingInfo
    //  b)  Return true status


	return SKERR_SUCCESS;
}

/**
* loadEntries()
* Loads <b>all</b> user entries into the provided list - unsorted.
*
* @param entries
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyUserDatabase::loadEntries(std::list<std::string>& entries) const
{
	entries.clear();

    // TODO:
    //  a)  Read all words from m_lingInfo and add each to the entries list
    //  b)  Return true status


	return SKERR_SUCCESS;
}

/**
* getEntries()
* <here is function description>
*
* @param offset
*   <perameter description>
*
* @param limit
*   <perameter description>
*
* @param entries
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyUserDatabase::getEntries(int offset, int limit, std::list<std::string>& entries)
{
	if (offset < 0 || limit < 0)
		return SKERR_BAD_PARAM;

	entries.clear();

	// Inefficient to load all entries into RAM, but we need to return a sorted list
	std::list<std::string> allEntries;
	SmartKeyErrorCode err = loadEntries(allEntries);
	if (err == SKERR_SUCCESS) {

		allEntries.sort(StringUtils::compareStrings);
		std::list<std::string>::const_iterator i = allEntries.begin();
		// Skip up to the offset
		while (i != allEntries.end() && offset-- > 0) {
			++i;
		}

		// Now read the entries
		while (i != allEntries.end() && limit-- > 0) {
			entries.push_back(*i++);
		}
	}

	return err;
}

/**
* getNumEntries()
* <here is function description>
*
* @param entries
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyUserDatabase::getNumEntries (int& entries)
{
	entries = 0;

    // TODO:
    //  a)  Get a count of words from m_lingInfo
    //  b)  Return true status

	return SKERR_SUCCESS;
}

/**
* smkyErrorToSmartKeyError()
* <here is function description>
*
* @param err
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyUserDatabase::smkyErrorToSmartKeyError(SMKY_STATUS err)
{
	switch (err) {
		case SMKY_STATUS_NONE: return SKERR_SUCCESS;
		case SMKY_STATUS_NO_MEMORY: return SKERR_NOMEMORY;
		case SMKY_STATUS_BAD_PARAM: return SKERR_BAD_PARAM;
		case SMKY_STATUS_WORD_EXISTS: return SKERR_WORD_EXISTS;
		case SMKY_STATUS_NO_MATCHING_WORDS: return SKERR_NO_MATCHING_WORDS;
		default: return SKERR_FAILURE;
	}
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
SmartKeyErrorCode SmkyUserDatabase::setLocaleSettings(const LocaleSettings& localeSettings)
{
	// We only have one user database for all locales.
	return SKERR_SUCCESS;
}

/**
* updateWordUsage()
* <here is function description>
*
* @param word
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyUserDatabase::updateWordUsage (const std::string& word)
{
	if (word.empty())
		return SKERR_BAD_PARAM;

    // TODO:
    //  a)  Update m_lingInfo with usage of this word
    //  b)  Note:  This only necessary if the dictionary system, collects statistics on word usage
    //  c)  Return true status


	return SKERR_SUCCESS;
}

}

