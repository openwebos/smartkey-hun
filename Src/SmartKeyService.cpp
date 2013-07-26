/* @@@LICENSE
*
*      Copyright (c) 2009-2013 LG Electronics, Inc.
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
/*
 *
 * SmartKeyService
 *
 * Exposes SmartKey to the luna bus
 *
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <lunaservice.h>
#include <memory>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <glib.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <unistd.h>
#include <unicode/uchar.h>
#include "StringUtils.h"
#include <wctype.h>
#include <cjson/json.h>
#include "SmartKeyService.h"
#include "Settings.h"
#include <boost/algorithm/string.hpp>

#define USE_KEY_LOCALITY 1

using namespace SmartKey;

static bool       g_useSysLog = false;
static GMainLoop* g_mainloop;

// Define to put this service into a special test mode where it reads in test
// corpa and writes out statistics.
#undef GATHER_STATS

const char* const serviceName = "com.palm.smartKey";
const char* const dbModSignalName = "palm://com.palm.smartKey/com/palm/smartKey/databaseModified";
const char* const languageChangedSignalName = "palm://com.palm.smartKey/com/palm/smartKey/languageChanged";

/// We set this property value to signify that we've read the carrier db network
/// settings and have applied them once. After which the user can override if they
/// want.
const char* const k_setToCarrierDefaultsPropName = "setToCarrierDefaults";

/**
* Is this process running as a daemon (i.e. by upstart)?
*
* @return bool
*    This is only true because our devices don't have HOME set and the
*    desktop does. It would be nice to have a more accurate way of knowing
*    that this process is daemonized.
*/
static bool isDaemonized()
{
    return getenv("HOME") == NULL;
}

template <class T>
bool ValidJsonObject(T jsonObj)
{
    return NULL != jsonObj && !is_error(jsonObj);
}

/**
* log filter
*
* @param log_domain
*   log domain
*
* @param log_level
*   log level
*
* @param message
*   message
*
* @param unused_data
*   unused data
*/
static void logFilter (const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer unused_data)
{
#if defined(GATHER_STATS)
    if ((log_level & G_LOG_LEVEL_MASK) != G_LOG_LEVEL_CRITICAL)
        return;
#endif
    if (g_useSysLog)
    {
        int priority;

        switch (log_level & G_LOG_LEVEL_MASK)
        {
        case G_LOG_LEVEL_CRITICAL:
            priority = LOG_CRIT;
            break;
        case G_LOG_LEVEL_ERROR:
            priority = LOG_ERR;
            break;
        case G_LOG_LEVEL_WARNING:
            priority = LOG_WARNING;
            break;
        case G_LOG_LEVEL_MESSAGE:
            priority = LOG_NOTICE;
            break;
        case G_LOG_LEVEL_DEBUG:
            // No reason to log debug log messages to syslog.
            return;
        case G_LOG_LEVEL_INFO:
        default:
            priority = LOG_INFO;
            break;
        }
        syslog(priority, "%s", message);
    }
    else
    {
        g_log_default_handler(log_domain, log_level, message, unused_data);
    }
}

/**
* handle signal
*
* @param sig
*   signal
*/
static void handle_signal (int sig)
{
    syslog(LOG_DEBUG, "smartKey got signal: %d", sig);

    switch (sig)
    {
    case SIGQUIT:
    case SIGHUP:
        exit(0);
        break;
    default:
        exit(sig);
        break;
    }
}

/**
* termination handler
*
* @param signal
*   signal
*/
static void term_handler (int signal)
{
    g_main_loop_quit( g_mainloop );
    syslog(LOG_INFO, "Stopping smartKey: terminated by signal");
}

/**
* install signal handlers
*/
void installSignalHandlers (void)
{
    signal (SIGINT, handle_signal);
    signal (SIGHUP, handle_signal);
    signal (SIGQUIT, handle_signal);
    signal (SIGTERM, handle_signal);
    signal (SIGABRT, handle_signal);

    /* Man pages say prefer sigaction() to signal() */
    struct sigaction sact;
    memset( &sact, 0, sizeof(sact) );
    sact.sa_handler = term_handler;
    (void)sigaction( SIGTERM, &sact, NULL );
}

/**
* main
*
* @return int
*   exit code
*/
int main (void)
{
    syslog(LOG_INFO, "Starting smartKey service");

    installSignalHandlers();

    g_useSysLog = isDaemonized();
    g_log_set_default_handler(logFilter, NULL);

    const char* p_settings_file = "/etc/palm/smartkey.conf";

    if (!Settings::getInstance()->load(p_settings_file))
    {
        g_warning("Error loading settings from '%s'", p_settings_file);
    }

    g_mainloop = g_main_loop_new(NULL, FALSE);

    std::auto_ptr<SmartKey::SmartKeyService> service(new SmartKey::SmartKeyService());

    bool started = service->start(g_mainloop, serviceName);

    if (!started)
    {
        g_warning("%s: Failed to attach service handle to main loop. Exiting", __FUNCTION__);
        return -2;
    }
    else
    {
        g_message("%s: started service %s", __FUNCTION__, serviceName);
    }

    service->enable(true);
    service->registerForSystemServiceStatus();
    service->registerForMojoDbStatus();
    g_main_loop_run(g_mainloop);

    service->stop();

    return 0;
}


namespace SmartKey
{

// methods exposed via service
/*! \page com_palm_smartKey_service com.palm.smartKey service API
 *
 *  Methods:
 *   - \ref com_palm_smartKey_search
 *   - \ref com_palm_smartKey_learn
 *   - \ref com_palm_smartKey_addUserWord
 *   - \ref com_palm_smartKey_forget
 *   - \ref com_palm_smartKey_removeUserWord
 *   - \ref com_palm_smartKey_numUserWords
 *   - \ref com_palm_smartKey_listUserWords
 *   - \ref com_palm_smartKey_addAutoReplace
 *   - \ref com_palm_smartKey_removeAutoReplace
 *   - \ref com_palm_smartKey_listAutoReplace
 *   - \ref com_palm_smartKey_numAutoReplace
 *   - \ref com_palm_smartKey_addPerson
 *   - \ref com_palm_smartKey_removePerson
 *   - \ref com_palm_smartKey_exit
 *   - \ref com_palm_smartKey_processTaps
 *   - \ref com_palm_smartKey_getCompletion
 *   - \ref com_palm_smartKey_updateWordUsage
 */
static LSMethod serviceMethods[] =
{
    { "search", SmartKeyService::cmdSearch },
    { "learn", SmartKeyService::cmdAddUserWord },
    { "addUserWord", SmartKeyService::cmdAddUserWord },
    { "forget", SmartKeyService::cmdRemoveUserWord },
    { "removeUserWord", SmartKeyService::cmdRemoveUserWord },
    { "numUserWords", SmartKeyService::cmdNumUserWords },
    { "listUserWords", SmartKeyService::cmdListUserWords },
    { "addAutoReplace", SmartKeyService::cmdAddAutoReplace },
    { "removeAutoReplace", SmartKeyService::cmdRemoveAutoReplace },
    { "listAutoReplace", SmartKeyService::cmdListAutoReplace },
    { "numAutoReplace", SmartKeyService::cmdNumAutoReplace },
    { "addPerson", SmartKeyService::cmdAddPerson },
    { "removePerson", SmartKeyService::cmdRemovePerson },
    { "exit" , SmartKeyService::cmdExit },
    { "processTaps", SmartKeyService::cmdProcessTaps },
    { "getCompletion", SmartKeyService::cmdGetCompletion },
    { "updateWordUsage", SmartKeyService::cmdUpdateWordUsage },
    { 0, 0 },
};

LSSignal serviceSignals[] =
{
    { "databaseModified" },
    { "languageChanged" },
    {},
};

/**
* SmartKeyService constructor
*/
SmartKeyService::SmartKeyService(void) :
    m_service(NULL)
    , m_carrierDbWatchToken(0)
    , m_mainLoop(NULL)
    , m_isEnabled(false)
    , m_readPeople(false)
{
    m_engine = new SmkySpellCheckEngine();
//#ifdef TARGET_DESKTOP
//    m_engine->changedLocaleSettings();
//#endif
}

/**
* ~SmartKeyService
*/
SmartKeyService::~SmartKeyService()
{
    cancelCarrierDbSettingsWatch();

    delete m_engine;
}

/**
* get auto accept guess
*
* @param info
*   input: SpellCheckWordInfo
*
* @return WordGuess*
*   == NULL if no guesses found
*/
static const WordGuess* getAutoAcceptGuess (const SpellCheckWordInfo& info)
{
    std::vector<WordGuess>::const_iterator guess;
    for (guess = info.guesses.begin(); guess != info.guesses.end(); ++guess)
    {
        if (guess->autoAccept && !guess->autoReplace)
            return &*guess;
    }

    return NULL;
}

/**
* get auto accept guess
*
* @param info
*   input: SpellCheckWordInfo
*
* @return WordGuess*
*   == NULL if no guesses found
*/
static const WordGuess* getAutoReplaceGuess (const SpellCheckWordInfo& info)
{
    std::vector<WordGuess>::const_iterator guess;
    for (guess = info.guesses.begin(); guess != info.guesses.end(); ++guess)
    {
        if (guess->autoReplace)
            return &*guess;
    }

    return NULL;
}

/**
* calculate persent
*
* @param num
*   number
*
* @param denom
*   denom
*
* @return double
*   persent
*/
static double percent (double num, double denom)
{
    if (denom == 0.0)
        return 0.0;
    else
        return 100.0 * num / denom;
}

/**
* get auto replace stats
*
* @param fname
*   file name
*/
void SmartKeyService::getAutoReplaceStats (const std::string& fname) const
{
    FILE* f = fopen(fname.c_str(), "r");
    if (!f)
        return;

    int numWords = 0;
    int numSpelledCorrectly = 0;
    int numMisspelled = 0;	// AKA not in dictionary
    int numErrors = 0;
    int numAutoAccepted = 0;
    int numMatchingAutoAccept = 0;
    int numAutoReplaced = 0;
    int numMatchingAutoReplaced = 0;
    int numMatchingGuess[10] = {0};	// idx0 = any match, 1, 2, 3 are the number for that guess idx
    int totalNumGuesses = 0;

    char line[256];

    while (fgets(line, G_N_ELEMENTS(line), f) != NULL)
    {
        numWords++;
        const char* typedWord = strtok(line,"|\x0d\x0a" );
        if (typedWord == NULL)
            continue;
        const char* intendedWord = strtok(NULL,"|\x0d\x0a" );
        if (intendedWord == NULL)
            continue;

        SpellCheckWordInfo	result;
#if USE_KEY_LOCALITY
        SmartKeyErrorCode err = m_engine->autoCorrect(typedWord, "", result, 50 /* max guesses */);
#else
        SmartKeyErrorCode err = m_engine->checkSpelling(typedWord, result, 50 /* max guesses */);
#endif
        if (err == SKERR_SUCCESS)
        {
            if (result.inDictionary)
            {
                numSpelledCorrectly++;
                // printf("Correct: %s\n", typedWord);
            }
            else
            {
                numMisspelled++;
            }

            std::vector<WordGuess>::const_iterator g;
            size_t idxGuess = 0;
            for (g = result.guesses.begin(); g != result.guesses.end(); ++g, ++idxGuess)
            {
                totalNumGuesses++;

                if (idxGuess > 0 && g->guess == intendedWord)
                {
                    numMatchingGuess[0] += 1;

                    if (idxGuess > 0 && idxGuess < G_N_ELEMENTS(numMatchingGuess))
                        numMatchingGuess[idxGuess] += 1;
                }
            }

            const WordGuess* guess = getAutoReplaceGuess(result);
            if (guess != NULL)
            {
                numAutoReplaced++;
                if (guess->guess == intendedWord)
                    numMatchingAutoReplaced++;
            }
            else
            {
                guess = getAutoAcceptGuess(result);
                if (guess != NULL)
                {
                    numAutoAccepted++;
//					printf("Checking \"%s\" == \"%s\"\n", guess->guess.c_str(), intendedWord);
                    if (guess->guess == intendedWord)
                        numMatchingAutoAccept++;
                    else
                    {
//						printf("Auto-accepted \"%s\" for word \"%s\" was wrong, expected \"%s\"\n",
//								guess->guess.c_str(), typedWord, intendedWord);
                    }
                }
            }
        }
        else
        {
            numErrors++;
        }
    }

    fclose(f);

    printf("Num Words: %d\n", numWords);
    printf("Spelled correctly: %d (%g%%)\n", numSpelledCorrectly, percent(numSpelledCorrectly, numWords));
    printf("Mispelled: %d (%g%%)\n", numMisspelled, percent(numMisspelled, numWords));
    printf("Auto-replaced: %d (%g%%)\n", numAutoReplaced, percent(numAutoReplaced, numMisspelled));
    printf("Auto-replaced (matching): %d (%g%%)\n", numMatchingAutoReplaced, percent(numMatchingAutoReplaced, numAutoReplaced));
    printf("Auto-accepted: %d (%g%%)\n", numAutoAccepted, percent(numAutoAccepted, numMisspelled));
    printf("Auto accepted (matching): %d (%g%%)\n", numMatchingAutoAccept, percent(numMatchingAutoAccept, numAutoAccepted));
    printf("Any matching guess: %d (%g%%)\n", numMatchingGuess[0], percent(numMatchingGuess[0], numMisspelled));
    //for (size_t i = 1; i < G_N_ELEMENTS(numMatchingGuess); i++)
    //{
    //     printf("guess %lu matches: %d\n", i, numMatchingGuess[i]);
    //}
    printf("Average number of guesses for word: %g\n", double(totalNumGuesses) / double(numMisspelled));
    printf("Num errors: %d\n", numErrors);
}

/**
* dir name
*
* @param path
*   path to the file
*
* @return std::string
*   path to the file
*/
std::string SmartKeyService::dirName (const std::string& path)
{
    std::string dir;

    if (!path.empty())
    {
        char* tmp = strdup(path.c_str());
        if (tmp != NULL)
        {
            char* p(NULL);
            if ((p=dirname(tmp)) != NULL)
            {
                dir = p;
            }
            free(tmp);
        }
    }

    return dir;
}

/**
* get file size
*
* @param fname
*   path to file
*
* @return size_t
*   size of file
*/
size_t SmartKeyService::fileSize (const std::string& fname)
{
    struct stat buf;
    if (0 == stat(fname.c_str(), &buf))
    {
        return buf.st_size;
    }
    else
    {
        return InvalidFileSize;
    };
}

/**
* startup
*
* @param mainLoop
*   pointer to GMainLoop
*
* @param serviceName
*   service name
*
* @return bool
*   true if started
*/
bool SmartKeyService::start (GMainLoop* mainLoop, const char* serviceName)
{
    LSError lserror;
    LSErrorInit(&lserror);
    bool success;

    success = LSRegister(serviceName, &m_service, &lserror);
    if (!success)
    {
        LSErrorPrint (&lserror, stderr);
        LSErrorFree(&lserror);
        goto Exit;
    }

    success  = LSRegisterCategory(m_service, "/", serviceMethods, NULL, NULL, &lserror);
    if (!success)
    {
        LSErrorPrint (&lserror, stderr);
        LSErrorFree(&lserror);
        goto Exit;
    }

    success = LSRegisterCategory (m_service, "/com/palm/smartKey", NULL, serviceSignals, NULL, &lserror);
    if (!success)
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }

