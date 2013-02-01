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

namespace SmartKey
{

static SmkyManufacturerDatabase* g_ManDb;

const int cMinimumHunspellEntryCount = 1000;

/**
* SmkyManufacturerDatabase()
*
* @param SMKY_LINFO& lingInfo
*   <perameter description>
*
* @param Settings& settings
*   <perameter description>
*
* @return
*   <return value description>
*/
SmkyManufacturerDatabase::SmkyManufacturerDatabase(SMKY_LINFO& lingInfo, const Settings& settings) :
	SmkyDatabase(lingInfo)
	, m_settings(settings)
	, m_hunspell(NULL)
	, m_hunspellSize(0)
	, m_expectedContactCount(500)
	, m_lastHunspellResult(NULL)
	, m_lastHunspellResultCount(0)
{
	g_ManDb = this;

	std::string fname =  m_settings.readOnlyDataDir + "/smky/DefaultData/manufacturer/man-db-entries";
	loadWords(fname.c_str(), m_universalWords);

	g_debug("Loaded %u universal words.", m_universalWords.size());

	// TODO:  Register manufacturer database callback with m_lingInfo:  ManufacturerDbCallback
}

/**
* ~SmkyManufacturerDatabase()
* <here is function description>
*
*/
SmkyManufacturerDatabase::~SmkyManufacturerDatabase()
{
	g_ManDb = NULL;
	deleteHunspellDictionary();
}

/**
* setExpectedCount()
* <here is function description>
*
* @param int count
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyManufacturerDatabase::setExpectedCount(int count)
{
	if (count > m_expectedContactCount)
		m_expectedContactCount = count;	// don't shrink minimum number we want to support...
	createHunspellDictionary();
	return SKERR_SUCCESS;
}

/**
* createHunspellDictionary()
* <here is function description>
*
*/
void SmkyManufacturerDatabase::createHunspellDictionary()
{
	if (!m_hunspell)
	{
		m_hunspellSize = m_expectedContactCount * 17 / 10 + 100;	// with duplicate names, one contact means 1.7 words, on average...
		if (m_hunspellSize < cMinimumHunspellEntryCount)
			m_hunspellSize = cMinimumHunspellEntryCount;
		if (m_hunspellSize < (int) m_words.size())
			m_hunspellSize = m_words.size();

		const char * emptyDicPath = "/tmp/emptyDic.dic";
		FILE * file = ::fopen(emptyDicPath, "w");
		if (file)
		{
			::fprintf(file, "%d\n", m_hunspellSize);
			::fclose(file);
		}
		if (!g_file_test(emptyDicPath, G_FILE_TEST_IS_REGULAR)) {
			g_warning("SmkyManufacturerDatabase::createHunspellDictionary: could not create empty dictionary.");
			return;
		}

		std::string fname = m_settings.readOnlyDataDir + "/hunspell/basicGrammar.aff";
		if (!g_file_test(fname.c_str(), G_FILE_TEST_IS_REGULAR)) {
			g_warning("SmkyManufacturerDatabase::createHunspellDictionary: basic grammar file not found at %s.", fname.c_str());
			return;
		}

		m_hunspell = new Hunspell(fname.c_str(), emptyDicPath);
//		std::string	test = "Marlène";
//		g_debug("before: %s", test.c_str());
//		m_hunspell->add(test.c_str());
//		g_debug("after: %s", test.c_str());
//		m_hunspell->add("Sigolène");

		::unlink(emptyDicPath);

		// populate the Hunspell dictionary, if necessary
		for (std::map<std::string, int>::const_iterator iter = m_words.begin(); iter != m_words.end(); ++iter)
		{
			const std::string & word = iter->first;
			m_hunspell->add(word.c_str());
		}

		g_debug("SmkyManufacturerDatabase::createHunspellDictionary: created Hunspell dictionary with %u words in it, capacity of %d entries.", m_words.size(), m_hunspellSize);
	}
}

/**
* deleteHunspellDictionary()
* <here is function description>
*
*/
void SmkyManufacturerDatabase::deleteHunspellDictionary()
{
	if (m_hunspell)
	{
		if (m_lastHunspellResult)
		{
			m_hunspell->free_list(&m_lastHunspellResult, m_lastHunspellResultCount);
			m_lastHunspellResult = NULL;
		}
		delete m_hunspell;
		m_hunspell = NULL;
	}
}

/**
* learnWord()
* <here is function description>
*
* @param std::string& word
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyManufacturerDatabase::learnWord(const std::string& word)
{
	//g_debug("Learning \"%s\"", word.c_str());

	if (!g_utf8_validate(word.c_str(), -1, NULL)) {
		g_warning("SmkyManufacturerDatabase::learnWord: NOT learning invalid utf8 word '%s'", word.c_str());
		return SKERR_FAILURE;
	}

	std::map<std::string, int>::iterator i = m_words.find(word);

	if (i == m_words.end()) {	// learn a new word
		m_lastFirstLastLetterResults.clear();
		m_words[word] = 1;
		//g_debug("learning '%s', %uth word.", word.c_str(), m_words.size());
		if (m_words.size() % 1000 == 0)
			g_debug("SmkyManufacturerDatabase::learnWord: %uth word...", m_words.size());
		if (m_hunspell && (int) m_words.size() > 5 * m_hunspellSize)	// we've blown out the capacity of Hunspell! We will recreate it & refeed all the words...
			deleteHunspellDictionary();
		if (m_hunspell)
			m_hunspell->add(word.c_str());
	}
	else {
		i->second++;
	}

	return SKERR_SUCCESS;
}

/**
* forgetWord()
* <here is function description>
*
* @param std::string& word
*   <perameter description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyManufacturerDatabase::forgetWord(const std::string& word)
{
	std::map<std::string, int>::iterator i = m_words.find(word);

	if (i == m_words.end()) {
		return SKERR_NO_MATCHING_WORDS;
	}
	else {
		if (i->second == 1) {
			m_lastFirstLastLetterResults.clear();
			m_words.erase(i);
			if (m_hunspell)
				m_hunspell->remove(word.c_str());
		}
		else {
			g_assert(i->second >= 1);
			i->second--;
		}
	}

	return SKERR_SUCCESS;
}

/**
* save()
* <here is function description>
*
* @return SmartKeyErrorCode
*   <return value description>
*/
SmartKeyErrorCode SmkyManufacturerDatabase::save()
{
	return SKERR_SUCCESS;
}

#ifdef TARGET_DESKTOP
#define DO_DEBUG_CALLBACK 1
#else
#define DO_DEBUG_CALLBACK 0
#endif

#if DO_DEBUG_CALLBACK
#define DEBUG_CALLBACK g_debug
#else
#define DEBUG_CALLBACK(fmt, args...) (void)0
#endif

/**
* setQuery()
* <here is function description>
*
* @param std::string &query
*   <perameter description>
*
*/
void SmkyManufacturerDatabase::setQuery(const std::string &query)
{
	m_lastFirstLastLetterResults.clear();

	if (!m_hunspell)
		createHunspellDictionary();

	if (!m_hunspell)
		return;

	if (query != m_lastHunspellQuery)
	{
		//PerfMonitor perf("m_hunspell->suggest");
		if (m_lastHunspellResult)
		{
			m_hunspell->free_list(&m_lastHunspellResult, m_lastHunspellResultCount);
			m_lastHunspellResult = NULL;
		}
//		size_t	length = query.length();
//		if (length > 4 && query[1] == '\'' && ::isalpha((unsigned char) query[0]))
//			m_lastHunspellResultCount = m_hunspell->suggest(&m_lastHunspellResult, query.substr(2).c_str()), g_debug("Hunspell: %s", query.substr(2).c_str());
//		else if (length > 4 && query[2] == '\'' && ::isalpha((unsigned char) query[0]) && ::isalpha((unsigned char) query[1]))
//			m_lastHunspellResultCount = m_hunspell->suggest(&m_lastHunspellResult, query.substr(3).c_str()), g_debug("Hunspell: %s", query.substr(3).c_str());
//		else
		m_lastHunspellResultCount = m_hunspell->suggest(&m_lastHunspellResult, query.c_str());
		m_lastHunspellQuery = query;
	}
}

/**
* setFistAndLastLetters()
* <here is function description>
*
* @param std::string & firstLetters
*   <perameter description>
*
* @param std::string & lastLetters
*   <perameter description>
*
* @param int minLength
*   <perameter description>
*
*/
void SmkyManufacturerDatabase::setFistAndLastLetters(const std::string & firstLetters, const std::string & lastLetters, int minLength)
{
	if (m_hunspell && m_lastHunspellResult)
	{
		m_hunspell->free_list(&m_lastHunspellResult, m_lastHunspellResultCount);
		m_lastHunspellResult = NULL;
	}
	m_lastFirstLastLetterResults.clear();
	auto_g_free_array<gunichar> first16 = g_utf8_to_ucs4(firstLetters.c_str(), -1, NULL, NULL, NULL);
	auto_g_free_array<gunichar> last16 = g_utf8_to_ucs4(lastLetters.c_str(), -1, NULL, NULL, NULL);
	if (first16 && last16)
	{
		gunichar * p = first16;
		while (*p)
			*p = g_unichar_tolower(*p), ++p;
		p = last16;
		while (*p)
			*p = g_unichar_tolower(*p), ++p;
		for (std::map<std::string, int>::iterator iter = m_words.begin(); iter != m_words.end(); ++iter)
		{
			const std::string & word = iter->first;
			const char * wordStr = word.c_str();
			gunichar firstLetter = g_unichar_tolower(g_utf8_get_char(wordStr));
			if (wcschr(first16.as<wchar_t>(), (wchar_t) firstLetter))
			{	// first letter is a match!
				const char * next = wordStr;
				int wordLength = 1;
				while ((next = g_utf8_next_char(wordStr)) && *next)
					wordStr = next, ++wordLength;
				if (wordLength >= minLength) {
					gunichar lastLetter = g_unichar_tolower(g_utf8_get_char(wordStr));
					if (wcschr(last16.as<wchar_t>(), (wchar_t) lastLetter))
					{
						DEBUG_CALLBACK("First-last letter match: %s-%s -> %s", firstLetters.c_str(), lastLetters.c_str(), word.c_str());
						m_lastFirstLastLetterResults.push_back(word.c_str());
					}
				}
				else {
					DEBUG_CALLBACK("'%s' is discarded because it's too short: %d letters, %d min", word.c_str(), wordLength, minLength);
				}
			}
		}
	}
}

/**
* dbCallback()
* <here is function description>
*
* @param SMKY_REQ_MODE eMdbRequestType
*   <perameter description>
*
* @param uint16_t wWordLen
*   <perameter description>
*
* @param uint16_t wMaxWordLen
*   <perameter description>
*
* @param uint16_t *psBuildTxtBuf
*   <perameter description>
*
* @param uint16_t *pwActWordLen
*   <perameter description>
*
* @param uint32_t *pdwWordListIdx
*   <perameter description>
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkyManufacturerDatabase::dbCallback(
    SMKY_REQ_MODE eMdbRequestType,   /**< I   - MDB request type. Should be one of the values defined above */
    uint16_t    wWordLen,          /**< I   - word length */
    uint16_t    wMaxWordLen,       /**< I   - maximum word length */
    uint16_t  *psBuildTxtBuf,     /**< O   - word to return */
    uint16_t   *pwActWordLen,      /**< O   - length of the returned word */
    uint32_t   *pdwWordListIdx)    /**< I/O - MDB word list index */
{
	//g_debug("In MDB Callback: idx=%lu, type=%u (%s)", *pdwWordListIdx, eMdbRequestType, (eMdbRequestType == SMKY_REQ_MODE_GETEXACTWORDS) ? "exact words" : (eMdbRequestType == SMKY_REQ_MODE_GETALLWORDS) ? "all words" : "neither exact nor all words");
	if (eMdbRequestType != SMKY_REQ_MODE_GETEXACTWORDS && eMdbRequestType != SMKY_REQ_MODE_GETALLWORDS)
		return SMKY_STATUS_ERROR;

	// first and follow-up callbacks: return words one-by-one, using *pdwWordListIdx as the index of the word to return in the list built above
	while (m_lastHunspellResult && (int) *pdwWordListIdx < m_lastHunspellResultCount)
	{
		const char * word = m_lastHunspellResult[*pdwWordListIdx];
		*pdwWordListIdx += 1;
		if (word && *word)
		{
			glong len(0);
			auto_g_free_array<gunichar2> utf16 = g_utf8_to_utf16(word, -1, NULL, &len, NULL);
			if (utf16)
			{
				memcpy(psBuildTxtBuf, utf16, len * sizeof(uint16_t));
				*pwActWordLen = len;
				DEBUG_CALLBACK("SmkyManufacturerDatabase::dbCallback: returning Hunspell word: '%s' for '%s' (%d-%d)", word, m_lastHunspellQuery.c_str(), wWordLen, wMaxWordLen);
				return SMKY_STATUS_NONE;
			}
			else
				g_warning("SmkyManufacturerDatabase::dbCallback: Hunspell returned a word that can't be converted to utf16: %s", word);
		}
	}

	while (*pdwWordListIdx < m_lastFirstLastLetterResults.size())
	{
		glong len(0);
		const char * word = m_lastFirstLastLetterResults[*pdwWordListIdx];
		auto_g_free_array<gunichar2> utf16 = g_utf8_to_utf16(word, -1, NULL, &len, NULL);
		*pdwWordListIdx += 1;
		if (utf16)
		{
			memcpy(psBuildTxtBuf, utf16, len * sizeof(uint16_t));
			*pwActWordLen = len;
			DEBUG_CALLBACK("SmkyManufacturerDatabase::dbCallback: returning first-last letter word: '%s' (%d-%d)", word, wWordLen, wMaxWordLen);
			return SMKY_STATUS_NONE;
		}
		else
			g_warning("SmkyManufacturerDatabase::dbCallback: m_lastFirstLastLetterResults contains a word that can't be converted to utf16: %s", word);
	}

	return SMKY_STATUS_ERROR;	// we're done
}

