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
/*
 * SmartKeyService
 *
 *
 */


#ifndef SMARTKEY_SERVICE_H
#define SMARTKEY_SERVICE_H

#include "lunaservice.h"

#include <string>
#include <vector>

#include "Settings.h"
#include "SpellCheckEngine.h"
#include "StringUtils.h"

namespace SmartKey
{

const size_t InvalidFileSize = -1;

class SmartKeyService
{
public:
    SmartKeyService(const Settings& settings);
    ~SmartKeyService();


    static bool cmdSearch(LSHandle* sh, LSMessage* message, void* ctx);
    static bool cmdAddUserWord(LSHandle* sh, LSMessage* message, void* ctx);
    static bool cmdRemoveUserWord(LSHandle* sh, LSMessage* message, void* ctx);
    static bool cmdExit(LSHandle* sh, LSMessage* message, void* ctx);
    static bool cmdListUserWords(LSHandle* sh, LSMessage* message, void* ctx);
    static bool cmdNumUserWords(LSHandle* sh, LSMessage* message, void* ctx);
    static bool cmdAddAutoReplace(LSHandle* sh, LSMessage* message, void* ctx);
    static bool cmdRemoveAutoReplace(LSHandle* sh, LSMessage* message, void* ctx);
    static bool cmdNumAutoReplace(LSHandle* sh, LSMessage* message, void* ctx);
    static bool cmdListAutoReplace(LSHandle* sh, LSMessage* message, void* ctx);
    static bool cmdAddPerson(LSHandle* sh, LSMessage* message, void* ctx);
    static bool cmdRemovePerson(LSHandle* sh, LSMessage* message, void* ctx);
    static bool cmdProcessTaps(LSHandle* sh, LSMessage* message, void* ctx);
    static bool cmdGetCompletion(LSHandle* sh, LSMessage* message, void* ctx);
    static bool cmdUpdateWordUsage(LSHandle* sh, LSMessage* message, void* ctx);

    bool start(GMainLoop* mainLoop, const char* name);
    bool stop();

    bool isEnabled();
    void enable(bool enabled);

    bool registerForSystemServiceStatus();
	bool registerForMojoDbStatus();

	void getAutoReplaceStats(const std::string& fname) const;

    bool validateDataSources();
	static bool fileExists(const std::string& fname);
	static size_t fileSize(const std::string& fname);
	static std::string dirName(const std::string& path);
	static double getTime();
	static bool copyFileFromDirToDir(const std::string& fname, const std::string& srcDir, const std::string& dstDir);

private:

	enum DbAction {
		AddedToDatabase,
		RemovedFromDatabase
	};

    enum LanguageAction {
        LanguageActionNone,
        LanguageActionKeyboardChanged,
        LanguageActionLocaleChanged
    };

	struct Name {

		std::vector<std::string> m_names;

		void addNames(const char * str);	// might be a name, a series of names, with need to sanitize: ex: "de Marcelier de Gaujac", "Dr. Seuss"

		static bool isValidName(const char * name);
	};

    bool restoreDefaultDataFromBackup();
	bool stageUserData();
	bool queryPreferences();
	bool queryPersons(const std::string& page = "");
	bool queryPersonsCount();
	bool notifyUserDbChange(DbAction eAction, const std::string& word);
	bool notifyVolatileDbChange(DbAction eAction, const std::string& word);
	bool notifyAutoReplaceDbChange(DbAction eAction, const AutoSubDatabase::Entry& entry);
    bool notifyLanguageChanged(LanguageAction eAction);
	void disableSpellingAutoCorrection();
	bool queryCarrierDbSettings();
	bool cancelCarrierDbSettingsWatch();
	bool addCarrierDbSettingsWatch();
	
	static bool copyFile(const std::string& src, const std::string& dst);
    static bool systemServiceStatusCallback(LSHandle *sh, LSMessage *message, void *ctx);
	static bool queryPreferencesCallback(LSHandle *sh, LSMessage *message, void *ctx);
	static bool carrierDbFindCallback(LSHandle *sh, LSMessage *message, void *ctx);
	static bool carrierDbWatchCallback(LSHandle *sh, LSMessage *message, void *ctx);
	static bool mojoDbServiceStatusCallback(LSHandle *sh, LSMessage *message, void *ctx);
	static bool queryPersonsCallback(LSHandle *sh, LSMessage *message, void *ctx);
	static bool queryCountPersonCallback(LSHandle *sh, LSMessage *message, void *ctx);
	static bool setPrefsCallback(LSHandle *sh, LSMessage *message, void *ctx);
	static bool wordIsUrl(const std::string& word);
	static bool wordIsAllPunctuation(const std::string& word);
	static bool parseName(struct json_object* objName, Name& name );
	static std::string stripPunctuation(const std::string& word, std::string& leadingChars, std::string& trailingChars);
	static std::string restorePunctuation(const std::string& word, const std::string& leadingChars, const std::string& trailingChars);

    LSHandle* m_service;
	LSMessageToken m_carrierDbWatchToken;
    GMainLoop* m_mainLoop;
	SpellCheckEngine*	m_engine;
    Settings m_settings;
    bool m_isEnabled;
	bool m_readPeople;	///< Have all people (AKA contacts) been read yet?
	std::string m_currTextInputPrefs;
};

}

#endif  // SMARTKEY_SERVICE_H