    success = LSCategorySetData(m_service, "/", this, &lserror);
    if (!success)
    {
        LSErrorPrint (&lserror, stderr);
        LSErrorFree(&lserror);
        goto Exit;
    }

    success = LSGmainAttach(m_service, mainLoop, &lserror);
    if (!success)
    {
        LSErrorPrint (&lserror, stderr);
        LSErrorFree(&lserror);
    }

    // publish supported correction languages
    if (m_engine)
    {
        json_object * jobj = json_object_new_object();
        json_object * jstring = json_object_new_string(m_engine->getSupportedLanguages());
        json_object_object_add(jobj,"x_palm_spelling_support", jstring);

        bool ret = LSCall(m_service, "palm://com.palm.systemservice/setPreferences", json_object_to_json_string(jobj), NULL, NULL, NULL, &lserror);
        if (!ret)
        {
            g_critical("%s: Failed to send supported languages (%s)", __FUNCTION__, lserror.message);
            LSErrorFree(&lserror);
        }

        json_object_put(jobj);
    }

Exit:
    if (success)
    {
        m_mainLoop = mainLoop;

        g_message("%s: started service %s", __FUNCTION__, serviceName);
    }
    else
    {
        g_debug("%s: failed to start service %s", __FUNCTION__, serviceName);
    }

    return success;
}

/**
* stop service
*
* @return bool
*   true if unregister from luna is succeed
*/
bool SmartKeyService::stop()
{
    g_message("%s: stopping service %s", __FUNCTION__, "smartKey");
    LSError lserror;
    LSErrorInit(&lserror);

    bool result = LSUnregister(m_service, &lserror);
    if (!result)
    {
        LSErrorPrint (&lserror, stderr);
        LSErrorFree(&lserror);
        return false;
    }

    m_service = NULL;

    return true;
}

/**
* translate errorcode to error description string
*
* @param err
*   error code
*
* @return const char*
*   pointer on the string with error description
*/
static const char* getErrorString (SmartKeyErrorCode err)
{
    switch (err)
    {
    case SKERR_SUCCESS:
        return "success";
    case SKERR_FAILURE:
        return "General failure";
    case SKERR_DISABLED:
        return "Service disabled";
    case SKERR_NOMEMORY:
        return "Insufficient memory";
    case SKERR_UNSUPPORTED:
        return "Unsupported feature";
    case SKERR_MISSING_PARAM:
        return "Missing parameter";
    case SKERR_BAD_PARAM:
        return "Invalid parameter";
    case SKERR_WORD_EXISTS:
        return "Word already exists";
    case SKERR_NO_MATCHING_WORDS:
        return "No matching words";
    case SKERR_BAD_WORD:
        return "Input word is incorrect";
    default:
        return "<UNKNOWN>";
    }
}

/**
* set reply response
*
* @param reply
*   reply in json format
*
* @param err
*   input: SmartKeyErrorCode
*/
void setReplyResponse (json_object* reply, SmartKeyErrorCode err)
{
    json_object_object_add(reply, "returnValue", json_object_new_boolean(err == SKERR_SUCCESS));

    if (err != SKERR_SUCCESS)
    {
        json_object_object_add(reply, "errorCode", json_object_new_int(err));
        json_object_object_add(reply, "errorText", json_object_new_string(getErrorString(err)));
    }
}

/**
* Determine if a word looks enough like a URL to not be spell checked.
*
* @param word
*   word to test
*
* @return bool
*   true if word is url (contain any of symbols '@:/.')
*/
bool SmartKeyService::wordIsUrl (const string& word)
{
    return( word.find_first_of("@:/.") != string::npos );
}

/**
* check if word contain punctuation symbols
*
* @param word
*   word to test
*
* @return bool
*   true if all symbols are punctuation
*/
bool SmartKeyService::wordIsAllPunctuation (const std::string& word)
{
    std::string::const_iterator i;
    for (i = word.begin(); i != word.end(); ++i)
    {
        if (!(ispunct(*i)))
            return false;
    }

    return true;
}

/**
* check if word contain punctuation symbols
*
* @param word
*   word to test
*
* @return bool
*   true if at least one symbol is punctuation
*/
bool SmartKeyService::wordHavePunctuation (const std::string& word)
{
    std::string::const_iterator i;
    for (i = word.begin(); i != word.end(); ++i)
    {
        if (ispunct(*i))
            return true;
    }

    return false;
}

/**
* trim spaces at end of string
*
* @param word
*   word to test
*
* @return string
*   word without spaces at end
*/
std::string SmartKeyService::rightTrim (std::string& word)
{
    std::string str = word;
    for (size_t i = str.length(); i > 0; --i)
    {
        if (str[i - 1] == ' ' || str[i - 1] == '\n' || str[i - 1] == '\t')
        {
            str.erase(i - 1, 1);
            ++i;
        }
    }

    return str;
}

/**
* verify input word for not allowed: numbers, alphanumerics, white spaces and special characters.
*
* @param word
*   word to test
*
* @return bool
*   true if validation is ok
*/
bool SmartKeyService::isGoodWord (std::string& word)
{
    std::string str = rightTrim(word);

    if ( (str.length() > 0) && !wordHavePunctuation(str) )
    {
        if( word.find_first_of("1234567890") == string::npos )
        {
            return true;
        }
    }

    return false;
}

/**
* validate string for numeric symbols only.
*
* @param word
*   word to test
*
* @return bool
*   true if validation is ok
*/
bool SmartKeyService::isNumber (std::string& word)
{
    std::string::const_iterator i;
    for (i = word.begin(); i != word.end(); ++i)
    {
        if (isdigit(*i) == 0)
            return false;
    }

    return true;
}

/**
* validate string for boolean {true, false}.
*
* @param word
*   word to test
*
* @return bool
*   true if validation is ok
*/
bool SmartKeyService::isBoolean (std::string& word)
{
    std::string str = word;
    boost::algorithm::to_lower(str);

    return ( (str.compare("true") == 0) || (str.compare("false") == 0) );
}

/**
* Strip the first and last characters (if they are punctuation) and return word
* If they are not punctuation then '\0' is returned.
*
* @param word
*   input word
*
* @param leadingChars
*   leading chars
*
* @param trailingChars
*   trailing chars
*
* @return std::string
*   return word
*/
std::string SmartKeyService::stripPunctuation (const std::string& word, std::string& leadingChars, std::string& trailingChars)
{
    leadingChars.clear();
    trailingChars.clear();

    if (word.empty())
        return word;

    // This could be made way more efficient, but most words do not have punctuation so
    // optimizing this function won't much of an impact on overall performance.
    std::string newWord = word;
    while (!newWord.empty() && u_ispunct(newWord[0]))
    {
        leadingChars += newWord[0];
        newWord.erase(0, 1);
    }

    int lastCharIdx = int(newWord.length()) - 1;
    int i = lastCharIdx;
    if (i > 0)
    {
        while (i >= 0 && u_ispunct(newWord[i]))
        {
            i--;
        }

        if (i != lastCharIdx)  	// if one or more chars were punctuation
        {
            i++;
            int charsToStrip = lastCharIdx - i + 1;
            g_debug("Stripping %d trailing chars from %d", charsToStrip, i);
            trailingChars.assign(newWord, i, charsToStrip);
            newWord.erase(i, charsToStrip);
        }
    }

    return newWord;
}


/**
* restore punctuation
*
* @param word
*   input word
*
* @param leadingChars
*   leading chars
*
* @param trailingChars
*   trailing chars
*
* @return std::string
*   return word
*/
std::string SmartKeyService::restorePunctuation (const std::string& word, const std::string& leadingChars, const std::string& trailingChars)
{
    // the following can be written in a single inefficient line. I prefer this, because string manipulations are expensive!
    std::string answer;

    if (leadingChars.size() > 0)
        answer = leadingChars + word;
    else
        answer = word;

    if (trailingChars.size() > 0)
        answer += trailingChars;

    return answer;
}

/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_search search

com_palm_smartKey_service/search

search the candidate substitution word for the query word.

\subsection com_palm_smartKey_service_syntax Syntax:
\code
{
	"query": string,
	"context": string,
	"quick": boolean
	"extended" : boolean
	"max": int
}
\endcode

\param query The word to correct. Required.
\param context The context word prior to the word to correct. Optional.
\param quick If set as true, engine will use checkSpelling, which is much faster but not as smart as autoCorrect which use reginal information
\param extended If set as true, engine will generate more suggestions in output (=60). Can be ommited.
\param max Maximum number of words for output result, by default is 10. This parameter have priority over parameter 'extended'. Can be ommited.

