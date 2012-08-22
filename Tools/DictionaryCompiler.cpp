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

#include <string.h>
#include <stdio.h>
#include <glib.h>

#include "SmartKeyDictionary.h"
#include "SmartKeyHashDictionary.h"

#include "Settings.h"

/*
static gchar* skippedCharsFilename = ;
static gint max_size = 8;
static gboolean verbose = FALSE;
static gboolean beep = FALSE;
static gboolean rand = FALSE;


static GOptionEntry entries[] =
{
  { "repeats", 'r', 0, G_OPTION_ARG_INT, &repeats, "Average over N repetitions", "N" },
  { "max-size", 'm', 0, G_OPTION_ARG_INT, &max_size, "Test up to 2^M items", "M" },
  { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Be verbose", NULL },
  { "beep", 'b', 0, G_OPTION_ARG_NONE, &beep, "Beep when done", NULL },
  { "rand", 0, 0, G_OPTION_ARG_NONE, &rand, "Randomize the data", NULL },
  { NULL }
};

*/

void smallTest()
{
	/*
    SmartKey::SmartKeyDictionary dict;

    dict.loadSkippedCharsFromFile("/var/tmp/key_skipped");
    //dict.loadTermsFromFile("/var/tmp/key_dict_us_the");
    dict.loadTermsFromFile("/var/tmp/key_dict_us_thermo2");
    dict.loadEquivalencesFromFile("/var/tmp/key_equiv");

    printf("---- dict read from ascii data ----\n");
    dict.print();

    const char* filename = "/tmp/node_content";

    dict.writeToFile(filename, true);

    SmartKey::SmartKeyDictionary loadedDict;

    loadedDict.loadCompiledDictionary(filename);
    printf("---- dict read from bin data ----\n");
    loadedDict.print();

    const char* compareFilename = "/tmp/node_content_compare";
    loadedDict.writeToFile(compareFilename, true);

    printf("---- wrote to file read dict: %s ----\n", compareFilename);
    SmartKey::SmartKeyDictionary reloadedDict;
    reloadedDict.loadCompiledDictionary(compareFilename);
    printf("---- dict read from bin data ----\n");
    reloadedDict.print();
    */

}

void buildDefaultDict(Settings& compilerSettings)
{
    SmartKey::SmartKeyDictionary dict(compilerSettings);

    printf("%s: skipped chars file: '%s'\n", __FUNCTION__, compilerSettings.skippedCharsFile.c_str());
    dict.loadSkippedCharsFromFile(compilerSettings.skippedCharsFile.c_str());
    printf("%s: terms file: '%s'\n", __FUNCTION__, compilerSettings.termsFile.c_str());
    dict.loadTermsFromFile(compilerSettings.termsFile.c_str());

    dict.loadEquivalencesFromFile(compilerSettings.keyEquivalencesFile);

    printf("%s: num nodes loaded from terms file: %d\n", __FUNCTION__, dict.getNodeCount());
    printf("%s: num words loaded from terms file: %d\n", __FUNCTION__, dict.getWordCount());

    // writing out memmaps
    bool didWrite = dict.writeMemmaps(compilerSettings.mappedNodesFile,
    		compilerSettings.mappedWordsFile,
    		compilerSettings.mappedScoresFile,
    		compilerSettings.mappedFreqsFile);

    printf("writing memmaped files: %s\n", didWrite ? "succeeded" : "failed");

    if (didWrite) {
    	printf("files generated:\n%s\n%s\n%s\n%s\n", compilerSettings.mappedNodesFile.c_str(),
    			compilerSettings.mappedWordsFile.c_str(),
    			compilerSettings.mappedScoresFile.c_str(),
    			compilerSettings.mappedFreqsFile.c_str());
    }

    if (!didWrite) {
    	exit(-1);
    }

    /*
	const std::string testSettingsFile("/etc/palm/smartkey.conf");
	Settings testSettings;
	if (!testSettings.load(testSettingsFile)) {
		g_warning("Error loading settings from '%s'", testSettingsFile.c_str());
	}

*/
    SmartKey::SmartKeyDictionary memMappedDict(compilerSettings);
    memMappedDict.initMemmaps(compilerSettings.mappedNodesFile,
    		compilerSettings.mappedWordsFile,
    		compilerSettings.mappedScoresFile,
    		compilerSettings.mappedFreqsFile);

    printf("%s: finished...\n", __FUNCTION__);
}

void buildAutoreplaceDict(Settings& s)
{
    SmartKey::SmartKeyHashDictionary dict(s);

    bool didWrite = dict.writeAutoreplaceData(s.autoreplaceDefaultFile,
    		s.autoreplaceTermsFile,
    		s.autoreplaceMappedWordsFile,
    		s.autoreplaceMappedNodesFile,
    		s.autoreplaceMappedScoresFile,
    		s.autoreplaceMappedFreqsFile);

    printf("num words loaded from terms file: %d\n", dict.getWordCount());

    printf("writing memmaped files: %s\n", didWrite ? "succeeded" : "failed");

    if (didWrite) {
    	printf("files generated:\n%s\n%s\n%s\n%s\n", s.autoreplaceMappedNodesFile.c_str(),
        		s.autoreplaceMappedWordsFile.c_str(),
        		s.autoreplaceMappedScoresFile.c_str(),
        		s.autoreplaceMappedFreqsFile.c_str());
    }

    if (!didWrite) {
    	exit(-1);
    }

	const std::string testSettingsFile("/etc/palm/smartkey.conf");
	Settings testSettings;
	if (!testSettings.load(testSettingsFile)) {
		g_warning("Error loading settings from '%s'", testSettingsFile.c_str());
	}

	// perform basic sanity check by loading the memmaped dictionary

    SmartKey::SmartKeyHashDictionary memMappedDict(s);
    memMappedDict.loadTermsFromFile(s.autoreplaceTermsFile);
    memMappedDict.initMemmaps(s.autoreplaceMappedNodesFile,
    		s.autoreplaceMappedWordsFile,
    		s.autoreplaceMappedScoresFile,
    		s.autoreplaceMappedFreqsFile);

//    memMappedDict.print();
}

int main (int argc, char* const argv[])
{

	const std::string compilerSettingsFile("/etc/palm/smartkey_compiler.conf");
	Settings compilerSettings;
	if (!compilerSettings.load(compilerSettingsFile)) {
		g_warning("Error loading settings from '%s'", compilerSettingsFile.c_str());
	}

    /*
    memMappedDict.initMemmaps("/var/tmp/memmaps_nodes",
    		"/var/tmp/memmaps_words",
    		"/var/tmp/memmaps_scores");
*/
    //memMappedDict.print();

	//buildAutoreplaceDict(compilerSettings);

	buildDefaultDict(compilerSettings);

    printf("finished loading memmapped dict\n");

    return 0;
}
