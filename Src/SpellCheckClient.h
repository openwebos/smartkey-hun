/* @@@LICENSE
*
*      Copyright (c) 2010-2013 LG Electronics, Inc.
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


#ifndef SPELL_CHECK_CLIENT_H
#define SPELL_CHECK_CLIENT_H

#include <string>
#include <vector>
#include <lunaservice.h>

namespace SmartKey
{

/**
 * Contains information about the correctl spelling guess for a word.
 */
struct WordGuess
{
    WordGuess() {};

    WordGuess(const std::string& guess_) :
        guess(guess_)
        , spellCorrection(false)
        , autoReplace(false)
        , autoAccept(false) {};

    std::string	guess;      ///< The actual guess of the word.
    bool spellCorrection;   ///< Guess is a result of a spelling correction?
    bool autoReplace;       ///< Guess is a result of a auto-replace match?
    bool autoAccept;        ///< Engine is recommending that we auto accept this guess.
};

/**
 * Contains the spell check information for a single word.
 */
struct SpellCheckWordInfo
{
    SpellCheckWordInfo() : inDictionary(true) {}
    bool isEmpty() const
    {
        return guesses.empty();
    }
    void clear()
    {
        inDictionary = true;
        guesses.clear();
    }

    bool inDictionary;  ///< Was this word in any dictionary.
    std::vector<WordGuess> guesses; ///< Collection of guesses (may be empty - even if mispelled.)
};

/**
 * Contains tap information.
 */
struct TapData
{
    TapData() : x(0), y(0), car(0), shifted(false) {}

    unsigned int x;
    unsigned int y;
    unsigned int car;
    bool shifted;
};

class TapDataArray : public std::vector<TapData> {};


/**
 * A simple class to communicate with the spell checking service for purposes of
 * checking the spelling of words.
 */
class SpellCheckClient
{
public:
    SpellCheckClient  (GMainLoop* mainLoop);
    ~SpellCheckClient (void);

    bool checkWordSpelling (const std::string& word, SpellCheckWordInfo& info);
    bool processTaps (const TapDataArray& taps, SpellCheckWordInfo& info);
    bool getCompletion (const std::string& prefix, std::string& result);

private:

    /*
     * A temp struct only to be used for our synchronous calls.
     * TODO: Replace with async implementation.
     */
    struct LastCallResponse
    {

        LastCallResponse (void) : gotResponse(false)
        {
        }

        void clear (void)
        {
            response.clear();
            gotResponse = false;
        }

        SpellCheckWordInfo response;
        bool gotResponse;
    };

    bool        registerWithServiceBus (void);
    bool        unregisterWithServiceBus (void);
    static bool spellCheckResponse (LSHandle *sh, LSMessage *reply, void *ctx);
    static bool getCompletionResponse (LSHandle *sg, LSMessage *reply, void *ctx);

    LSHandle*         mp_serviceClient;
    bool              m_attachedToServiceBus;
    LastCallResponse  m_lastCallResponse;
    GMainLoop*        mp_mainLoop;
    GMainContext*     mp_mainContext;
};


}

#endif