\subsection com_palm_smartKey_service_reply Reply:
\code
{
    "spelledCorrectly" : boolean
	"guesses : [
		{
        "str" : string
        "sp"  : boolean
        "auto-replace" : boolean
	 	"auto-accept": boolean
        }
      ]
    "returnValue": boolean
    "errorCode": int
    "errorText": string
}
\endcode
\param spelledCorrectly Set as true, if there is no error, otherwise false. Required
\param guesses a array of guess object which contain the substitution string. Optional
\param str The actual guess of the word.
\param sp  true if guess is a result of a spelling correction
\param auto-replace true if guess is a result of a auto-replace match
\param auto-accept true if engine is recommending to auto accept this guess.
\param returnValue true (success) or false (failure). Required
\param errorCode the error code of error if there is error. Optional
\param errorText the error text of error if there is error. Optional

\subsection com_palm_smartKey_service_examples Examples:
\code

luna-send -n 1 -f palm://com.palm.smartKey/search '{ "query": "oul", "context": "start", "quick": "false", "max": 10 }'
{
    "spelledCorrectly": false,
    "guesses": [
        {
            "str": "oul",
            "sp": false
        },
        {
            "str": "oulu",
            "sp": true,
            "auto-accept": true
        },
        {
            "str": "out",
            "sp": true
        },
        {
            "str": "our",
            "sp": true
        },
        {
            "str": "oil",
            "sp": true
        }
    ],
    "returnValue": true
}


\endcode
*/
bool SmartKeyService::cmdSearch (LSHandle* sh, LSMessage* message, void* ctx)
{
    if (!message)
    {
        return true;
    }

    double start = getTime();

    const char* payload = LSMessageGetPayload(message);

    g_debug("%s: received '%s'", __FUNCTION__, payload);

    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);

    LSError lserror;
    LSErrorInit(&lserror);

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        return false;
    }

    json_object* replyJson = json_object_new_object();
    SmartKeyErrorCode err = SKERR_SUCCESS;

    if (service->isEnabled())
    {
        SpellCheckWordInfo	result;

        int maxGuesses = SMK_MIN_GUESSES;

        std::string context;
        json_object* contextValue = json_object_object_get(json, "context");
        if (ValidJsonObject(contextValue))
            context = json_object_get_string(contextValue);

        json_object* extendedValue = json_object_object_get(json, "extended");
        if( ValidJsonObject(extendedValue) && json_object_get_boolean(extendedValue) )
        {
            maxGuesses = SMK_MAX_GUESSES;
        }

        json_object* limitValue = json_object_object_get(json, "max");
        if (ValidJsonObject(limitValue))
        {
            maxGuesses = json_object_get_int(limitValue);
        }

        json_object* value = json_object_object_get(json, "query");
        if (ValidJsonObject(value))
        {
            std::string query = json_object_get_string(value);

            if (query.empty() || wordIsAllPunctuation(query))
            {
                err = SKERR_BAD_PARAM;
                result.inDictionary = false;
            }
            else if (wordIsUrl(query))
            {
                // It's not really in the dictionary, but this will keep these things from
                // being underlined or auto-corrected.
                result.inDictionary = true;
            }
            else
            {
                if (!isGoodWord(query))
                {
                    err = SKERR_BAD_WORD;
                    result.inDictionary = false;
                }
                else
                {
                    // Before we spell check let's first check to see if the query (with any punctuation)
                    // matches an auto-replace entry. If so we'll do that first. Else spell-check.
                    SmkyAutoSubDatabase* autosubdatabase = service->m_engine->getAutoSubDatabase();
                    std::string substitution;
                    if (autosubdatabase)
                        substitution = autosubdatabase->findEntry(query);

                    if (!substitution.empty())
                    {
                        result.inDictionary = true;
                        if (query != substitution)  	// Only happens for ASDB entries that differ only by case (i->I)
                        {
                            //g_debug("'%s' found in auto-sub db. Returning as valid.", query.c_str());
                            result.guesses.push_back(WordGuess(query));	// First result is always input word.

                            WordGuess guess(substitution);
                            guess.autoReplace = true;
                            guess.autoAccept = true;
                            result.guesses.push_back(guess);
                        }
                    }
                    else
                    {
                        std::string leadingChars, trailingChars;
                        std::string strippedQuery = stripPunctuation(query, leadingChars, trailingChars);
                        std::string strippedContext;
                        if (context.length())
                        {
                            std::string leadingChars, trailingChars;
                            strippedContext = stripPunctuation(context, leadingChars, trailingChars);
                        }

    #if USE_KEY_LOCALITY
                        // "quick" will tell us if we should force the use of checkSpelling, which is much faster, but not as smart as autoCorrect which uses key regional information
                        bool	useAutoCorrect = true;
                        json_object * autorequest = json_object_object_get(json, "quick");
                        if (autorequest && ValidJsonObject(autorequest) && json_object_get_boolean(autorequest))
                            useAutoCorrect = false;

                        if (useAutoCorrect)
                            err = service->m_engine->autoCorrect(strippedQuery, strippedContext, result, maxGuesses);
                        else
                            err = service->m_engine->checkSpelling(strippedQuery, result, maxGuesses);
    #else
                        err = service->m_engine->checkSpelling(strippedQuery, result, maxGuesses);
    #endif
                        if (!leadingChars.empty() || !trailingChars.empty())
                        {
                            std::vector<WordGuess>::iterator gi;
                            for (gi = result.guesses.begin(); gi != result.guesses.end(); ++gi)
                            {
                                // Add the same punctuation to the guess to match the query word.
                                gi->guess = restorePunctuation(gi->guess, leadingChars, trailingChars);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            g_debug("Can't find query param.");
        }

        if (err == SKERR_SUCCESS)
        {
            json_object_object_add(replyJson, "spelledCorrectly", json_object_new_boolean(result.inDictionary));

            json_object* guessesJson = json_object_new_array();
            if (guessesJson)
            {
                std::vector<WordGuess>::const_iterator i;
                for (i = result.guesses.begin(); i != result.guesses.end(); ++i)
                {
                    json_object* wordReplyJson = json_object_new_object();
                    if (wordReplyJson)
                    {
                        json_object_object_add(wordReplyJson, "str", json_object_new_string(i->guess.c_str()) );
                        json_object_object_add(wordReplyJson, "sp", json_object_new_boolean(i->spellCorrection) );
                        if (i->autoReplace)
                        {
                            // default assumed to be false so will only set property if not the default
                            json_object_object_add(wordReplyJson, "auto-replace", json_object_new_boolean(i->autoReplace) );
                        }

                        if (i->autoAccept)
                        {
                            // default assumed to be false so will only set property if not the default
                            json_object_object_add(wordReplyJson, "auto-accept", json_object_new_boolean(i->autoAccept) );
                        }

                        json_object_array_add( guessesJson, wordReplyJson );
                    }
                }
                json_object_object_add( replyJson, const_cast<char*>("guesses"), guessesJson );
            }
        }
    }
    else
    {
        err = SKERR_DISABLED;
    }

    setReplyResponse(replyJson, err);
    const char * replyString = json_object_to_json_string(replyJson);

    if (!LSMessageReply(sh, message, replyString, &lserror))
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }
    g_debug("%s: %g msec to return '%s'", __FUNCTION__, (getTime()-start) * 1000.0, replyString);
    json_object_put(replyJson);
    json_object_put(json);

    return true;
}

/**
* notify user db change
*
* @param eAction
*   input: DbAction
*
* @param word
*   word
*
* @return bool
*   true if succeed
*/
bool SmartKeyService::notifyUserDbChange (DbAction eAction, const std::string& word)
{
    if (word.empty())
        return false;

    LSError lsError;

    LSErrorInit(&lsError);

    json_object* json = json_object_new_object();
    json_object_object_add(json, "database", json_object_new_string("user"));
    json_object_object_add(json, "action",
                           json_object_new_string(eAction == AddedToDatabase ? "added" : "removed"));
    json_object_object_add(json, "word", json_object_new_string(word.c_str()));

    bool succeeded = LSSignalSend(m_service, dbModSignalName, json_object_to_json_string(json), &lsError);
    if (!succeeded)
    {
        LSErrorPrint(&lsError, stderr);
        LSErrorFree(&lsError);
    }

    json_object_put(json);

    return succeeded;
}

/**
* notify volatile db change
*
* @param eAction
*   input: DbAction
*
* @param word
*   word
*
* @return bool
*   true if succeed
*/
bool SmartKeyService::notifyVolatileDbChange (DbAction eAction, const std::string& word)
{
    if (word.empty())
        return false;

    LSError lsError;

    LSErrorInit(&lsError);

    json_object* json = json_object_new_object();
    json_object_object_add(json, "database", json_object_new_string("volatile"));
    json_object_object_add(json, "action",
                           json_object_new_string(eAction == AddedToDatabase ? "added" : "removed"));
    json_object_object_add(json, "word", json_object_new_string(word.c_str()));

    bool succeeded = LSSignalSend(m_service, dbModSignalName, json_object_to_json_string(json), &lsError);
    if (!succeeded)
    {
        LSErrorPrint(&lsError, stderr);
        LSErrorFree(&lsError);
    }

    json_object_put(json);

    return succeeded;
}

/**
* notify auto replace db change
*
* @param eAction
*   input: DbAction
*
* @param entry
*   entry
*
* @return bool
*   true if succeed
*/
bool SmartKeyService::notifyAutoReplaceDbChange (DbAction eAction, const Entry& entry)
{
    if (entry.shortcut.empty())
        return false;

    LSError lsError;

    LSErrorInit(&lsError);

    json_object* json = json_object_new_object();
    json_object_object_add(json, "database", json_object_new_string("auto-replace"));
    json_object_object_add(json, "action",
                           json_object_new_string(eAction == AddedToDatabase ? "added" : "removed"));
    json_object_object_add(json, "shortcut", json_object_new_string(entry.shortcut.c_str()));
    json_object_object_add(json, "substitution",
                           json_object_new_string(entry.substitution.empty() ? "" : entry.substitution.c_str()));

    bool succeeded = LSSignalSend(m_service, dbModSignalName, json_object_to_json_string(json), &lsError);
    if (!succeeded)
    {
        LSErrorPrint(&lsError, stderr);
        LSErrorFree(&lsError);
    }

    json_object_put(json);

    return succeeded;
}

/**
* notify language changed
*
* @param eAction
*   input: LanguageAction
*
* @return bool
*   true if succeed
*/
bool SmartKeyService::notifyLanguageChanged (LanguageAction eAction)
{
    if (eAction == LanguageActionNone)
        return false;

    LSError lsError;
    LSErrorInit(&lsError);

    json_object* json = json_object_new_object();
    json_object_object_add(json, "action",
                           json_object_new_string(eAction == LanguageActionKeyboardChanged ? "keyboard" : "locale"));

    bool succeeded = LSSignalSend(m_service, languageChangedSignalName, json_object_to_json_string(json), &lsError);
    if (!succeeded)
    {
        LSErrorPrint(&lsError, stderr);
        LSErrorFree(&lsError);
    }

    json_object_put(json);

    return succeeded;
}

/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_addUserWord addUserWord

com_palm_smartKey_service/addUserWord

Add new word to spell engine dictionary. Same as the method "learn"
*/

/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_learn learn

com_palm_smartKey_service/learn

Add new word to spell engine dictionary

\subsection com_palm_smartKey_service_syntax Syntax:
\code
{
    "word": string
}
\endcode

\param word the word to be add to database, required

\subsection com_palm_smartKey_service_reply Reply:
\code
{
    "word": string
    "returnValue": boolean
    "errorCode": int
    "errorText": string
}
\endcode
\param word the word to be add to database, required
\param returnValue true (success) or false (failure). Required
\param errorCode the error code of error if there is error. Optional
\param errorText the error text of error if there is error. Optional

\subsection com_palm_smartKey_service_examples Examples:
\code
luna-send -n 1 -f palm://com.palm.smartKey/learn '{"word": "Oulu"}'
{
    "word": "oulu",
    "returnValue": true
}
\endcode
*/
bool SmartKeyService::cmdAddUserWord(LSHandle* sh, LSMessage* message, void* ctx)
{
    if (!message)
        return true;

    const char* payload = LSMessageGetPayload(message);

    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);

    if (!service || !service->isEnabled())
    {
        g_message("%s: service is not enabled", __FUNCTION__);
        return true;
    }

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        return false;
    }

    SmartKeyErrorCode err = SKERR_FAILURE;

    std::string word;

    json_object* value = json_object_object_get(json, "word");
    if (value)
    {
        word = json_object_get_string(value);
        word = StringUtils::utf8tolower(word);

        if (isGoodWord(word))
        {
            service->m_engine->getUserDatabase()->learnWord(word);
            err = SKERR_SUCCESS;
        }
        else
        {
            err = SKERR_BAD_WORD;
        }

    }
    else
    {
        err = SKERR_MISSING_PARAM;
    }

    if (err == SKERR_SUCCESS)
    {
        service->m_engine->getUserDatabase()->save();
    }

    json_object* replyJson = json_object_new_object();

    if (!word.empty())
        json_object_object_add(replyJson, "word", json_object_new_string(word.c_str()) );

    setReplyResponse(replyJson, err);

    LSError lserror;
    LSErrorInit(&lserror);

    if (!LSMessageReply(sh, message, json_object_to_json_string(replyJson), &lserror))
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }
    json_object_put(replyJson);
    json_object_put(json);

    if (err == SKERR_SUCCESS)
    {
        service->notifyUserDbChange(AddedToDatabase, word);
    }

    return true;
}