/**
* ManufacturerDbCallback()
* <here is function description>
*
* @param SMKY_LINFO *pLingInfo
*   pointer to FieldInfo struct owning MDB
*
* @param SMKY_REQ_MODE eMdbRequestType
*   MDB request type. Should be one of the values defined above
*
* @param uint16_t wWordLen
*   word length
*
* @param uint16_t wMaxWordLen
*   maximum word length
*
* @param uint16_t *psBuildTxtBuf
*   word to return
*
* @param uint16_t *pwActWordLen
*   length of the returned word
*
* @param uint32_t *pdwWordListIdx
*   MDB word list index
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkyManufacturerDatabase::ManufacturerDbCallback(
    SMKY_LINFO *pLingInfo,    
    SMKY_REQ_MODE eMdbRequestType,   
    uint16_t    wWordLen,          
    uint16_t    wMaxWordLen,       
    uint16_t  *psBuildTxtBuf,     
    uint16_t   *pwActWordLen,     
    uint32_t   *pdwWordListIdx)   
{
	if (g_ManDb == NULL) {
        g_warning("SmkyManufacturerDatabase::ManufacturerDbCallback: no manufacturer database");
		return SMKY_STATUS_ERROR;
	}
	else {
		return g_ManDb->dbCallback(eMdbRequestType, wWordLen, wMaxWordLen,
									psBuildTxtBuf, pwActWordLen, pdwWordListIdx);
	}
}

/**
* getExactMatch()
* <here is function description>
*
* @param std::string & outExactMatch
*   <perameter description>
*
* @return bool
*   TRUE :
*   FALSE:
*/
bool SmkyManufacturerDatabase::getExactMatch(std::string & outExactMatch)
{
	if (m_hunspell && m_lastHunspellResult && m_lastHunspellResultCount > 0)
	{
		for (int k = 0; k < m_lastHunspellResultCount; ++k)
		{
			if (::strcasecmp(m_lastHunspellResult[k], m_lastHunspellQuery.c_str()) == 0)
			{
				outExactMatch = m_lastHunspellResult[k];
				return true;
			}
		}
	}
	return false;
}

