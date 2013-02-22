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

#ifndef SMARTKEY_SERVICE_H
#define SMARTKEY_SERVICE_H

#include "lunaservice.h"

#include <string>
#include <vector>

#include "Settings.h"
#include "SmkySpellCheckEngine.h"
#include "StringUtils.h"

namespace SmartKey
{

const size_t InvalidFileSize = -1;

class SmartKeyService
{
private:
    LSHandle* m_service;
    LSMessageToken m_carrierDbWatchToken;
    GMainLoop* m_mainLoop;
    SmkySpellCheckEngine* m_engine;
    bool m_isEnabled;
    bool m_readPeople; ///< Have all people (AKA contacts) been read yet?
    std::string m_currTextInputPrefs;

public:
    SmartKeyService(void);
    ~SmartKeyService(void);

    //search
    static bool cmdSearch(LSHandle* sh, LSMessage* message, void* ctx);

    //add a new word to user dictionary
    static bool cmdAddUserWord(LSHandle* sh, LSMessage* message, void* ctx);

    //remove word from user dictionary
    static bool cmdRemoveUserWord(LSHandle* sh, LSMessage* message, void* ctx);

    //close this service
    static bool cmdExit(LSHandle* sh, LSMessage* message, void* ctx);

    //list all words from the user dictionary
    static bool cmdListUserWords(LSHandle* sh, LSMessage* message, void* ctx);

    //get number of words from user dictionary
    static bool cmdNumUserWords(LSHandle* sh, LSMessage* message, void* ctx);

    //add a new word to auto substitution dictionary
    static bool cmdAddAutoReplace(LSHandle* sh, LSMessage* message, void* ctx);

    //remove word from auto substitution dictionary
    static bool cmdRemoveAutoReplace(LSHandle* sh, LSMessage* message, void* ctx);

    //get number of words from auto substitution dictionary
    static bool cmdNumAutoReplace(LSHandle* sh, LSMessage* message, void* ctx);

    //list all words from the auto substitution dictionary
    static bool cmdListAutoReplace(LSHandle* sh, LSMessage* message, void* ctx);

    //add a new word to context dictionary
    static bool cmdAddPerson(LSHandle* sh, LSMessage* message, void* ctx);

    //remove word from context dictionary
    static bool cmdRemovePerson(LSHandle* sh, LSMessage* message, void* ctx);

    //taps -- not used
    static bool cmdProcessTaps(LSHandle* sh, LSMessage* message, void* ctx);

    //get completion
    static bool cmdGetCompletion(LSHandle* sh, LSMessage* message, void* ctx);

    // -- not used
    static bool cmdUpdateWordUsage(LSHandle* sh, LSMessage* message, void* ctx);

    //start service
    bool start(GMainLoop* mainLoop, const char* name);

    //stop service
    bool stop();

    //is service enabled ?
    bool isEnabled();

    //enable/disable service
    void enable(bool enabled);

    // --
    bool registerForSystemServiceStatus();

    // --
    bool registerForMojoDbStatus();

    // -- not used
    void getAutoReplaceStats(const std::string& fname) const;

    // --
    bool validateDataSources();

    // --
    static bool fileExists(const std::string& fname);

    // --
    static size_t fileSize(const std::string& fname);

    // --
    static std::string dirName(const std::string& path);

    // --
    static double getTime(void);

    // --
    static bool copyFileFromDirToDir(const std::string& fname, const std::string& srcDir, const std::string& dstDir);

private:

    enum DbAction
    {
        AddedToDatabase,
        RemovedFromDatabase
    };

    enum LanguageAction
    {
        LanguageActionNone,
        LanguageActionKeyboardChanged,
        LanguageActionLocaleChanged
    };

    struct Name
    {

        std::vector<std::string> m_names;

        void addNames(const char * str);	// might be a name, a series of names, with need to sanitize: ex: "de Marcelier de Gaujac", "Dr. Seuss"

        static bool isValidName(const char * name);
    };

    //restore default data from backup
    bool restoreDefaultDataFromBackup (void);

    //stage user data
    bool stageUserData (void);

    //query preferences
    bool queryPreferences (void);

    //query persons
    bool queryPersons (const std::string& page = "");

    //query persons count
    bool queryPersonsCount (void);

    //notify user db change
    bool notifyUserDbChange (DbAction eAction, const std::string& word);

    //notify volatile db change
    bool notifyVolatileDbChange (DbAction eAction, const std::string& word);

    //notify auto replace db change
    bool notifyAutoReplaceDbChange (DbAction eAction, const Entry& entry);

    //notify language changed
    bool notifyLanguageChanged (LanguageAction eAction);

    //disable spelling auto correction
    void disableSpellingAutoCorrection (void);

    //query carrier db settings
    bool queryCarrierDbSettings (void);

    //cancel carrier db settings watch
    bool cancelCarrierDbSettingsWatch (void);

    //add carrier db settings watch
    bool addCarrierDbSettingsWatch (void);

    //copy file
    static bool copyFile (const std::string& src, const std::string& dst);

    //system service status callback
    static bool systemServiceStatusCallback (LSHandle *sh, LSMessage *message, void *ctx);

    //query preferences callback
    static bool queryPreferencesCallback (LSHandle *sh, LSMessage *message, void *ctx);

    //carrier db find callback
    static bool carrierDbFindCallback (LSHandle *sh, LSMessage *message, void *ctx);

    //carrier db find callback
    static bool carrierDbWatchCallback (LSHandle *sh, LSMessage *message, void *ctx);

    //mojoDb service status callback
    static bool mojoDbServiceStatusCallback (LSHandle *sh, LSMessage *message, void *ctx);

    //query persons callback
    static bool queryPersonsCallback (LSHandle *sh, LSMessage *message, void *ctx);

    //query count person callback
    static bool queryCountPersonCallback (LSHandle *sh, LSMessage *message, void *ctx);

    //set prefs callback
    static bool setPrefsCallback (LSHandle *sh, LSMessage *message, void *ctx);

    //is word Url?
    static bool wordIsUrl (const std::string& word);

    //is all word letters punctuation ?
    static bool wordIsAllPunctuation (const std::string& word);

    //parse name
    static bool parseName (struct json_object* objName, Name& name );

    //strip punctuation
    static std::string stripPunctuation (const std::string& word, std::string& leadingChars, std::string& trailingChars);

    //restore punctuation
    static std::string restorePunctuation (const std::string& word, const std::string& leadingChars, const std::string& trailingChars);
};

}

#endif  // SMARTKEY_SERVICE_H