/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_numUserWords numUserWords

com_palm_smartKey_service/numUserWords

Get the number of entries of the database

\subsection com_palm_smartKey_service_syntax Syntax:
\code
{
}
\endcode

\subsection com_palm_smartKey_service_reply Reply:
\code
{
    "count": int
    "returnValue": boolean
    "errorCode": int
    "errorText": string
}
\endcode
\param count the number of entries of spell engine database. Required
\param returnValue true (success) or false (failure). Required
\param errorCode the error code of error if there is error. Optional
\param errorText the error text of error if there is error. Optional

\subsection com_palm_smartKey_service_examples Examples:
\code
luna-send -n 1 -f palm://com.palm.smartKey/numUserWords '{}'
{
    "returnValue": true,
    "count": 2
}
\endcode
*/
bool SmartKeyService::cmdNumUserWords(LSHandle* sh, LSMessage* message, void* ctx)
{
    if (!message)
        return true;

    const char* payload = LSMessageGetPayload(message);

    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);

    if (!service || !service->isEnabled())
    {
        g_message("%s: service is not enabled", __FUNCTION__);
        return true;
    }

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        return false;
    }

    int numEntries(0);
    SmartKeyErrorCode err = SKERR_SUCCESS;
    service->m_engine->getUserDatabase()->getNumEntries(numEntries);

    json_object* replyJson = json_object_new_object();

    setReplyResponse(replyJson, err);

    if (err == SKERR_SUCCESS)
    {
        json_object_object_add(replyJson, "count", json_object_new_int(numEntries));
    }

    LSError lserror;
    LSErrorInit(&lserror);

    if (!LSMessageReply(sh, message, json_object_to_json_string(replyJson), &lserror))
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }
    json_object_put(replyJson);
    json_object_put(json);

    return true;
}

/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_listUserWords listUserWords

com_palm_smartKey_service/listUserWords

Get the used words list from spell engine database

\subsection com_palm_smartKey_service_syntax Syntax:
\code
{
	"offset": int
	"limit": int
}
\endcode

\param offset the offset of the start entry
\param limit the size of entries

\subsection com_palm_smartKey_service_reply Reply:
\code
{
	"words": [string]
    "returnValue": boolean
    "errorCode": int
    "errorText": string
}
\endcode
\param words the word array for the query. Required
\param returnValue true (success) or false (failure). Required
\param errorCode the error code of error if there is error. Optional
\param errorText the error text of error if there is error. Optional

\subsection com_palm_smartKey_service_examples Examples:
\code
luna-send -n 1 -f palm://com.palm.smartKey/listUserWords '{ "offset":1, "limit":2}'
{
    "returnValue": true,
    "words": [
        "haerbin",
        "kemi"
    ]
}
\endcode
*/
bool SmartKeyService::cmdListUserWords(LSHandle* sh, LSMessage* message, void* ctx)
{
    if (!message)
        return true;

    const char* payload = LSMessageGetPayload(message);

    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);

    if (!service || !service->isEnabled())
    {
        g_message("%s: service is not enabled", __FUNCTION__);
        return true;
    }

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        return false;
    }

    std::list<std::string> words;
    SmartKeyErrorCode err = SKERR_FAILURE;

    json_object* value = json_object_object_get(json, "offset");
    if (value)
    {
        std::string str_offset;
        str_offset = json_object_get_string(value);

        //test for digits
        if (isNumber(str_offset))
        {
            int offset = atoi(str_offset.c_str());//json_object_get_int(value);

            value = json_object_object_get(json, "limit");
            if (value)
            {
                std::string str_limit;
                str_limit = json_object_get_string(value);

                //test for digits
                if (isNumber(str_limit))
                {
                    int limit = atoi(str_limit.c_str());//json_object_get_int(value);
                    err = service->m_engine->getUserDatabase()->getEntries(offset, limit, words);
                }
                else
                {
                    err = SKERR_BAD_PARAM;
                }
            }
            else
            {
                err = SKERR_MISSING_PARAM;
            }
        }
        else
        {
            err = SKERR_BAD_PARAM;
        }
    }
    else
    {
        err = SKERR_MISSING_PARAM;
    }

    json_object* replyJson = json_object_new_object();

    setReplyResponse(replyJson, err);

    if (err == SKERR_SUCCESS)
    {
        json_object* wordsJson = json_object_new_array();
        if (wordsJson)
        {
            std::list<std::string>::const_iterator i;
            for (i = words.begin(); i != words.end(); ++i)
            {
                json_object_array_add( wordsJson, json_object_new_string(i->c_str()) );
            }
            json_object_object_add( replyJson, const_cast<char*>("words"), wordsJson );
        }
    }

    LSError lserror;
    LSErrorInit(&lserror);

    if (!LSMessageReply(sh, message, json_object_to_json_string(replyJson), &lserror))
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }
    json_object_put(replyJson);
    json_object_put(json);

    return true;
}

/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_addAutoReplace addAutoReplace

com_palm_smartKey_service/addAutoReplace

Add an auto replace entry to spell engine database

\subsection com_palm_smartKey_service_syntax Syntax:
\code
{
    "shortcut":  string
    "substitution": string the substitution word for shortcut
}
\endcode

\param shutcut the string to be replaced
\param substitution the substitution word for shortcut

\subsection com_palm_smartKey_service_reply Reply:
\code
{
    "returnValue": boolean
    "errorCode": int
    "errorText": string
}
\endcode
\param returnValue true (success) or false (failure). Required
\param errorCode the error code of error if there is error. Optional
\param errorText the error text of error if there is error. Optional

\subsection com_palm_smartKey_service_examples Examples:
\code
luna-send -n 1 -f palm://com.palm.smartKey/addAutoReplace '{ "shortcut":"OuluU", "substitution": "Oulu Univeristy"}'
{
    "returnValue": true
}


\endcode
*/
bool SmartKeyService::cmdAddAutoReplace(LSHandle* sh, LSMessage* message, void* ctx)
{
    if (!message)
        return true;

    const char* payload = LSMessageGetPayload(message);

    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);

    if (!service || !service->isEnabled())
    {
        g_message("%s: service is not enabled", __FUNCTION__);
        return true;
    }

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        return false;
    }

    SmartKeyErrorCode err = SKERR_FAILURE;

    Entry entry;

    SmkyAutoSubDatabase* autosubdatabase = service->m_engine->getAutoSubDatabase();
    if (autosubdatabase)
    {
        json_object* value = json_object_object_get(json, "shortcut");
        if (value)
        {
            entry.shortcut = json_object_get_string(value);

            value = json_object_object_get(json, "substitution");
            if (value)
            {
                entry.substitution = json_object_get_string(value);

                if (isGoodWord(entry.shortcut) && isGoodWord(entry.substitution))
                {
                    err = autosubdatabase->addEntry(entry);
                    autosubdatabase->save();
                }
                else
                {
                    err = SKERR_BAD_WORD;
                }
            }
            else
            {
                err = SKERR_MISSING_PARAM;
            }
        }
        else
        {
            err = SKERR_MISSING_PARAM;
        }
    }

    json_object* replyJson = json_object_new_object();

    setReplyResponse(replyJson, err);

    LSError lserror;
    LSErrorInit(&lserror);

    if (!LSMessageReply(sh, message, json_object_to_json_string(replyJson), &lserror))
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }
    json_object_put(replyJson);
    json_object_put(json);

    if (err == SKERR_SUCCESS)
    {
        service->notifyAutoReplaceDbChange(SmartKeyService::AddedToDatabase, entry);
    }

    return true;
}

/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_removeAutoReplace removeAutoReplace

com_palm_smartKey_service/removeAutoReplace

Remove the auto replace entry from spell engine database

\subsection com_palm_smartKey_service_syntax Syntax:
\code
{
	"shortcut": string
}
\endcode

\param shortcut the shortcut string to be removed from database

\subsection com_palm_smartKey_service_reply Reply:
\code
{
    "returnValue": boolean
    "errorCode": int
    "errorText": string
}
\endcode
\param returnValue true (success) or false (failure). Required
\param errorCode the error code of error if there is error. Optional
\param errorText the error text of error if there is error. Optional

\subsection com_palm_smartKey_service_examples Examples:
\code
luna-send -n 1 -f palm://com.palm.smartKey/removeAutoReplace '{"shortcut": "OuluU"}'
{
    "returnValue": true
}

\endcode
*/
bool SmartKeyService::cmdRemoveAutoReplace(LSHandle* sh, LSMessage* message, void* ctx)
{
    if (!message)
        return true;

    const char* payload = LSMessageGetPayload(message);

    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);

    if (!service || !service->isEnabled())
    {
        g_message("%s: service is not enabled", __FUNCTION__);
        return true;
    }

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        return false;
    }

    SmartKeyErrorCode err(SKERR_FAILURE);

    std::string shortcut;
    SmkyAutoSubDatabase* autosubdatabase = service->m_engine->getAutoSubDatabase();
    if (autosubdatabase)
    {
        json_object* value = json_object_object_get(json, "shortcut");
        if (value)
        {
            shortcut = json_object_get_string(value);

            if(autosubdatabase->forgetWord(shortcut))
            {
                autosubdatabase->save();
                err = SKERR_SUCCESS;
            }
            else
            {
                err = SKERR_NO_MATCHING_WORDS;
            }
        }
    }

    json_object* replyJson = json_object_new_object();

    setReplyResponse(replyJson, err);

    LSError lserror;
    LSErrorInit(&lserror);

    if (!LSMessageReply(sh, message, json_object_to_json_string(replyJson), &lserror))
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }
    json_object_put(replyJson);
    json_object_put(json);

    if (err == SKERR_SUCCESS)
    {
        Entry entry;
        entry.shortcut = shortcut;
        service->notifyAutoReplaceDbChange(SmartKeyService::RemovedFromDatabase, entry);
    }

    return true;
}

/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_numAutoReplace numAutoReplace

com_palm_smartKey_service/numAutoReplace

Get the number of auto replace entries

\subsection com_palm_smartKey_service_syntax Syntax:
\code
{
    "all": boolean
}
\endcode

\param all if true, set the entry category to AllEntries, otherwise use UserEntries

\subsection com_palm_smartKey_service_reply Reply:
\code
{
    "count": int
    "returnValue": boolean
    "errorCode": int
    "errorText": string
}
\endcode
\count the auto replace entry number of the category in spell engine database
\param returnValue true (success) or false (failure). Required
\param errorCode the error code of error if there is error. Optional
\param errorText the error text of error if there is error. Optional

