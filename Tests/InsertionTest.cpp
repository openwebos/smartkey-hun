/**
 *  Copyright (c) 2010 - 2012 Hewlett-Packard Development Company, L.P.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <string.h>
#include <iostream>
#include <google/heap-profiler.h>
#include <google/profiler.h>

#include "SmartKeyDictionary.h"
#include "Settings.h"

Settings g_settings;

bool test(SmartKey::SmartKeyDictionary& d, const char *prefix, const char* expectedMatch)
{
    std::string query(prefix);
    std::string expected(expectedMatch);
    
    std::string found = d.findBestMatch(prefix);
    if (found != expected) {
        printf("%s: FAILED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!, query: '%s', result: '%s' != '%s' \n",
                __FUNCTION__, prefix, found.c_str(), expectedMatch);
        return false;
    } else {
        return true;
    }
}


void allWordsTest(SmartKey::SmartKeyDictionary& d) {
    test(d, "theres", "there's");
    test(d, "fot", "for");

    test(d, "therr", "there");
    test(d, "rhsy", "that");
    test(d, "that", "that");
    test(d, "yall", "y'all");
    test(d, "thats", "that's");


    test(d, "ot", "it");
    test(d, "dindrsmrusl", "substantial");
    test(d, "subs", "subs");
    test(d, "substanti", "substantial");
    test(d, "I", "I");
    test(d, "im", "I'm");
    test(d, "I'm", "I'm");
    test(d, "fyts", "guys");
    test(d, "thisisnotinthedict", "");
}

void testExactMatchesInFile(SmartKey::SmartKeyDictionary& d, const char* filename)
{
    // since we treat some characters as "invisible" like the apostrophe,
    // there are cases when the possessive form is more common than the plural,
    // like "subs" will return "sub's"

    FILE* f = fopen(filename,"rb");

    if (!f) {
        printf("%s: unable to open file: '%s'\n", __FUNCTION__, filename);
        return;
    }

    const int kMaxLineBuffer = 128;
    char linebuffer[kMaxLineBuffer+1];

    int words = 0;
    int failures = 0;

    // all characters in set are skipped
    while (fgets(linebuffer, kMaxLineBuffer, f))
    {
        char* word = strtok(linebuffer," " );
        if (word) {
            if (!test(d, word, word)) {
                failures++;
            }
            words++;
        }
    }

    printf("%s: number of failed lookups: %d\n", __FUNCTION__, failures);

    fclose(f);

}

void memMappedTest()
{

    SmartKey::SmartKeyDictionary* dict = new SmartKeyDictionary(g_settings);

    dict->loadSkippedCharsFromFile(g_settings.skippedCharsFile);
    dict->initMemmaps(g_settings.mappedNodesFile, g_settings.mappedWordsFile,
    		g_settings.mappedScoresFile, g_settings.mappedUserFrequenciesFile);
    dict->loadEquivalencesFromFile(g_settings.keyEquivalencesFile);

    allWordsTest(*dict);
}

void memTest() {
    SmartKey::SmartKeyDictionary* dict = new SmartKeyDictionary(g_settings);

    HeapProfilerStart("smartkey");
    HeapProfilerDump("period");
    dict->loadSkippedCharsFromFile("/var/tmp/key_skipped");
    dict->loadCompiledDictionary("/var/tmp/compiled_key_dict_us");
    dict->loadEquivalencesFromFile("/var/tmp/key_equiv");
    HeapProfilerDump("period");

    test(*dict, "ofrss", "ideas");

    delete dict;
    HeapProfilerDump("period");
}

void bigDictSmallSearchTest() {
    SmartKey::SmartKeyDictionary dictionary(g_settings);

    dictionary.loadSkippedCharsFromFile("/var/tmp/key_skipped");
    dictionary.loadTermsFromFile("/var/tmp/key_dict_us");
    dictionary.loadEquivalencesFromFile("/var/tmp/key_equiv");

    test(dictionary, "ofrss", "ideas");
}

void bigDictTest() {
    SmartKey::SmartKeyDictionary dictionary(g_settings);

#ifdef HEAP
    HeapProfilerStart("smartkey");
    HeapProfilerDump("period");
#endif

    dictionary.loadSkippedCharsFromFile("/var/tmp/key_skipped");

#ifdef HEAP
    HeapProfilerDump("period");
#endif

    dictionary.loadTermsFromFile("/var/tmp/key_dict_us");
    //dictionary.loadCompiledDictionary("/var/tmp/compiled_key_dict_us");

    //HeapProfilerDump("period");

    dictionary.loadEquivalencesFromFile("/var/tmp/key_equiv");

    //testExactMatchesInFile(dictionary, "/var/tmp/key_dict_us");


     // old test
    /*
    int numInserts = 0;
    dictionary.insert("dork", 69); numInserts++;
    dictionary.insert("door", 67); numInserts++;
    dictionary.insert("doored", 39); numInserts++;
    dictionary.insert("flake", 70); numInserts++;
    dictionary.insert("the", 90); numInserts++;
    dictionary.insert("that", 90); numInserts++;
    dictionary.insert("they", 80); numInserts++;
    dictionary.insert("there", 82); numInserts++;
    dictionary.insert("There's", 81); numInserts++;
    dictionary.insert("at", 90); numInserts++;
    dictionary.insert("it", 91); numInserts++;
    dictionary.insert("for", 69); numInserts++;
    dictionary.insert("attempt", 30); numInserts++;
    dictionary.insert("substantial", 43); numInserts++;
    dictionary.insert("subs", 2); numInserts++;
    dictionary.insert("nonsense", 4); numInserts++;

    dictionary.print();
*/




    allWordsTest(dictionary);

    // small tree insertion test