/**
* logStatistics()
* <here is function description>
*/
void SmkyManufacturerDatabase::logStatistics() const
{
	size_t sumWords = m_words.size();
	size_t sumChars = 0;
	size_t sumBytes = 0;
	size_t maxChars = 0;
	size_t minChars = 9999999;
	int maxCounts = 0;
	int minCounts = 9999999;
	int sumCounts = 0;
	int maxNumCounts = 0;

	std::map<std::string, int>::const_iterator i;
	for (i = m_words.begin(); i != m_words.end(); ++i) {

		const size_t len = i->first.size();
		maxChars = std::max(maxChars, len);
		minChars = std::min(minChars, len);
		sumChars += len;
		
		maxNumCounts = std::max(maxNumCounts, i->second);
		maxCounts = std::max(maxCounts, i->second);
		minCounts = std::min(minCounts, i->second);
		sumCounts += i->second;

		const size_t malloc_hdr_size = 2 * sizeof(int);
		const size_t str_obj_size = 2*sizeof(int);
		const size_t map_entry_size = 2*sizeof(int);
		sumBytes += sizeof(char) * len + str_obj_size + map_entry_size + malloc_hdr_size;
	}

	double avgEntrySize;
	double avgEntryChars;
	double avgEntryCount;
	if (sumWords > 0) {
		avgEntrySize = double(sumBytes) / double(sumWords);
		avgEntryCount = double(sumCounts) / double(sumWords);
		avgEntryChars = double(sumChars) / double(sumWords);
	}
	else {
		avgEntrySize = 0;
		avgEntryChars = 0;
		avgEntryCount = 0;
		minChars = 0;
	}

	g_debug("Man DB stats: numEntries=%u, map size=%u B, avgEntrySize=%g", sumWords, sumBytes, avgEntrySize);
	g_debug("Man DB stats:  minChars=%u, maxChars=%u, avgEntryChars=%g", minChars, maxChars, avgEntryChars);
	g_debug("Man DB stats:  minCounts=%u, maxCounts=%u, avgCounts=%g", minCounts, maxCounts, avgEntryCount);
}