\subsection com_palm_smartKey_service_examples Examples:
\code
luna-send -n 1 -f palm://com.palm.smartKey/numAutoReplace '{ "all":false}'
{
    "returnValue": true,
    "count": 1
}
\endcode
*/
bool SmartKeyService::cmdNumAutoReplace(LSHandle* sh, LSMessage* message, void* ctx)
{
    if (!message)
        return true;

    const char* payload = LSMessageGetPayload(message);

    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);

    if (!service || !service->isEnabled())
    {
        g_message("%s: service is not enabled", __FUNCTION__);
        return true;
    }

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        return false;
    }

    SmartKeyErrorCode err = SKERR_SUCCESS;
    WhichEntries whichEntries = UserEntries;
    int numEntries(0);

    json_object* value = json_object_object_get(json, "all");
    if (ValidJsonObject(value))
    {
        std::string str = json_object_get_string(value);
        if (isBoolean(str))
        {
            if (json_object_get_boolean(value))
                whichEntries = AllEntries;

            SmkyAutoSubDatabase* autosubdatabase = service->m_engine->getAutoSubDatabase();
            err = autosubdatabase ? autosubdatabase->getNumEntries(whichEntries, numEntries) : SKERR_FAILURE;
        }
        else
        {
            err = SKERR_BAD_PARAM;
        }
    }
    else
    {
        err = SKERR_MISSING_PARAM;
    }

    json_object* replyJson = json_object_new_object();

    setReplyResponse(replyJson, err);
    if (err == SKERR_SUCCESS)
    {
        json_object_object_add(replyJson, "count", json_object_new_int(numEntries));
    }

    LSError lserror;
    LSErrorInit(&lserror);

    if (!LSMessageReply(sh, message, json_object_to_json_string(replyJson), &lserror))
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }
    json_object_put(replyJson);
    json_object_put(json);

    return true;
}

/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_listAutoReplace listAutoReplace

com_palm_smartKey_service/listAutoReplace

List auto replace entries which cotains the entries from offset with size limitation

\subsection com_palm_smartKey_service_syntax Syntax:
\code
{
    "offset": int
    "all": boolean
    "limit": int
}
\endcode

\param offset the offset position of the entry
\param all if true, set entry category to AllEnties, otherwise use UserEntries
\param limit the size of the return array

\subsection com_palm_smartKey_service_reply Reply:
\code
{
    "entries: [
        {
            "shortcut": shortcut of entries
            "sub": the substitution of the shortcut
        }
    ]
    "returnValue": boolean
    "errorCode": int
    "errorText": string
}
\endcode
\param entries The entry array. Required
\param returnValue true (success) or false (failure). Required
\param errorCode the error code of error if there is error. Optional
\param errorText the error text of error if there is error. Optional

\subsection com_palm_smartKey_service_examples Examples:
\code
luna-send -n 1 -f palm://com.palm.smartKey/listAutoReplace '{ "offset":0, "all":false, "limit":2}'
{
    "returnValue": true,
    "entries": [
        {
            "shortcut": "OuluU",
            "sub": "Oulu Univeristy"
        }
    ]
}
\endcode
*/
bool SmartKeyService::cmdListAutoReplace(LSHandle* sh, LSMessage* message, void* ctx)
{
    if (!message)
        return true;

    const char* payload = LSMessageGetPayload(message);

    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);

    if (!service || !service->isEnabled())
    {
        g_message("%s: service is not enabled", __FUNCTION__);
        return true;
    }

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        return false;
    }

    SmartKeyErrorCode err = SKERR_FAILURE;

    std::list<Entry> entries;

    json_object* value = json_object_object_get(json, "offset");
    if (value)
    {
        std::string str_offset;
        str_offset = json_object_get_string(value);

        //test for digits
        if (isNumber(str_offset))
        {
            int offset = atoi(str_offset.c_str());//json_object_get_int(value);

            WhichEntries whichEntries = UserEntries;
            value = json_object_object_get(json, "all");
            if (ValidJsonObject(value) && json_object_get_boolean(value))
            {
                whichEntries = AllEntries;
            }

            value = json_object_object_get(json, "limit");
            if (value)
            {
                std::string str_limit;
                str_limit = json_object_get_string(value);

                //test for digits
                if (isNumber(str_limit))
                {
                    int limit = atoi(str_limit.c_str());//json_object_get_int(value);
                    SmkyAutoSubDatabase* autosubdatabase = service->m_engine->getAutoSubDatabase();
                    if (autosubdatabase)
                        err = autosubdatabase->getEntries(offset, limit, whichEntries, entries);
                }
                else
                {
                    err = SKERR_BAD_PARAM;
                }
            }
            else
            {
                err = SKERR_MISSING_PARAM;
            }
        }
        else
        {
            err = SKERR_BAD_PARAM;
        }
    }
    else
    {
        err = SKERR_MISSING_PARAM;
    }

    json_object* replyJson = json_object_new_object();

    setReplyResponse(replyJson, err);
    if (err==SKERR_SUCCESS)
    {

        json_object* entriesJson = json_object_new_array();
        if (entriesJson)
        {
            std::list<Entry>::const_iterator entry;
            for (entry = entries.begin(); entry != entries.end(); ++entry)
            {
                json_object* entryJson = json_object_new_object();
                if (entryJson)
                {
                    json_object_object_add(entryJson, "shortcut", json_object_new_string(entry->shortcut.c_str()) );
                    json_object_object_add(entryJson, "sub", json_object_new_string(entry->substitution.c_str()) );

                    json_object_array_add( entriesJson, entryJson );
                }
            }
            json_object_object_add( replyJson, const_cast<char*>("entries"), entriesJson );
        }
    }

    LSError lserror;
    LSErrorInit(&lserror);

    if (!LSMessageReply(sh, message, json_object_to_json_string(replyJson), &lserror))
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }
    json_object_put(replyJson);
    json_object_put(json);

    return true;
}

/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_addPerson addPerson

com_palm_smartKey_service/addPerson

Add person name array to spell engine database

\subsection com_palm_smartKey_service_syntax Syntax:
\code
{
    [
        {
            "familyName": string
            "midlleName": string
            "givenName": string
        }
    ]
}
\endcode

\param familyName Optional
\param midlleName Optional
\param givenName Optional

\subsection com_palm_smartKey_service_reply Reply:
\code
{
    "returnValue": boolean
    "errorCode": int
    "errorText": string
}
\endcode
\param returnValue true (success) or false (failure). Required
\param errorCode the error code of error if there is error. Optional
\param errorText the error text of error if there is error. Optional

\subsection com_palm_smartKey_service_examples Examples:
\code
luna-send -n 1 -f palm://com.palm.smartKey/addPerson '[{"familyName":"Sun","givenName":"zheng"}, {"familyName":"Wang", "givenName":"rong"}]'
{
    "returnValue": true
}
\endcode
*/
//{Igor: relinked to user database, GF-1374}
bool SmartKeyService::cmdAddPerson(LSHandle* sh, LSMessage* message, void* ctx)
{
    if (!message)
        return true;

    const char* payload = LSMessageGetPayload(message);

    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);

    if (!service || !service->isEnabled())
    {
        g_message("%s: service is not enabled", __FUNCTION__);
        return true;
    }

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        return false;
    }

    SmartKeyErrorCode err = SKERR_SUCCESS;
    std::set<std::string> allNames;

    SmkyUserDatabase* db = service->m_engine->getUserDatabase();
    if (db == NULL)
        return true;

    array_list* names = json_object_get_array(json);
    int numNames = array_list_length(names);
    for (int n = 0; n < numNames; n++)
    {
        json_object* name = static_cast<json_object*>(array_list_get_idx(names, n));

        Name	parsedName;
        if (parseName(name, parsedName))
        {
            for (std::vector<std::string>::const_iterator iter = parsedName.m_names.begin(); iter != parsedName.m_names.end(); ++iter)
            {
                db->learnContextWord(*iter);
                allNames.insert(*iter);
            }
        }
    }

    json_object* replyJson = json_object_new_object();

    setReplyResponse(replyJson, err);

    LSError lserror;
    LSErrorInit(&lserror);

    if (!LSMessageReply(sh, message, json_object_to_json_string(replyJson), &lserror))
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }
    json_object_put(replyJson);
    json_object_put(json);

    if (err == SKERR_SUCCESS)
    {
        std::set<std::string>::const_iterator i;
        for (i = allNames.begin(); i != allNames.end(); ++i)
        {
            service->notifyVolatileDbChange(SmartKeyService::AddedToDatabase, *i);
        }
    }

    return true;
}

/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_removePerson removePerson

com_palm_smartKey_service/removePerson

Remove the person name array from spell engine database

\subsection com_palm_smartKey_service_syntax Syntax:
\code
{
    [
        {
            "familyName": string
            "midlleName": string
            "givenName": string
        }
    ]
}
\endcode

\param familyName Required
\param midlleName Required
\param givenName Required

\subsection com_palm_smartKey_service_reply Reply:
\code
{
    "returnValue": boolean
    "errorCode": int
    "errorText": string
}
\endcode
\param returnValue true (success) or false (failure). Required
\param errorCode the error code of error if there is error. Optional
\param errorText the error text of error if there is error. Optional

\subsection com_palm_smartKey_service_examples Examples:
\code
luna-send -n 1 -f palm://com.palm.smartKey/removePerson '[{"familyName":"Sun", "givenName":"zheng"}, {"familyName":"Wang", "givenName":"rong"}]'
{
    "returnValue": true
}
\endcode
*/
//{Igor: relinked to user database, GF-1374}
bool SmartKeyService::cmdRemovePerson(LSHandle* sh, LSMessage* message, void* ctx)
{
    if (!message)
        return true;

    const char* payload = LSMessageGetPayload(message);

    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);

    if (!service || !service->isEnabled())
    {
        g_message("%s: service is not enabled", __FUNCTION__);
        return true;
    }

    SmkyUserDatabase* db = service->m_engine->getUserDatabase();
    if (db == NULL)
        return true;

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        return false;
    }

    SmartKeyErrorCode err(SKERR_FAILURE);
    std::set<std::string> allNames;

    array_list* names = json_object_get_array(json);
    int numNames = array_list_length(names);
    for (int n = 0; n < numNames; n++)
    {
        json_object* name = static_cast<json_object*>(array_list_get_idx(names, n));

        Name	parsedName;
        if (parseName(name, parsedName))
        {
            for (std::vector<std::string>::const_iterator iter = parsedName.m_names.begin(); iter != parsedName.m_names.end(); ++iter)
            {
                if(db->forgetContextWord(*iter))
                {
                    db->save();
                    allNames.insert(*iter);
                    err = SKERR_SUCCESS;
                }
                else
                {
                    err = SKERR_NO_MATCHING_WORDS;
                }
            }
        }
    }

    json_object* replyJson = json_object_new_object();

    setReplyResponse(replyJson, err);

    LSError lserror;
    LSErrorInit(&lserror);

    if (!LSMessageReply(sh, message, json_object_to_json_string(replyJson), &lserror))
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }
    json_object_put(replyJson);
    json_object_put(json);

    if (err == SKERR_SUCCESS)
    {
        std::set<std::string>::const_iterator i;
        for (i = allNames.begin(); i != allNames.end(); ++i)
        {
            service->notifyVolatileDbChange(SmartKeyService::RemovedFromDatabase, *i);
        }
    }

    return true;
}

/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_removeUserWord removeUserWord

com_palm_smartKey_service/removeUserWord

Remove the word from dictionary. Same as method "forget"
*/
/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_forget forget

com_palm_smartKey_service/forget

Remove the word from dictionary

\subsection com_palm_smartKey_service_syntax Syntax:
\code
{
    "word": string
}
\endcode

\param word the word to be removed from database. Required

\subsection com_palm_smartKey_service_reply Reply:
\code
{
    "word": string
    "returnValue": boolean
    "errorCode": int
    "errorText": string
}
\endcode
\param word the word to be remove from database, required
\param returnValue true (success) or false (failure). Required
\param errorCode the error code of error if there is error. Optional
\param errorText the error text of error if there is error. Optional