/*
    std::string there = "there";

    std::string theres = "theres";
    
    std::string therr = "therr";
    
    std::string rhsy = "rhsy";
    
    std::string that = "that";
    std::string that_rgsr = "rgsy";

    dictionary.find(there);
    dictionary.find(therr);
    
    dictionary.find(that);
    dictionary.find(that_rgsr);

    dictionary.find(theres);

    dictionary.find(std::string("fot"));
    dictionary.find(std::string("ot"));
    dictionary.find(std::string("dindrsmrusl"));
  */




/*
    //tiny test
    printf("---- inserting ------\n");
    dictionary.insert("that", 80);
    dictionary.print();
    printf("---- inserting ------\n");
    dictionary.insert("there", 90);
    dictionary.print();
    printf("---- inserting ------\n");
    dictionary.insert("they", 70);
    dictionary.print();
    printf("---- inserting ------\n");
    dictionary.insert("that's", 70);
    dictionary.print();
*/
    /* weird ordering issue
    dictionary.insert("theater", 2);
    dictionary.insert("theaters", 3);
    dictionary.insert("there", 2);
    dictionary.insert("the", 1);
    dictionary.insert("Theme", 4);
    dictionary.insert("Thematic", 1);
    dictionary.insert("Th", 10);
    dictionary.insert("those", 1);
     */

    //dictionary.print();
    //test(dictionary, "there", "there");
#ifdef HEAP
    ProfilerStart("smartkeyCpu");
#endif

    /*
    test(dictionary, "tha", "that");
    test(dictionary, "rgru", "they");
    test(dictionary, "subs", "sub's");
    test(dictionary, "substanti", "substantial");
    test(dictionary, "mimdrbdr", "nonsense");
*/
#ifdef HEAP
    ProfilerStop();
#endif

    //dictionary.insert("the", 100000);



    int size = 0;
    int numOfSize = -1;

    printf("%s: number of nodes: %d\n", __FUNCTION__, dictionary.getNodeCount());

    while (numOfSize != 0) {
        int capacity = 0;
        numOfSize = dictionary.singlesCounter(size, capacity);
        printf("%s: number of child maps with only %d child: %d\n", __FUNCTION__, size, numOfSize);
        size++;
    }

    printf("%s: num removable nodes: %d\n", __FUNCTION__, dictionary.numRemovableNodes());



    printf("%s: node size: %d\n", __FUNCTION__,
            sizeof(SmartKeyNode));
}

void apostropheTest()
{
    SmartKey::SmartKeyDictionary dictionary(g_settings);

     dictionary.loadSkippedCharsFromFile("/var/tmp/key_skipped");
     dictionary.loadEquivalencesFromFile("/var/tmp/key_equiv");
     dictionary.loadTermsFromFile("/var/tmp/key_dict_us");
     //dictionary.insert("talk", 413);
     /*
     dictionary.insert("y'all", 3);
     dictionary.insert("yskk", 4);
     dictionary.insert("y'akk", 6);
     dictionary.insert("y'dork", 5);

     dictionary.print();
      */
     printf("%s: number of nodes: %d\n", __FUNCTION__, dictionary.getNodeCount());

     /*
     dictionary.insert("thats", 10);
     dictionary.insert("that's", 11);
     dictionary.insert("that''s", 12);
*/
     ///test(dictionary, "thats", "that's");
     test(dictionary, "yall", "y'all");


}

int main (int argc, char * const argv[]) {

	const std::string settingsFile("/etc/palm/smartkey.conf");

	if (!g_settings.load(settingsFile)) {
		g_warning("Error loading settings from '%s'", settingsFile.c_str());
	}

    memMappedTest();

    //bigDictTest();

    //bigDictSmallSearchTest();

    //memTest();

    //apostropheTest();

    return 0;
}
