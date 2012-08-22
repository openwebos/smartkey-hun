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

#include "SmkyDatabase.h"

namespace SmartKey
{

SmkyDatabase::SmkyDatabase(SMKY_LINFO& lingInfo) :
	m_lingInfo(lingInfo)
{
}

/**
 * Add a word to this database.
 */
SmartKeyErrorCode SmkyDatabase::learnWord(const std::string& word)
{
	return SKERR_UNSUPPORTED;
}

/**
 * Un-learn a word.
 */
SmartKeyErrorCode SmkyDatabase::forgetWord(const std::string& word)
{
	return SKERR_UNSUPPORTED;
}


}