\subsection com_palm_smartKey_service_examples Examples:
\code
luna-send -n 1 -f palm://com.palm.smartKey/forget '{ "word": "oulu"}'
{
    "word": "oulu",
    "returnValue": true
}
\endcode
*/
bool SmartKeyService::cmdRemoveUserWord(LSHandle* sh, LSMessage* message, void* ctx)
{
    if (!message)
        return true;

    const char* payload = LSMessageGetPayload(message);

    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);

    if (!service || !service->isEnabled())
    {
        g_message("%s: service is not enabled", __FUNCTION__);
        return true;
    }

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        return false;
    }

    SmartKeyErrorCode err = SKERR_FAILURE;

    std::string word;
    json_object* value = json_object_object_get(json, "word");
    if (value)
    {
        word = json_object_get_string(value);

        if(service->m_engine->getUserDatabase()->forgetWord(word))
        {
            service->m_engine->getUserDatabase()->save();
            err = SKERR_SUCCESS;
        }
        else
        {
            err = SKERR_NO_MATCHING_WORDS;
        }
    }

    json_object* replyJson = json_object_new_object();

    if (!word.empty())
        json_object_object_add(replyJson, "word", json_object_new_string(word.c_str()) );

    setReplyResponse(replyJson, err);

    LSError lserror;
    LSErrorInit(&lserror);

    if (!LSMessageReply(sh, message, json_object_to_json_string(replyJson), &lserror))
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }
    json_object_put(replyJson);
    json_object_put(json);

    if (err == SKERR_SUCCESS)
    {
        service->notifyUserDbChange(SmartKeyService::RemovedFromDatabase, word);
    }

    return true;
}

/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_exit exit

com_palm_smartKey_service/exit

DESCRIPTION

\subsection com_palm_smartKey_service_syntax Syntax:
\code
{
}
\endcode

\subsection com_palm_smartKey_service_reply Reply:
\code
{
    "returnValue": boolean
    "errorCode": int
    "errorText": string
}
\endcode
\param returnValue true (success) or false (failure). Required
\param errorCode the error code of error if there is error. Optional
\param errorText the error text of error if there is error. Optional

\subsection com_palm_smartKey_service_examples Examples:
\code
luna-send -n 1 -f palm://com.palm.smartKey/exit '{}'
{
    "returnValue": true
}

\endcode
*/
bool SmartKeyService::cmdExit(LSHandle* sh, LSMessage* message, void* ctx)
{
    json_object* replyJson = json_object_new_object();

    setReplyResponse(replyJson, SKERR_SUCCESS);

    LSError lserror;
    LSErrorInit(&lserror);

    if (!LSMessageReply(sh, message, json_object_to_json_string(replyJson), &lserror))
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }
    json_object_put(replyJson);

    g_main_loop_quit(g_mainloop);

    return true;
}

/**
* set perferences callback
*
* @param *sh
*   input: LSHandle
*
* @param *message
*   input: LSMessage
*
* @param *ctx
*   input: context
*
* @return bool
*   return always true
*/
bool SmartKeyService::setPrefsCallback (LSHandle *sh, LSMessage *message, void *ctx)
{
    g_debug("Preferences saved");

    return true;
}

/**
* disable spelling auto correction
*/
void SmartKeyService::disableSpellingAutoCorrection (void)
{
    const std::string key("x_palm_textinput");
    if (!m_currTextInputPrefs.empty())
    {
        json_object* json = json_tokener_parse(m_currTextInputPrefs.c_str());
        if (ValidJsonObject(json))
        {

            const char* const scPropName = "spellChecking";
            json_object* prop = json_object_object_get(json, scPropName);
            if (ValidJsonObject(prop))
            {

                // disableAutotext only means don't auto-correct.
                std::string strProp = json_object_get_string(prop);
                if (strProp == "autoCorrect")
                {
                    json_object_object_add(json, scPropName, json_object_new_string("underline"));
                }

                json_object_object_add(json, k_setToCarrierDefaultsPropName, json_object_new_boolean(true));

                // Only change the pref if spellChecking is defined. This just makes development
                // easier because it allows the preference to be set to {} and then changed again.
                char* newVal(NULL);
                if (asprintf(&newVal, "{\"x_palm_textinput\":%s}", json_object_get_string(json)) > 0)
                {
                    // Now write back the modified preference.
                    LSError lserror;
                    LSErrorInit(&lserror);
                    if (!LSCall(m_service, "palm://com.palm.systemservice/setPreferences", newVal,
                                setPrefsCallback, this, NULL, &lserror))
                    {
                        LSErrorPrint(&lserror, stderr);
                        LSErrorFree(&lserror);
                    }
                    free(newVal);
                }
            }

            json_object_put(json);
        }
    }
}

/**
* carrier db watch callback
*
* @param *sh
*   input: LSHandle
*
* @param *message
*   input: LSMessage
*
* @param *ctx
*   input: context
*
* @return bool
*   return always true
*/
bool SmartKeyService::carrierDbWatchCallback (LSHandle *sh, LSMessage *message, void *ctx)
{
    SmartKeyService* service = reinterpret_cast<SmartKeyService*>(ctx);

    if (!message)
        return true;

    const char* payload = LSMessageGetPayload(message);
    if( !payload )
        return true;

//	g_debug("%s: %s", __FUNCTION__, payload);

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        g_warning("Invalid JSON response watching carrierdb");
        return false;
    }

    json_object* value = json_object_object_get(json, "returnValue");
    if (ValidJsonObject(value) && json_object_get_boolean(value))
    {
        json_object* fired = json_object_object_get(json, "fired" );
        if (ValidJsonObject(fired))
        {
            service->queryCarrierDbSettings();
        }
        else
        {
            // watch successful, will be called later if it changed.
        }
    }
    else
    {
        g_debug("carrierdb watch failed.");
    }

    json_object_put(json);

    return true;
}

/**
* cancel carrier db settings watch
*
* @return bool
*   true if done
*/
bool SmartKeyService::cancelCarrierDbSettingsWatch ()
{
    if (!m_carrierDbWatchToken)
        return true;

    LSError lserror;
    LSErrorInit(&lserror);

    bool success = LSCallCancel(m_service, m_carrierDbWatchToken, &lserror);
    if (!success)
    {
        g_warning ("Unable to cancel call with token %lu error message %s", m_carrierDbWatchToken, lserror.message);
        LSErrorFree(&lserror);
    }
    m_carrierDbWatchToken = 0;

    return success;
}

/**
* add carrier db settings watch
*
* @return bool
*   true if done
*/
bool SmartKeyService::addCarrierDbSettingsWatch ()
{
    cancelCarrierDbSettingsWatch();

    LSError lserror;
    LSErrorInit(&lserror);

    bool success = LSCall(m_service,
                          "palm://com.palm.db/watch", "{\"query\":{\"from\":\"com.palm.carrierdb.settings.current:1\"}}",
                          carrierDbWatchCallback, this, &m_carrierDbWatchToken, &lserror);

    if (!success)
    {
        g_critical("Error watching carrierdb: %s", lserror.message);
        LSErrorFree(&lserror);
    }

    return success;
}

/**
* carrier db find callback
*
* @param *sh
*   input: LSHandle
*
* @param *message
*   input: LSMessage
*
* @param *ctx
*   input: context
*
* @return bool
*   return always true
*/
bool SmartKeyService::carrierDbFindCallback (LSHandle *sh, LSMessage *message, void *ctx)
{
    SmartKeyService* service = reinterpret_cast<SmartKeyService*>(ctx);

    if (!message)
        return true;

    const char* payload = LSMessageGetPayload(message);
    if( !payload )
        return true;

//	g_debug("%s: %s", __FUNCTION__, payload);

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        g_warning("Invalid JSON response to carrierdb");
        return false;
    }

    json_object* value = json_object_object_get(json, "returnValue");
    if (!ValidJsonObject(value))
    {
        json_object_put(json);
        g_debug("Call to carrierdb failed");
        return true;
    }

    json_object* results = json_object_object_get(json, "results" );
    if(ValidJsonObject(results))
    {
        array_list* resultArr = json_object_get_array(results);
        if(ValidJsonObject(resultArr))
        {
            json_object* resultObj = static_cast<json_object*>(array_list_get_idx(resultArr, 0));
            if (ValidJsonObject(resultObj))
            {
                json_object* value = json_object_object_get(resultObj, "disableAutotext" );
                if (ValidJsonObject(value) && json_object_get_boolean(value))
                {
                    g_debug("Carrier db specifies to disable text auto-correction");
                    service->disableSpellingAutoCorrection();
                }
                else
                {
                    g_debug("Carrier db not overriding text assist");
                }
            }
            else
            {
                g_debug("No results for carrierdb");
            }
        }
        else
        {
            g_debug("results false in carrierdb");
        }
    }
    else
    {
        g_debug("No results in carrierdb");
    }

    json_object_put(json);

    return true;
}

/**
* Apparently some world-ready customers want text assist auto-correction disabled
* but others don't. The problem is that they all have the same build and only differ
* by their carrier db. So this means that somebody needs to query this db and then set preference
* values to a different default value if a certain carrier db value is present. More info
* at CAS-16172 and NOV-110038 for more info.
*
* @return bool
*   true if done
*/
bool SmartKeyService::queryCarrierDbSettings ()
{
    LSError lserror;
    LSErrorInit(&lserror);

    bool success = LSCallOneReply(m_service,
                                  "palm://com.palm.db/find", "{\"query\":{\"from\":\"com.palm.carrierdb.settings.current:1\"}}",
                                  carrierDbFindCallback, this, NULL, &lserror);

    if (!success)
    {
        g_critical("Error Connecting to carrierdb");
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }
    return success;
}

/**
* Called whenever one of our subscribed preference property changes.
*
* @param *sh
*   input: LSHandle
*
* @param *message
*   input: LSMessage
*
* @param *ctx
*   input: context
*
* @return bool
*   return always true
*/
bool SmartKeyService::queryPreferencesCallback (LSHandle *sh, LSMessage *message, void *ctx)
{
    if (!message)
        return true;

    const char* payload = LSMessageGetPayload(message);
    g_debug("Preferences payload = '%s'", payload);

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        return false;
    }

    Settings* p_settings = Settings::getInstance();
    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);

    LSError error;
    LSErrorInit(&error);

    LanguageAction languageAction = LanguageActionNone;

    json_object* virtualKeyboard = json_object_object_get(json, "x_palm_virtualkeyboard_settings");
    if (ValidJsonObject(virtualKeyboard))
    {

        json_object* payload = json_tokener_parse(json_object_get_string(virtualKeyboard));
        const char* str = 0;

        json_object* label = json_object_object_get(payload, "layout");
        str = json_object_get_string(label);
        if (str)
        {
            p_settings->localeSettings.m_keyboardLayout = str;
            p_settings->localeSettings.m_hasVirtualKeyboard = true;
        }

        label = json_object_object_get(payload, "language");
        str = json_object_get_string(label);
        if (str)
        {
            p_settings->localeSettings.m_inputLanguage = str;
        }

        json_object_put(payload);

        languageAction = LanguageActionKeyboardChanged;

        g_debug("Virtual keyboard layout: '%s', auto-correction: '%s'.", p_settings->localeSettings.m_keyboardLayout.c_str(), p_settings->localeSettings.m_inputLanguage.c_str());
    }

    json_object* localeValue = json_object_object_get(json, "locale");
    if (ValidJsonObject(localeValue))
    {

        json_object* label = json_object_object_get(localeValue, "languageCode");
        if (ValidJsonObject(label))
            p_settings->localeSettings.m_deviceLanguage = json_object_get_string(label);

        label = json_object_object_get(localeValue, "countryCode");
        if (ValidJsonObject(label))
            p_settings->localeSettings.m_deviceCountry = json_object_get_string(label);

        languageAction = LanguageActionLocaleChanged;
    }

    if (languageAction != LanguageActionNone)
    {
        g_debug("SmartKeyService::queryPreferencesCallback: Locale settings: %s", p_settings->localeSettings.getFullLocale().c_str());

        // adjust locale settings for various fallback behaviors...
        LocaleSettings& locale = p_settings->localeSettings;

        if (locale.m_deviceLanguage.size() != 2)
        {
            if (locale.m_inputLanguage.size() == 2)
                locale.m_deviceLanguage = locale.m_inputLanguage;
            else
                locale.m_deviceLanguage = "en";
        }
        if (locale.m_deviceCountry.size() != 2)
            locale.m_deviceCountry = "us";
        if (locale.m_inputLanguage.size() > 2 && locale.m_inputLanguage[2] == '_')
            locale.m_inputLanguage = locale.m_inputLanguage.substr(0, 2);
        else if (locale.m_inputLanguage.size() < 2)
            locale.m_inputLanguage = locale.m_deviceLanguage;
        if (locale.m_keyboardLayout.size() == 0)
            locale.m_keyboardLayout = "qwerty";

        service->m_engine->changedLocaleSettings();
        service->notifyLanguageChanged(languageAction);
    }

    json_object* textInput = json_object_object_get(json, "x_palm_textinput");
    if (ValidJsonObject(textInput))
    {
        service->m_currTextInputPrefs = json_object_get_string(textInput);

        json_object* prop = json_object_object_get(textInput, k_setToCarrierDefaultsPropName);
        if (!ValidJsonObject(prop) || !json_object_get_boolean(prop))
        {
            // Haven't yet set the carrier db defaults (which can override the standard
            // default preferences).
            g_debug("Carrier db never consulted for defaults. Doing so now...");
            #if !defined(TARGET_DESKTOP)
                service->addCarrierDbSettingsWatch();
            #endif
        }
    }
    else
    {
        // This isn't an error and can happen if there is no x_palm_textinput property exists or if it didn't change.
        g_warning("No x_palm_textinput in response.");
    }

    json_object_put(json);

    return true;
}

