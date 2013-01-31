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


#include "SpellCheckClient.h"
#include <pbnjson.hpp>


namespace SmartKey
{

const char* const k_pszResponseSchema = "{}";

const char* const k_pszCallSchema = "{\"type\" : \"object\", \
									\"properties\" : { \
										\"guess\" : {\"type\" : \"string\"} \
									} \
								}";

const char* const k_pszCompCallSchema = "{\"type\" : \"object\", \
										\"properties\" : { \
											\"prefix\" : {\"type\" : \"string\"} \
										} \
									}";


/**
* SpellCheckClient()
* <here is function description>
*
* @param mainLoop
*   <perameter description>
*/
SpellCheckClient::SpellCheckClient(GMainLoop* mainLoop) :
	  m_serviceClient(NULL)
	, m_attachedToServiceBus(false)
	, m_mainLoop(mainLoop)
	, m_mainContext(g_main_loop_get_context(mainLoop))
{
	LSError lserror;
    LSErrorInit(&lserror);

    if (!LSRegister(NULL, &m_serviceClient, &lserror)) {
		g_warning("Error %d registering with LS bus: '%s'", lserror.error_code, lserror.message);
        LSErrorFree(&lserror);
    }
    else {
		m_attachedToServiceBus = LSGmainAttach(m_serviceClient, m_mainLoop, &lserror);
        if (!m_attachedToServiceBus) {
			g_warning("Error %d attaching to LS bus: '%s'", lserror.error_code, lserror.message);
            LSErrorFree(&lserror);
        }
    }
}

/**
* ~SpellCheckClient()
* <here is function description>
*/
SpellCheckClient::~SpellCheckClient()
{
	LSError lserror;
    LSErrorInit(&lserror);
#if 0
	if (!LSGmainDetach(m_serviceClient, &lserror)) {
		g_warning("Error %d detaching from bus: '%s'", lserror.error_code, lserror.message);
		LSErrorFree(&lserror);
    	LSErrorInit(&lserror);
	}
#endif

	if (!LSUnregister(m_serviceClient, &lserror)) {
		g_warning("Error %d unregistering with: '%s'", lserror.error_code, lserror.message);
		LSErrorFree(&lserror);
	}
}

/**
* spellCheckResponse()
* <here is function description>
*
* @param *sh
*   <perameter description>
*
* @param *reply
*   <perameter description>
*
* @param *ctx
*   <perameter description>
*
* @return bool
*   <return value description>
*/
bool SpellCheckClient::spellCheckResponse (LSHandle *sh, LSMessage *reply, void *ctx)
{
	SpellCheckClient* checker = static_cast<SpellCheckClient*>(ctx);
	g_debug("Got spell check response");

	checker->m_lastCallResponse.gotResponse = true;

	std::string jsonRaw = LSMessageGetPayload(reply);
    
    static pbnjson::JSchema inputSchema = pbnjson::JSchemaFragment(k_pszResponseSchema);

    pbnjson::JDomParser parser;
    if (parser.parse(jsonRaw, inputSchema)) {
		pbnjson::JValue parsed = parser.getDom();

		if (parsed["returnValue"].asBool()) {
			pbnjson::JValue guesses = parsed["guesses"];
			if (guesses.isArray()) {
				for (int i = 0; i < guesses.arraySize(); i++) {
					pbnjson::JValue val = guesses[i];
					WordGuess guess(val["str"].asString().c_str());
					if (!val.hasKey("sp")) {
						guess.spellCorrection = val["sp"].asBool();
					}
					if (!val.hasKey("auto-replace")) {
						guess.autoReplace = val["auto-replace"].asBool();
					}
					checker->m_lastCallResponse.response.guesses.push_back(guess);
				}
			}
		}
    }

	return true;
}

/**
* checkWordSpelling()
* <here is function description>
*
* @param word
*   <perameter description>
*
* @param info
*   <perameter description>
*
* @return bool
*   <return value description>
*/
bool SpellCheckClient::checkWordSpelling (const std::string& word, SpellCheckWordInfo& info)
{
	bool success(false);

	if (word.empty())
		return success;

	g_debug("Asked to get guesses for '%s'", word.c_str());

	g_assert(info.isEmpty());

	if (m_attachedToServiceBus) {
		
		pbnjson::JValue callData = pbnjson::Object();
		callData.put("query", word);

		pbnjson::JGenerator serializer(NULL);
		std::string payload;
		pbnjson::JSchema callSchema = pbnjson::JSchemaFragment(k_pszCallSchema);

		if (serializer.toString(callData, callSchema, payload)) {
			LSError lserror;
			LSErrorInit(&lserror);
			if (LSCall(m_serviceClient, "palm://com.palm.smartKey/search",
					payload.c_str(), spellCheckResponse, this, NULL, &lserror)) {
				g_debug("Waiting for response");
				while (!m_lastCallResponse.gotResponse) {
					g_main_context_iteration(m_mainContext, true /*may block*/);
					info = m_lastCallResponse.response;
				}
				success = true;
				g_debug("got response");
			}
			else {
				g_warning("Error %d making LS bus call: '%s'", lserror.error_code, lserror.message);
                LSErrorFree(&lserror);
			}
		}
	}
	else {
		g_warning("Not attached to service bus");
	}

	return success;
}

/**
* processTaps()
* <here is function description>
*
* @param taps
*   <perameter description>
*
* @param info
*   <perameter description>
*
* @return bool
*   <return value description>
*/
bool SpellCheckClient::processTaps (const TapDataArray& taps, SpellCheckWordInfo& info)
{
	bool success(false);

	g_debug("Asked to process taps");

	g_assert(info.isEmpty());

	if (m_attachedToServiceBus) {
		
		pbnjson::JValue callData = pbnjson::Object();
		pbnjson::JValue tapData = pbnjson::Array();
		for (size_t i=0; i<taps.size(); i++) {

			tapData.append(taps[i].x);
			tapData.append(taps[i].y);
			tapData.append(taps[i].car);
			tapData.append(taps[i].shifted);
		}
		callData.put("taps", tapData);

		pbnjson::JGenerator serializer(NULL);
		std::string payload;
		pbnjson::JSchema callSchema = pbnjson::JSchemaFragment("{}");	// we should have a schema, but do we really care?...

		if (serializer.toString(callData, callSchema, payload)) {
			LSError lserror;
			LSErrorInit(&lserror);
			if (LSCall(m_serviceClient, "palm://com.palm.smartKey/processTaps",
					payload.c_str(), spellCheckResponse, this, NULL, &lserror)) {
				g_debug("Waiting for response");
				while (!m_lastCallResponse.gotResponse) {
					g_main_context_iteration(m_mainContext, true /*may block*/);
					info = m_lastCallResponse.response;
				}
				success = true;
				g_debug("got response");
			}
			else {
				g_warning("Error %d making LS bus call: '%s'", lserror.error_code, lserror.message);
                LSErrorFree(&lserror);
			}
		}
	}
	else {
		g_warning("Not attached to service bus");
	}

	return success;
}

/**
* getCompletion()
* <here is function description>
*
* @param prefix
*   <perameter description>
*
* @param result
*   <perameter description>
*
* @return bool
*   <return value description>
*/
bool SpellCheckClient::getCompletion (const std::string& prefix, std::string& result)
{
	bool success(false);

	g_debug("Asked to process taps");

	if (m_attachedToServiceBus) {
		
		pbnjson::JValue callData = pbnjson::Object();
		callData.put("prefix", prefix);

		pbnjson::JGenerator serializer(NULL);
		std::string payload;
		pbnjson::JSchema callSchema = pbnjson::JSchemaFragment(k_pszCompCallSchema);

		if (serializer.toString(callData, callSchema, payload)) {
			LSError lserror;
			LSErrorInit(&lserror);
			if (LSCall(m_serviceClient, "palm://com.palm.smartKey/getCompletion",
					payload.c_str(), spellCheckResponse, this, NULL, &lserror)) {
				g_debug("Waiting for response");
				while (!m_lastCallResponse.gotResponse) {
					g_main_context_iteration(m_mainContext, true /*may block*/);
					result = m_lastCallResponse.response.guesses.front().guess;
				}
				success = true;
				g_debug("got response");
			}
			else {
				g_warning("Error %d making LS bus call: '%s'", lserror.error_code, lserror.message);
                LSErrorFree(&lserror);
			}
		}
	}
	else {
		g_warning("Not attached to service bus");
	}

	return success;
}

/**
* getCompletionResponse()
* <here is function description>
*
* @param *sh
*   <perameter description>
*
* @param *reply
*   <perameter description>
*
* @param *ctx
*   <perameter description>
*
* @return bool
*   <return value description>
*/
bool SpellCheckClient::getCompletionResponse (LSHandle *sh, LSMessage *reply, void *ctx)
{
	SpellCheckClient* checker = static_cast<SpellCheckClient*>(ctx);
	g_debug("Got completion response");

	checker->m_lastCallResponse.gotResponse = true;

	std::string jsonRaw = LSMessageGetPayload(reply);
    
    static pbnjson::JSchema inputSchema = pbnjson::JSchemaFragment(k_pszResponseSchema);

    pbnjson::JDomParser parser;
    if (parser.parse(jsonRaw, inputSchema)) {
		pbnjson::JValue parsed = parser.getDom();

		if (parsed["returnValue"].asBool()) {
			pbnjson::JValue completion = parsed["comp"];
			if (completion.isString()) {
				WordGuess guess(completion.asString().c_str());
				checker->m_lastCallResponse.response.guesses.push_back(guess);
			}
		}
    }

	return true;
}

}