/**
* loadWords()
* <here is function description>
*
* @param *path
*   <perameter description>
*
* @param wordList
*   <perameter description>
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkyManufacturerDatabase::loadWords(const char * path, std::list<std::string> & wordList)
{
	SMKY_STATUS wStatus = SMKY_STATUS_NONE;

	std::list<std::string>::const_iterator i;
	for (i = wordList.begin(); i != wordList.end(); ++i) {
		forgetWord(*i);
	}
	wordList.clear();

	FILE* f = fopen(path,"r");
	if (f) {
		char linebuffer[128];
		while (fgets(linebuffer, sizeof(linebuffer), f)) {
			StringUtils::chomp(linebuffer, G_N_ELEMENTS(linebuffer));
			std::string word(linebuffer);
			if (!word.empty()) {
				learnWord(word);
				wordList.push_back(word);
//				g_debug("Loaded '%s' from '%s'", word.c_str(), path);
			}
		}
        fclose(f);
	}
	else {
		wStatus = SMKY_STATUS_READ_DB_FAIL;
	}

	return wStatus;
}

/**
* loadDefaultData()
* Load our default hard-coded words and put them into this database. Save the words we add
* so that if the locale changes we can remove those words and re-read in new ones from
* the new locale.
*
* @param localeSettings
*   <perameter description>
*
* @return SMKY_STATUS
*   <return value description>
*/
SMKY_STATUS SmkyManufacturerDatabase::loadDefaultData (const LocaleSettings& localeSettings)
{
	std::string fname =  localeSettings.findLocalResource(m_settings.readOnlyDataDir + "/smky/DefaultData/manufacturer/", "/man-db-entries");
	SMKY_STATUS wStatus = loadWords(fname.c_str(), m_stockManDbWords);

	g_debug("Loaded %u words from man-db default data for locale '%s' from '%s'. Result: %u",
			m_stockManDbWords.size(), localeSettings.getFullLocale().c_str(), fname.c_str(), wStatus);

	return wStatus;
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
SmartKeyErrorCode SmkyManufacturerDatabase::setLocaleSettings (const LocaleSettings& localeSettings)

{
	return SmkyUserDatabase::smkyErrorToSmartKeyError(loadDefaultData(localeSettings));
}

} // end namespace