/**
* Query the system service for our service preferences.
*
* @return bool
*   true if successful
*/
bool SmartKeyService::queryPreferences (void)
{
    LSError error;
    LSErrorInit(&error);

    bool ret = LSCall(m_service,
                      "palm://com.palm.systemservice/getPreferences", "{\"subscribe\":true, \"keys\": [ \"locale\", \"x_palm_textinput\", \"x_palm_virtualkeyboard_settings\" ]}",
                      queryPreferencesCallback, this, NULL, &error);

    if (!ret)
    {
        g_warning("Failed calling palm://com.palm.systemservice/getPreferences: %s", error.message);
        LSErrorFree(&error);
    }

    return ret;
}

/**
* Register with the luna service bus to be notified whenever the system service
* status changes.
*
* @return bool
*   true if successful
*/
bool SmartKeyService::registerForSystemServiceStatus (void)
{
    LSError error;
    LSErrorInit(&error);

    bool ret = LSCall(m_service,"palm://com.palm.bus/signal/registerServerStatus",
                      "{\"serviceName\":\"com.palm.systemservice\", \"subscribe\":true}",
                      systemServiceStatusCallback,this,NULL, &error);

    if (!ret)
    {
        g_warning("Failed to call palm://com.palm.bus/signal/registerServerStatus: %s",
                  error.message);
        LSErrorFree(&error);
    }

    return ret;
}

/**
* Called by luna-service with the system service status changes (connected/disconnected)
*
* @param *sh
*   input: LSHandle
*
* @param *message
*   input: LSMessage
*
* @param *ctx
*   input: context
*
* @return bool
*   return always true
*/
bool SmartKeyService::systemServiceStatusCallback (LSHandle *sh, LSMessage *message, void *ctx)
{
    if (!message)
        return true;

    const char* payload = LSMessageGetPayload(message);

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        return false;
    }

    LSError error;
    LSErrorInit(&error);

    json_object* value = json_object_object_get(json, "connected");
    if (ValidJsonObject(value))
    {
        if (json_object_get_boolean(value) == true)
        {
            reinterpret_cast<SmartKeyService*>(ctx)->queryPreferences();
        }
    }

    json_object_put(json);

    return true;
}

/**
* mojo db service status callback
*
* @param *sh
*   input: LSHandle
*
* @param *message
*   input: LSMessage
*
* @param *ctx
*   input: context
*
* @return bool
*   return always true
*/
bool SmartKeyService::mojoDbServiceStatusCallback (LSHandle *sh, LSMessage *message, void *ctx)
{
    if (!message)
        return true;

    const char* payload = LSMessageGetPayload(message);

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        return false;
    }

    LSError error;
    LSErrorInit(&error);

    json_object* value = json_object_object_get(json, "connected");
    if (ValidJsonObject(value))
    {
        if (json_object_get_boolean(value) && !reinterpret_cast<SmartKeyService*>(ctx)->m_readPeople)
        {
            reinterpret_cast<SmartKeyService*>(ctx)->m_readPeople = true;
            reinterpret_cast<SmartKeyService*>(ctx)->queryPersonsCount();
            reinterpret_cast<SmartKeyService*>(ctx)->queryPersons();
        }
    }

    json_object_put(json);

    return true;
}

/**
* register for mojo db status
*
* @return bool
*   true if succeed
*/
bool SmartKeyService::registerForMojoDbStatus (void)
{
    LSError error;
    LSErrorInit(&error);

    bool ret = LSCall(m_service,"palm://com.palm.bus/signal/registerServerStatus",
                      "{\"serviceName\":\"com.palm.db\", \"subscribe\":true}",
                      mojoDbServiceStatusCallback,this,NULL, &error);

    if (!ret)
    {
        g_warning("Failed to call palm://com.palm.bus/signal/registerServerStatus: %s",
                  error.message);
        LSErrorFree(&error);
    }

    return ret;
}

/**
* is valid name
*
* @param *name
*   name
*
* @return bool
*   true if --> (name length > 2) and first two letters is alphabet and all letters in name are not '#' or '@'
*/
bool SmartKeyService::Name::isValidName (const char *name)
{
    // We add default contacts for "technical support" with names like
    // #DATA, #411, etc. Filter these out.
    if (!(name && ::strlen(name) >= 2 && ::isalpha(name[0]) && ::isalpha(name[1]) && !::strchr(name, '#') && !::strchr(name, '@')))
        return false;

    return true;
}

/**
* add names
*
* @param *str
*   name
*/
void SmartKeyService::Name::addNames (const char *str)
{
    //g_debug("addNames: %s", str.c_str());
    const char * cstr = str;
    while (cstr && *cstr)
    {
        const char * next = ::strpbrk(cstr, " .,:;()/\\\"&?!+*");
        if (next)
        {
            int length = next - cstr;
            if (length >= 2)
            {
                std::string aname;
                aname.assign(cstr, length);
                if (isValidName(aname.c_str()))
                    m_names.push_back(aname);
            }
            cstr = next + 1;	// skip that delimiter
        }
        else
        {
            if (isValidName(cstr))
                m_names.push_back(cstr);
            break;
        }
    }
}

/**
* parse name
*
* @param *objName
*   pointer to json_object
*
* @param name
*   name
*
* @return bool
*   return true always
*/
bool SmartKeyService::parseName (json_object* objName, Name& name )
{
    json_object* value = json_object_object_get(objName, "familyName");
    if (ValidJsonObject(value))
    {
        name.addNames(json_object_get_string(value));
    }

    value = json_object_object_get(objName, "middleName");
    if (ValidJsonObject(value))
    {
        name.addNames(json_object_get_string(value));
    }

    value = json_object_object_get(objName, "givenName");
    if (ValidJsonObject(value))
    {
        name.addNames(json_object_get_string(value));
    }

    return true;
}

/**
* query persons callback
*
* @param *sh
*   input: LSHandle
*
* @param *message
*   input: LSMessage
*
* @param *ctx
*   input: context
*
* @return bool
*   return always true
*/
bool SmartKeyService::queryPersonsCallback (LSHandle *sh, LSMessage *message, void *ctx)
{
    if (!message)
        return true;

    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);
    if (service->m_engine == NULL)
        return true;

    SmkyManufacturerDatabase* db = service->m_engine->getManufacturerDatabase();
    if (db == NULL)
        return true;

    const char* payload = LSMessageGetPayload(message);

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        return false;
    }

    LSError error;
    LSErrorInit(&error);

    json_object* value = json_object_object_get(json, "returnValue");
    if (ValidJsonObject(value))
    {

        if (json_object_get_boolean(value))
        {
            value = json_object_object_get(json, "results");
            if (ValidJsonObject(value))
            {
                array_list* results = json_object_get_array(value);

                int numResults = array_list_length(results);
                g_debug("Received %d contact names", numResults);
                for (int i = 0; i < numResults; i++)
                {
                    json_object* result = static_cast<json_object*>(array_list_get_idx(results, i));
                    if (ValidJsonObject(result))
                    {
                        value = json_object_object_get(result, "names");
                        if (ValidJsonObject(value))
                        {


                            array_list* names = json_object_get_array(value);
                            int numNames = array_list_length(names);
                            for (int n = 0; n < numNames; n++)
                            {
                                json_object* name = static_cast<json_object*>(array_list_get_idx(names, n));

                                Name	parsedName;
                                if (parseName(name, parsedName))
                                {
                                    for (std::vector<std::string>::const_iterator iter = parsedName.m_names.begin(); iter != parsedName.m_names.end(); ++iter)
                                        db->learnWord(*iter);
                                }
                            }
                        }
                    }
                }
            }

            // are there more contacts available?
            value = json_object_object_get(json, "next");
            if (ValidJsonObject(value))
            {
                service->queryPersons(json_object_get_string(value));
            }
        }
    }

    json_object_put(json);

    return true;
}

/**
* query count person callback
*
* @param *sh
*   input: LSHandle
*
* @param *message
*   input: LSMessage
*
* @param *ctx
*   input: context
*
* @return bool
*   return always true
*/
bool SmartKeyService::queryCountPersonCallback (LSHandle *sh, LSMessage *message, void *ctx)
{
    if (!message)
        return true;

    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);
    if (service->m_engine == NULL)
        return true;

    SmkyManufacturerDatabase* db = service->m_engine->getManufacturerDatabase();
    if (db == NULL)
        return true;

    const char* payload = LSMessageGetPayload(message);

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
    {
        return false;
    }
    // {"returnValue":true,"results":[],"count":10949}
    json_object* value = json_object_object_get(json, "returnValue");
    if (ValidJsonObject(value) && json_object_get_boolean(value))
    {
        value = json_object_object_get(json, "count");
        if (ValidJsonObject(value))
        {
            int count = json_object_get_int(value);
            g_debug("SmartKeyService::queryCountPersonCallback: we're expecting %d contacts", count);
            db->setExpectedCount(count);
        }

        // are there more contacts available?
        value = json_object_object_get(json, "next");
        if (ValidJsonObject(value))
        {
            service->queryPersons(json_object_get_string(value));
        }
    }

    json_object_put(json);

    return true;
}

/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_processTaps processTaps

com_palm_smartKey_service/processTaps

Get the spell check candidte from tap squence or trace on virtual keyboard

\subsection com_palm_smartKey_service_syntax Syntax:
\code
{
    "taps": [ int  ]
    "trace": [ int ]
    "shift": string "once" or "lock"
    "first": string
    "last": string
}

\endcode
\param taps the tap sequence data array. Tap consist of 4 int continuesly. If trace is not specified, Tap is required
       Tap data structure in c is
       {
         int x  //x coodinate of tap Optional
         int y  //  y coordinate of tap Optional
         int car   //Unclear. There is no usage of it
         boolean shift  //Unclear. There is no usage of it
       }
\param trace the point trace sequence. If Tap is not specified, trace is Required
\param shft shift keys state. "once" if the shfit is pressed once time. "lock" if shift key is always pressed. Other value means shift key is not pressed. Required if trace is specified
\param first Unclear, There is no useage of it. Required if trace is specified
\param last Unclear, There is no useage of it. Required if trace is specified

\subsection com_palm_smartKey_service_reply Reply:
\code
{
    "traceEntry": boolean
    "guesses":[
        {
            "str": the substitude str
            "auto-replace": boolean
            "auto-accept": boolean
        }
    ]

    "returnValue": boolean
    "errorCode": int
    "errorText": string
}
\endcode
\param traceEntry true if the input param is trace entry. false if the input param is taps. Required
\param guesses
\param returnValue true (success) or false (failure). Required
\param errorCode the error code of error if there is error. Optional
\param errorText the error text of error if there is error. Optional

\subsection com_palm_smartKey_service_examples Examples:
\code
luna-send -n 1 -f palm://com.palm.smartKey/processTaps '{ "taps":[100,630,0,1,335,556,0,1, 230,554,0,1]}'
{
    "returnValue": false,
    "errorCode": 8,
    "errorText": "No matching words"
}

\endcode
*/
bool SmartKeyService::cmdProcessTaps(LSHandle* sh, LSMessage* message, void* ctx)
{
    double start = getTime();

    const char* payload = LSMessageGetPayload(message);
    if (!payload)
        return false;

    //g_debug("%s: received '%s'", __FUNCTION__, payload);

    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);
    SmartKeyErrorCode err = SKERR_SUCCESS;

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
        return false;

    json_object* replyJson = json_object_new_object();

    if (service->isEnabled())
    {

        SpellCheckWordInfo result;
        const int maxGuesses = 10;
        TapDataArray taps;

        json_object* array = 0;
        json_object* shift = 0;
        json_object* first = 0;
        json_object* last = 0;
        if ((array = json_object_object_get(json, "taps")) && json_object_is_type(array, json_type_array))
        {

            json_object_object_add(replyJson, const_cast<char*>("traceEntry"), json_object_new_boolean(false));

            int len = json_object_array_length(array) / 4;	// each tap has 4 elements: x, y, char, shift
            taps.resize(len);	// allocate it all at once, with init
            int i = 0;
            for ( ; i < len; i++)
            {
                TapData & d = taps[i];

                json_object* item = json_object_array_get_idx(array, 4 * i);
                if (!item || !json_object_is_type(item, json_type_int))
                    continue;
                d.x = json_object_get_int(item);

                item = json_object_array_get_idx(array, 4 * i + 1);
                if (!item || !json_object_is_type(item, json_type_int))
                    continue;
                d.y = json_object_get_int(item);

                item = json_object_array_get_idx(array, 4 * i + 2);
                if (!item || !json_object_is_type(item, json_type_int))
                    continue;
                d.car = json_object_get_int(item);

                item = json_object_array_get_idx(array, 4 * i + 3);
                if (!item || !json_object_is_type(item, json_type_boolean))
                    continue;
                d.shifted = json_object_get_boolean(item);
            }
            if (i == len)
                err = service->m_engine->processTaps(taps, result, maxGuesses);
            else
                g_warning("SmartKeyService::cmdProcessTaps: failed to parse taps payload!");
        }
        else if ((array = json_object_object_get(json, "trace")) && json_object_is_type(array, json_type_array)
                 && (shift = json_object_object_get(json, "shift")) && json_object_is_type(shift, json_type_string)
                 && (first = json_object_object_get(json, "first")) && json_object_is_type(first, json_type_string)
                 && (last = json_object_object_get(json, "last")) && json_object_is_type(last, json_type_string))
        {

            json_object_object_add(replyJson, const_cast<char*>("traceEntry"), json_object_new_boolean(true));

            std::vector<unsigned int> points;
            int len = json_object_array_length(array);
            points.reserve(2 * len);	// allocate it all at once, but don't have anything in it yet
            for (int i = 0; i < len; i++)
            {
                json_object* item = json_object_array_get_idx(array, i);
                if (!item || !json_object_is_type(item, json_type_int))
                    break;
                unsigned int coordinates = json_object_get_int(item);
                points.push_back((coordinates >> 16) & 0xffff);	// x top 16 bits
                points.push_back(coordinates & 0xffff);			// y lower 16 bits
            }
            std::string	shiftStr = json_object_get_string(shift);
            EShiftState shiftState;
            if (shiftStr == "once")
                shiftState = eShiftState_once;
            else if (shiftStr == "lock")
                shiftState = eShiftState_lock;
            else
                shiftState = eShiftState_off;
            std::string firstChars = json_object_get_string(first);
            std::string lastChars = json_object_get_string(last);
            //g_message("cmdProcessTaps: processing %u trace points, shift=%s, first=%s, last=%s", points.size() / 2, shiftStr.c_str(), firstChars.c_str(), lastChars.c_str());
            err = service->m_engine->processTrace(points, shiftState, firstChars, lastChars, result, maxGuesses);
        }

        if (err == SKERR_SUCCESS)
        {
            json_object_object_add(replyJson, "spelledCorrectly", json_object_new_boolean(result.inDictionary));

            json_object* guessesJson = json_object_new_array();
            if (guessesJson)
            {
                std::vector<WordGuess>::const_iterator i;
                for (i = result.guesses.begin(); i != result.guesses.end(); ++i)
                {
                    json_object* wordReplyJson = json_object_new_object();
                    if (wordReplyJson)
                    {
                        json_object_object_add(wordReplyJson, "str", json_object_new_string(i->guess.c_str()) );
                        json_object_object_add(wordReplyJson, "sp", json_object_new_boolean(i->spellCorrection) );
                        if (i->autoReplace)
                        {
                            // default assumed to be false so will only set property if not the default
                            json_object_object_add(wordReplyJson, "auto-replace", json_object_new_boolean(i->autoReplace) );
                        }

                        if (i->autoAccept)
                        {
                            // default assumed to be false so will only set property if not the default
                            json_object_object_add(wordReplyJson, "auto-accept", json_object_new_boolean(i->autoAccept) );
                        }

                        json_object_array_add( guessesJson, wordReplyJson );
                    }
                }
                json_object_object_add( replyJson, const_cast<char*>("guesses"), guessesJson );
            }
        }
    }
    else
    {
        err = SKERR_DISABLED;
    }

    LSError lserror;
    LSErrorInit(&lserror);

    setReplyResponse(replyJson, err);

    const char * replyStr = json_object_to_json_string(replyJson);
    //g_debug("SmartKeyService::cmdProcessTaps:\nRequest: %s\nReply: %s", payload, replyStr);

    if (!LSMessageReply(sh, message, replyStr, &lserror))
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }
    json_object_put(replyJson);
    json_object_put(json);

    g_debug("%s took %g msec", __FUNCTION__, (getTime()-start) * 1000.0);

    return true;
}

/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_getCompletion getCompletion

com_palm_smartKey_service/getCompletion



\subsection com_palm_smartKey_service_syntax Syntax:
\code
{
    "prefix": string
}

\endcode
\param prefix The prefix to try to complete Required

\subsection com_palm_smartKey_service_reply Reply:
\code
{
    "com": string
    "exact": boolean
    "returnValue": boolean
    "errorCode": int
    "errorText": string
}
\endcode
\param com The completed word based on the prefix. Required
\param exact true if com is empty
\param returnValue true (success) or false (failure). Required
\param errorCode the error code of error if there is error. Optional
\param errorText the error text of error if there is error. Optional

\subsection com_palm_smartKey_service_examples Examples:
\code
luna-send -n 1 -f palm://com.palm.smartKey/getCompletion '{"prefix":"pre"}'
{
    "comp": "",
    "exact": true,
    "returnValue": true
}
\endcode
*/
bool SmartKeyService::cmdGetCompletion(LSHandle* sh, LSMessage* message, void* ctx)
{
    double start = getTime();

    const char* payload = LSMessageGetPayload(message);
    if (!payload)
        return false;

    g_debug("%s: received '%s'", __FUNCTION__, payload);

    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);
    SmartKeyErrorCode err = SKERR_SUCCESS;

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
        return false;

    json_object* replyJson = json_object_new_object();

    if (service->isEnabled())
    {

        std::string prefix, result;
        json_object* prop = json_object_object_get(json, "prefix");
        if (prop && json_object_is_type(prop, json_type_string))
        {
            prefix = json_object_get_string(prop);
        }

        err = service->m_engine->getCompletion(prefix, result);

        if (err == SKERR_SUCCESS)
        {

            json_object_object_add(replyJson, "comp", json_object_new_string(result.c_str()));
            json_object_object_add(replyJson, "exact", json_object_new_boolean(result.empty()));
        }
    }
    else
    {
        err = SKERR_DISABLED;
    }

    LSError lserror;
    LSErrorInit(&lserror);

    setReplyResponse(replyJson, err);

    if (!LSMessageReply(sh, message, json_object_to_json_string(replyJson), &lserror))
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }
    json_object_put(replyJson);
    json_object_put(json);

    g_debug("%s took %g msec", __FUNCTION__, (getTime()-start) * 1000.0);

    return true;
}

/*! \page  com_palm_smartKey_service
\n
\section  com_palm_smartKey_updateWordUsage updateWordUsage

com_palm_smartKey_service/updateWordUsage

Updates usage statistics of a word. It will add it to the database if the word is not already known.

\subsection com_palm_smartKey_service_syntax Syntax:
\code
{
    "word":string
}
\endcode

\param word the word to be updated. Required

\subsection com_palm_smartKey_service_reply Reply:
\code
{
    "returnValue": boolean
    "errorCode": int
    "errorText": string
}
\endcode
\param returnValue true (success) or false (failure). Required
\param errorCode the error code of error if there is error. Optional
\param errorText the error text of error if there is error. Optional

\subsection com_palm_smartKey_service_examples Examples:
\code
luna-send -n 1 -f palm://com.palm.smartKey/updateWordUsage '{"word":"oulu"}'
{
    "returnValue": true
}
\endcode
*/
bool SmartKeyService::cmdUpdateWordUsage(LSHandle* sh, LSMessage* message, void* ctx)
{
    double start = getTime();

    const char* payload = LSMessageGetPayload(message);
    if (!payload)
        return false;

    g_debug("%s: received '%s'", __FUNCTION__, payload);

    SmartKeyService* service = static_cast<SmartKeyService*>(ctx);
    SmartKeyErrorCode err = SKERR_SUCCESS;

    json_object* json = json_tokener_parse(payload);
    if (!ValidJsonObject(json))
        return false;

    json_object* replyJson = json_object_new_object();

    if (service->isEnabled())
    {

        std::string word;
        json_object* prop = json_object_object_get(json, "word");
        if (prop && json_object_is_type(prop, json_type_string))
        {
            word = json_object_get_string(prop);
        }

        err = service->m_engine->getUserDatabase()->updateWordUsage(word);
    }
    else
    {
        err = SKERR_DISABLED;
    }

    LSError lserror;
    LSErrorInit(&lserror);

    setReplyResponse(replyJson, err);

    if (!LSMessageReply(sh, message, json_object_to_json_string(replyJson), &lserror))
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
    }
    json_object_put(replyJson);
    json_object_put(json);

    g_debug("%s took %g msec", __FUNCTION__, (getTime()-start) * 1000.0);

    return true;
}

/**
* query persons
*
* @param page
*   page
*
* @return bool
*   true if done
*/
bool SmartKeyService::queryPersons (const std::string& page)
{
    LSError error;
    LSErrorInit(&error);

    g_message("Reading contacts for page '%s'", page.c_str());
    auto_g_free_array<gchar> pageStr;
    if (!page.empty())
    {
        pageStr = g_strdup_printf(",\"page\":\"%s\"", page.c_str());
    }
    auto_g_free_array<gchar> payload = g_strdup_printf("{\"query\":{\"from\":\"com.palm.person:1\",\"select\":[\"names.givenName\",\"names.middleName\",\"names.familyName\"]%s}}", pageStr ? pageStr.p : "");
    bool ret = LSCall(m_service, "luna://com.palm.db/find", payload, queryPersonsCallback, this, NULL, &error);

    if (!ret)
    {
        g_warning("Failed querying contacts: %s", error.message);
        LSErrorFree(&error);
    }

    return ret;
}

/**
* query persons count
*
* @return bool
*   true if done
*/
bool SmartKeyService::queryPersonsCount ()
{
    LSError error;
    LSErrorInit(&error);

    bool ret = LSCall(m_service, "luna://com.palm.db/find", "{\"query\":{\"from\":\"com.palm.person:1\",\"limit\":0},\"count\":true}", queryCountPersonCallback, this, NULL, &error);
    if (!ret)
    {
        g_warning("Failed querying contacts count: %s", error.message);
        LSErrorFree(&error);
    }

    return ret;
}

/**
* enable service
*
* @param on
*   true or false
*/
void SmartKeyService::enable (bool on)
{
    m_isEnabled = on;

    g_message("%s: SmartKeyService is %s", __FUNCTION__, m_isEnabled ? "enabled" : "disabled");
}

/**
* is service enabled?
*
* @return bool
*   true if enabled
*/
bool SmartKeyService::isEnabled()
{
    return m_isEnabled;
}

/**
* Return the current UNIX epoch time.
*
* @return double
*   time value
*/
double SmartKeyService::getTime (void)
{
    struct timespec curTime;
    clock_gettime(CLOCK_MONOTONIC, &curTime);

    return static_cast<double>(curTime.tv_sec) +
           static_cast<double>(curTime.tv_nsec) / 1000000000.0f;
}

}


